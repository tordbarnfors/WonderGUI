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

#include <wax_gpuedgemap.h>
#include <wax_tinttools.h>

#include <cstring>
#include <utility>

namespace wax
{

	const TypeInfo GpuEdgemap::TYPEINFO = { "GpuEdgemap", &Edgemap::TYPEINFO };

	const int GpuEdgemap::c_tintSlotSize = TintTools::gpuTintBlockMaxSize(Tint::c_maxMixComponents, Tint::c_maxMixComponents * Tint::c_maxStops);

	//____ constructor ___________________________________________________________

	GpuEdgemap::GpuEdgemap(const Blueprint& bp) : Edgemap(bp)
	{
		m_samplesSize = bp.size.w * (bp.segments - 1) * 2;			// Edge strips and raw samples.
	}

	//____ destructor ____________________________________________________________

	GpuEdgemap::~GpuEdgemap()
	{
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& GpuEdgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _createContent() ______________________________________________________

	bool GpuEdgemap::_createContent()
	{
		m_pGpuData = nullptr;
		m_bHasTintSlots = false;

		if( !_replaceBuffer(m_nbTints > 0) )
			return false;

		_samplesUpdated(0, m_nbSegments - 1, 0, m_size.w + 1);
		_writeColors(0, m_nbSegments);
		return true;
	}

	//____ _samplesUpdated() _______________________________________________________

	void GpuEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
	{
		if( !m_pGpuData )
			return;

		// The columns on either side of the first and last sample change too.

		int columnBegin = sampleBegin == 0 ? 0 : sampleBegin - 1;
		int columnEnd = sampleEnd > m_size.w ? m_size.w : sampleEnd;

		int nPixelColumns = columnEnd - columnBegin;

		int edgeStripPitch = m_nbSegments - 1;

		const spx * pEdges = m_pSamples + edgeStripPitch * columnBegin;
		int outColumnSkip = ((m_nbSegments - 1) - (edgeEnd - edgeBegin)) * 4;

		// Raw samples for warps, the left and right side of each column in pixels.

		float * pRaw = m_pGpuData + (_rawSamplesOfs() + columnBegin * edgeStripPitch + edgeBegin) * 4;

		for (int i = 0; i < nPixelColumns; i++)
		{
			for (int j = edgeBegin; j < edgeEnd; j++)
			{
				*pRaw++ = pEdges[i * edgeStripPitch + j] / 64.f;
				*pRaw++ = pEdges[(i + 1) * edgeStripPitch + j] / 64.f;
				*pRaw++ = 0.f;
				*pRaw++ = 0.f;
			}

			pRaw += outColumnSkip;
		}

		// Edge strips

		float * pOut = m_pGpuData + columnBegin * edgeStripPitch * 4 + edgeBegin * 4;

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

	void GpuEdgemap::_colorsUpdated(int beginSegment, int endSegment)
	{
		if( !m_pGpuData )
			return;

		// First tint on an edgemap that had none, it needs a bigger buffer. Happens once per
		// edgemap at most.

		if( m_nbTints > 0 && !m_bHasTintSlots )
		{
			if( !_replaceBuffer(true) )
				return;

			beginSegment = 0;
			endSegment = m_nbSegments;
		}

		_writeColors(beginSegment, endSegment);
	}

	//____ _replaceBuffer() ________________________________________________________
	//
	// Gets a new buffer, with or without room for tint blocks, and carries over the edge strips
	// and raw samples of the one it replaces. Colors are left for _writeColors().

	bool GpuEdgemap::_replaceBuffer(bool bWithTintSlots)
	{
		int entries = m_samplesSize + m_nbSegments * 2;

		if( bWithTintSlots )
			entries += m_nbSegments * c_tintSlotSize;

		size_t bytes = size_t(entries) * 4 * sizeof(float);

		float * pNew = _allocBuffer(bytes);
		if( !pNew )
			return false;

		// A new buffer comes with whatever was in that memory. Columns nothing has written to
		// yet would otherwise be noise.

		memset( pNew, 0, bytes );

		if( m_pGpuData )
			memcpy( pNew, m_pGpuData, size_t(m_samplesSize) * 4 * sizeof(float) );

		_commitBuffer();

		m_pGpuData = pNew;
		m_bHasTintSlots = bWithTintSlots;
		return true;
	}

	//____ _writeColors() __________________________________________________________
	//
	// Flat colors, tint table entries and tint blocks of the given segments. A segment with a
	// tint gets its block written to its own slot.

	void GpuEdgemap::_writeColors(int beginSegment, int endSegment)
	{
		RectSPX rect(0, 0, m_size.w * 64, m_size.h * 64);

		for (int seg = beginSegment; seg < endSegment; seg++)
		{
			float * pFlat = m_pGpuData + (_flatColorsOfs() + seg) * 4;
			float * pTable = m_pGpuData + (_tintTableOfs() + seg) * 4;

			m_pFlatColors[seg].toLinearFloat(pFlat);		// Colors are sRGB, but we blend in linear.

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
					TintTools::writeGpuTintBlock(decoded, m_pGpuData + blockOfs * 4);
				}
			}

			pTable[0] = float(blockOfs);
			pTable[1] = 0.f;
			pTable[2] = 0.f;
			pTable[3] = 0.f;
		}
	}

} // namespace wax
