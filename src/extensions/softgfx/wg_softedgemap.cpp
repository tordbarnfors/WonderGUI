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
#include <wg_softedgemap.h>

#include <cstring>

namespace wg
{

const TypeInfo SoftEdgemap::TYPEINFO = { "SoftEdgemap", &Edgemap::TYPEINFO };


//____ create() ______________________________________________________________

SoftEdgemap_p	SoftEdgemap::create( const Blueprint& blueprint )
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;
	
	return SoftEdgemap_p( new SoftEdgemap(blueprint) );
}

SoftEdgemap_p SoftEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;

	auto p = SoftEdgemap_p( new SoftEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

SoftEdgemap_p SoftEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;
	
	auto p = SoftEdgemap_p( new SoftEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

//____ constructor ___________________________________________________________

SoftEdgemap::SoftEdgemap(const Blueprint& bp) : Edgemap(bp)
{
	_colorsUpdated(0, m_nbSegments);
}

//____ destructor ____________________________________________________________

SoftEdgemap::~SoftEdgemap()
{
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& SoftEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ _samplesUpdated() _______________________________________________________

void SoftEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
{
	// We do nothing.
}

//____ _colorsUpdated() ________________________________________________________

void SoftEdgemap::_colorsUpdated(int beginSegment, int endSegment)
{
	RectSPX rect(0, 0, m_size.w * 64, m_size.h * 64);

	for (int seg = beginSegment; seg < endSegment; seg++)
	{
		if (m_pTints[seg])
		{
			m_segmentTints[seg].set(m_pTints[seg], rect);
			m_transparentSegments[seg] = m_segmentTints[seg].isTransparent();
			m_opaqueSegments[seg] = m_segmentTints[seg].isOpaque();
		}
		else
		{
			m_segmentTints[seg].setFlat(m_pFlatColors[seg]);

			int alpha = m_pFlatColors[seg].a;
			m_transparentSegments[seg] = (alpha == 0);
			m_opaqueSegments[seg] = (alpha == 4096);
		}
	}
}

} // namespace wg

