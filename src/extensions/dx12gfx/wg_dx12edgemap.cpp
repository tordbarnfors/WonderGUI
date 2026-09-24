/*=========================================================================

							 >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

								-----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

								-----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/


#include <wg_dx12edgemap.h>
#include <wg_dx12backend.h>
#include <wg_tinttools.h>
#include <wg_gfxbase.h>

#include <cstring>
#include <algorithm>
#include <utility>

namespace wg
{

	const TypeInfo DX12Edgemap::TYPEINFO = { "DX12Edgemap", &Edgemap::TYPEINFO };

	ID3D12Device * DX12Edgemap::s_pDevice = nullptr;

	const int DX12Edgemap::c_tintSlotSize = TintTools::gpuTintBlockMaxSize(Tint::c_maxMixComponents, Tint::c_maxMixComponents * Tint::c_maxStops);

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
		m_samplesSize = bp.size.w * (bp.segments - 1);

		if( !s_pDevice )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No D3D12 device set. DX12Backend must be created before any edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		if( _createBuffer(m_nbTints > 0) )
			_writeColors(0, m_nbSegments);
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

	//____ _colorsUpdated() ______________________________________________________

	void DX12Edgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		if( !m_pBuffer )
			return;

		// First tint on an edgemap that had none, it needs a bigger buffer. The
		// GPU may still be reading the one we have, so wait for it before letting
		// go. Happens once per edgemap at most.

		if( m_nbTints > 0 && !m_bHasTintSlots )
		{
			DX12Backend::waitForCompletionOfAll();

			if( !_createBuffer(true) )
				return;

			beginSegment = 0;
			endSegment = m_nbSegments;
		}

		_writeColors(beginSegment, endSegment);
	}

	//____ _createBuffer() _________________________________________________________
	//
	// Creates the buffer, with or without room for tint blocks, and carries over
	// the edge strips of the one it replaces. Colors are left for _writeColors().

	bool DX12Edgemap::_createBuffer(bool bWithTintSlots)
	{
		int entries = m_samplesSize + m_nbSegments * 2;

		if( bWithTintSlots )
			entries += m_nbSegments * c_tintSlotSize;

		size_t bytes = size_t(entries) * 4 * sizeof(float);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(bytes);
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		Microsoft::WRL::ComPtr<ID3D12Resource>	buffer;
		float * pBuffer = nullptr;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(buffer.GetAddressOf()))) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create buffer for edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		D3D12_RANGE readRange = { 0, 0 };			// We only write.

		if( FAILED(buffer->Map(0, &readRange, (void**) &pBuffer)) )
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to map buffer for edgemap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		// A D3D12 upload buffer comes with whatever was in that memory, unlike an
		// MTLBuffer. Columns nothing has written to yet would otherwise be noise.
		//
		//TODO: There is one buffer and two frames in flight, so updating samples
		// can change what the GPU is still reading for the previous frame. Metal
		// has the same, but our own per-frame buffers show what the fix looks like.

		memset( pBuffer, 0, bytes );

		if( m_pBuffer )
		{
			memcpy( pBuffer, m_pBuffer, size_t(m_samplesSize) * 4 * sizeof(float) );
			m_buffer->Unmap(0, nullptr);
		}

		m_buffer = buffer;
		m_pBuffer = pBuffer;
		m_bHasTintSlots = bWithTintSlots;
		return true;
	}

	//____ _writeColors() __________________________________________________________
	//
	// Flat colors, tint table entries and tint blocks of the given segments. A
	// segment with a tint gets its block written to its own slot.

	void DX12Edgemap::_writeColors(int beginSegment, int endSegment)
	{
		RectSPX rect(0, 0, m_size.w * 64, m_size.h * 64);

		for (int seg = beginSegment; seg < endSegment; seg++)
		{
			float * pFlat = m_pBuffer + (_flatColorsOfs() + seg) * 4;
			float * pTable = m_pBuffer + (_tintTableOfs() + seg) * 4;

			const HiColor& col = m_pFlatColors[seg];

			pFlat[0] = col.r / 4096.f;
			pFlat[1] = col.g / 4096.f;
			pFlat[2] = col.b / 4096.f;
			pFlat[3] = col.a / 4096.f;

			int blockOfs = -1;

			if( m_pTints[seg] && m_bHasTintSlots )
			{
				uint16_t	words[TintTools::c_maxEncodedTintWords];
				HiColor		colors[TintTools::c_maxEncodedTintColors];
				int			nColors;

				TintTools::encodeTint(m_pTints[seg], rect, words, colors, nColors);

				const HiColor* pColors = colors;
				TintTools::DecodedTint decoded;
				TintTools::decodeTint(words, pColors, decoded);

				if( decoded.nLayers > 0 )
				{
					blockOfs = _tintSlotOfs(seg);
					TintTools::writeGpuTintBlock(decoded, m_pBuffer + blockOfs * 4);
				}
			}

			pTable[0] = float(blockOfs);
			pTable[1] = 0.f;
			pTable[2] = 0.f;
			pTable[3] = 0.f;
		}
	}


} // namespace wg

