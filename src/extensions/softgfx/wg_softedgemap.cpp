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

#include <wg_softedgemap.h>
#include <wg_gradyent.h>

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

void SoftEdgemap::_colorsUpdated(int beginColor, int endColor)
{
	// Update m_transparentSegments and m_opaqueSegments

	if (m_pFlatColors)
	{
		for (int seg = beginColor; seg < endColor; seg++)
		{
			int alpha = m_pPalette[seg].a;
			m_transparentSegments[seg] = (alpha == 0);
			m_opaqueSegments[seg] = (alpha == 4096);
		}
	}
	else
	{
		// To keep it simple we go through the whole palette on each update.

		for (int seg = 0; seg < m_nbSegments; seg++)
		{
			int totalAlpha = 0;
			int totalColors = 0;

			if (m_pColorstripsX)
			{
				HiColor* pColor = m_pColorstripsX + m_size.w * seg;
				for (int i = 0; i < m_size.w; i++)
					totalAlpha += pColor[i].a;

				totalColors += m_size.w;
			}

			if (m_pColorstripsY)
			{
				HiColor* pColor = m_pColorstripsY + m_size.h * seg;
				for (int i = 0; i < m_size.h; i++)
					totalAlpha += pColor[i].a;

				totalColors += m_size.h;
			}

			m_transparentSegments[seg] = (totalAlpha == 0);
			m_opaqueSegments[seg] = (totalAlpha == 4096 * totalColors);
		}
	}
}


} // namespace wg

