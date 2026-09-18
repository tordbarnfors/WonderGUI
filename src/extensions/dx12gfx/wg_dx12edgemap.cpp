/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_dx12edgemap.h>
#include <wg_gradyent.h>
#include <wg_gfxbase.h>

#include <cstring>
#include <algorithm>
#include <utility>

namespace wg
{

	const TypeInfo DX12Edgemap::TYPEINFO = { "DX12Edgemap", &Edgemap::TYPEINFO };

	ID3D12Device * DX12Edgemap::s_pDevice = nullptr;

	//____ setDevice() _________________________________________________________

	bool DX12Edgemap::setDevice( ID3D12Device * pDevice )
	{
		s_pDevice = pDevice;
		return true;
	}

	//____ exitDevice() ________________________________________________________

	void DX12Edgemap::exitDevice()
	{
		s_pDevice = nullptr;
	}


	//____ create() ______________________________________________________________

	DX12Edgemap_p	DX12Edgemap::create(const Blueprint& blueprint)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		return DX12Edgemap_p(new DX12Edgemap(blueprint));
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigo origo, const float* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	DX12Edgemap_p DX12Edgemap::create(const Blueprint& blueprint, SampleOrigo origo, const spx* pSamples, int edges, int edgePitch, int samplePitch)
	{
		if (!_validateBlueprint(blueprint))
			return nullptr;

		if (edges >= blueprint.segments)
			return nullptr;

		auto p = DX12Edgemap_p(new DX12Edgemap(blueprint));

		p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w + 1, edgePitch, samplePitch);
		p->m_nbRenderSegments = edges + 1;

		return p;
	}

	//____ constructor ___________________________________________________________

	DX12Edgemap::DX12Edgemap(const Blueprint& bp) : Edgemap(bp)
	{
		// One buffer holds everything the segments shader reads: an edge strip per
		// pixel column, then the palette, then a white color. Laid out as floats
		// here, read as float4 entries by the shader.

		int samplesSize = bp.size.w * (bp.segments - 1) * 4;
		int paletteSize = m_paletteSize * 4;

		m_paletteOfs = samplesSize;
		m_whiteColorOfs = samplesSize + paletteSize;

		int bufferSize = samplesSize + paletteSize + 4;			// The last 4 are the white color.

		if( !s_pDevice )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend must be created before any edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(bufferSize) * sizeof(float);
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_buffer.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		D3D12_RANGE readRange = { 0, 0 };			// We only write.

		if( FAILED(m_buffer->Map(0, &readRange, (void**) &m_pBuffer)) )
		{
			m_buffer = nullptr;
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map buffer for edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		// A D3D12 upload buffer comes with whatever was in that memory, unlike an
		// MTLBuffer. Columns nothing has written to yet would otherwise be noise.
		//
		//TODO: There is one buffer and two frames in flight, so updating samples
		// can change what the GPU is still reading for the previous frame. Metal
		// has the same, but our own per-frame buffers show what the fix looks like.

		memset( m_pBuffer, 0, size_t(bufferSize) * sizeof(float) );

		_colorsUpdated(0, m_paletteSize);

		// The white color, for the axis that has no colorstrip of its own.

		float * pOut = &m_pBuffer[m_whiteColorOfs];

		*pOut++ = 1.f;
		*pOut++ = 1.f;
		*pOut++ = 1.f;
		*pOut++ = 1.f;
	}

	//____ destructor ____________________________________________________________

	DX12Edgemap::~DX12Edgemap()
	{
		if( m_buffer && m_pBuffer )
			m_buffer->Unmap(0, nullptr);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& DX12Edgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _samplesUpdated() _______________________________________________________

	void DX12Edgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		if( !m_pBuffer )
			return;

		// The columns on either side of the first and last sample change too.

		int columnBegin = sampleBegin == 0 ? 0 : sampleBegin - 1;
		int columnEnd = sampleEnd > m_size.w ? m_size.w : sampleEnd;

		int nPixelColumns = columnEnd - columnBegin;

		int edgeStripPitch = m_nbSegments - 1;

		const spx * pEdges = m_pSamples + edgeStripPitch * columnBegin;
		int outColumnSkip = ((m_nbSegments - 1) - (edgeEnd - edgeBegin)) * 4;

		float * pOut = m_pBuffer + columnBegin * edgeStripPitch * 4 + edgeBegin * 4;

		for (int i = 0; i < nPixelColumns; i++)
		{
			for (int j = edgeBegin; j < edgeEnd; j++)
			{
				int edgeIn = pEdges[j];
				int edgeOut = pEdges[edgeStripPitch + j];

				if (edgeIn > edgeOut)
					std::swap(edgeIn, edgeOut);

				float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
				float beginAdder;
				float endAdder;

				if ((edgeOut & 0xFFFFFFC0) == (edgeIn & 0xFFFFFFC0))
				{
					float firstPixelCoverage = ((64 - (edgeOut & 0x3F)) + (edgeOut - edgeIn) / 2) / 64.f;

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = beginAdder;
				}
				else
				{
					int height = 64 - (edgeIn & 0x3F);
					int width = (int)(increment * height);
					float firstPixelCoverage = (height * width) / (2 * 4096.f);
					float lastPixelCoverage = 1.f - (edgeOut & 0x3F) * increment * (edgeOut & 0x3F) / (2 * 4096.f);

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = lastPixelCoverage - (1.f - (edgeOut & 0x3F) * increment / 64.f);
				}

				*pOut++ = edgeIn / 64.f;				// Where the segment begins, in pixels.
				*pOut++ = increment;
				*pOut++ = beginAdder;
				*pOut++ = endAdder;
			}

			pEdges += edgeStripPitch;
			pOut += outColumnSkip;
		}
	}

	//____ _colorsUpdated() ________________________________________________________

	void DX12Edgemap::_colorsUpdated(int beginColor, int endColor)
	{
		if( !m_pBuffer )
			return;

		int nColors = endColor - beginColor;

		const HiColor * pIn = m_pPalette + beginColor;
		float * pOut = m_pBuffer + m_paletteOfs + beginColor * 4;

		for (int i = 0; i < nColors; i++)
		{
			*pOut++ = pIn->r / 4096.f;
			*pOut++ = pIn->g / 4096.f;
			*pOut++ = pIn->b / 4096.f;
			*pOut++ = pIn->a / 4096.f;

			pIn++;
		}
	}


} // namespace wg

