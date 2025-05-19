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

#include <cstring>

namespace wg
{

const TypeInfo MetalEdgemap::TYPEINFO = { "MetalEdgemap", &Edgemap::TYPEINFO };


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
	// Create Metal buffer

	int samplesSize = bp.size.w * (bp.segments - 1) * 4;
	int paletteSize = m_paletteSize * 4;

	m_paletteOfs = samplesSize;
	m_whiteColorOfs = samplesSize + paletteSize;

	int bufferSize = samplesSize + paletteSize + 4;		// +4 for white color

	m_bufferId = [MetalBackend::s_metalDevice newBufferWithLength:bufferSize*sizeof(float) options:MTLResourceStorageModeShared];
	m_pBuffer = (float *)[m_bufferId contents];

	// Convert and upload colorstrips

	_colorsUpdated(0, m_paletteSize);

	// Add and upload default white color

	auto pOut = &m_pBuffer[m_whiteColorOfs];

	* pOut++ = 1.f;
	* pOut++ = 1.f;
	* pOut++ = 1.f;
	* pOut++ = 1.f;
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

	auto pOut = m_pBuffer + sampleBegin * edgeStripPitch * 4;

	for (int i = 0; i < nPixelColumns; i++)
	{
		for (int j = 0; j < m_nbSegments - 1; j++)
		{
			int edgeIn = pEdges[j];
			int edgeOut = pEdges[edgeStripPitch + j];

			if (edgeIn > edgeOut)
				std::swap(edgeIn, edgeOut);

			float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
			float beginAdder;
			float endAdder;

			if ((edgeOut & 0xFFFFFFC0) <= (unsigned int)edgeIn)
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
	}
}


//____ _colorsUpdated() ____________________________________________________

void MetalEdgemap::_colorsUpdated(int beginColor, int endColor)
{
	int nColors = endColor - beginColor;

	auto pIn = m_pPalette + beginColor;
	auto pOut = m_pBuffer + m_paletteOfs + beginColor * 4;
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

