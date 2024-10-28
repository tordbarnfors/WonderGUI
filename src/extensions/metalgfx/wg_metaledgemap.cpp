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
}

//____ destructor ____________________________________________________________

MetalEdgemap::~MetalEdgemap()
{
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& MetalEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ _samplesUpdated() ______________________________________________________

void MetalEdgemap::_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd)
{
	
}

//____ _colorsUpdated() _______________________________________________________

void MetalEdgemap::_colorsUpdated(int beginColor, int endColor)
{
	
}



} // namespace wg

