/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Metalware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_metaledgemap.h>
#include <wg_metalbackend.h>
#include <wg_tinttools.h>

#include <cstring>

namespace wg
{

const TypeInfo MetalEdgemap::TYPEINFO = { "MetalEdgemap", &Edgemap::TYPEINFO };

const int MetalEdgemap::c_tintSlotSize = TintTools::gpuTintBlockMaxSize(Tint::c_maxMixComponents, Tint::c_maxMixComponents * Tint::c_maxStops);


//____ create() ______________________________________________________________

MetalEdgemap_p	MetalEdgemap::create( const Blueprint& blueprint )
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;
	
	return MetalEdgemap_p( new MetalEdgemap(blueprint) );
}

MetalEdgemap_p MetalEdgemap::create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;

	auto p = MetalEdgemap_p( new MetalEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

MetalEdgemap_p MetalEdgemap::create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;
	
	auto p = MetalEdgemap_p( new MetalEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

//____ constructor ___________________________________________________________

MetalEdgemap::MetalEdgemap(const Blueprint& bp) : Edgemap(bp)
{
	m_samplesSize = bp.size.w * (bp.segments - 1);

	_createBuffer(m_nbTints > 0);
	_writeColors(0, m_nbSegments);
}

//____ destructor ____________________________________________________________

MetalEdgemap::~MetalEdgemap()
{
	[m_bufferId release];
	m_bufferId = nil;
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& MetalEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ _samplesUpdated() ______________________________________________________

void MetalEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
{
	// We need to update columns left and right of first/last sample.

	int columnBegin = sampleBegin == 0 ? 0 : sampleBegin - 1;
	int columnEnd = sampleEnd > m_size.w ? m_size.w : sampleEnd;

	int nPixelColumns = columnEnd - columnBegin;

	int edgeStripPitch = m_nbSegments - 1;

	const spx* pEdges = m_pSamples + edgeStripPitch * columnBegin;
	int outColumnSkip = ((m_nbSegments - 1) - (edgeEnd - edgeBegin)) * 4;

	auto pOut = m_pBuffer + columnBegin * edgeStripPitch * 4 + edgeBegin * 4;

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

			*pOut++ = edgeIn / 64.f;				// Segment begin pixel
			*pOut++ = increment;					// Segment increment
			*pOut++ = beginAdder;					// Segment begin adder
			*pOut++ = endAdder;						// Segment end adder
		}

		pEdges += edgeStripPitch;
		pOut += outColumnSkip;
	}
}


//____ _colorsUpdated() ____________________________________________________

void MetalEdgemap::_colorsUpdated(int beginSegment, int endSegment)
{
	// First tint on an edgemap that had none needs a bigger buffer. Command
	// buffers already encoded keep the old one alive until they are done.

	if( m_nbTints > 0 && !m_bHasTintSlots )
	{
		_createBuffer(true);
		beginSegment = 0;
		endSegment = m_nbSegments;
	}

	_writeColors(beginSegment, endSegment);
}

//____ _createBuffer() _____________________________________________________
//
// Creates the buffer, with or without room for tint blocks, and carries over
// the edge strips of the one it replaces. Colors are left for _writeColors().

void MetalEdgemap::_createBuffer(bool bWithTintSlots)
{
	int entries = m_samplesSize + m_nbSegments * 2;

	if( bWithTintSlots )
		entries += m_nbSegments * c_tintSlotSize;

	size_t bytes = size_t(entries) * 4 * sizeof(float);

	id<MTLBuffer> bufferId = [MetalBackend::s_metalDevice newBufferWithLength:bytes options:MTLResourceStorageModeShared];
	float * pBuffer = (float *)[bufferId contents];

	std::memset( pBuffer, 0, bytes );

	if( m_pBuffer )
		std::memcpy( pBuffer, m_pBuffer, size_t(m_samplesSize) * 4 * sizeof(float) );

	[m_bufferId release];

	m_bufferId = bufferId;
	m_pBuffer = pBuffer;
	m_bHasTintSlots = bWithTintSlots;
}

//____ _writeColors() ______________________________________________________
//
// Flat colors, tint table entries and tint blocks of the given segments.

void MetalEdgemap::_writeColors(int beginSegment, int endSegment)
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

