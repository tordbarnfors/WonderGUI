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

#include <wax_metaledgemap.h>
#include <wax_metalbackend.h>
#include <wax_base.h>

#include <cstring>

namespace wax
{

const TypeInfo MetalEdgemap::TYPEINFO = { "MetalEdgemap", &GpuEdgemap::TYPEINFO };


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

	p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
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

	p->_importSamples(origo, pSamples, 0, edges, 0, p->samples(), edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

//____ constructor ___________________________________________________________

MetalEdgemap::MetalEdgemap(const Blueprint& bp) : GpuEdgemap(bp)
{
	// Without a buffer MetalBackend draws nothing for the edgemap.

	if( MetalBackend::s_metalDevice == nil )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
			"No Metal device set. MetalBackend::setMetalDevice() must be called before any edgemap is created.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	_createContent();
}

//____ destructor ____________________________________________________________

MetalEdgemap::~MetalEdgemap()
{
	[m_buffer release];
	[m_newBuffer release];
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& MetalEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ _allocBuffer() ________________________________________________________

float * MetalEdgemap::_allocBuffer( size_t bytes )
{
	[m_newBuffer release];
	m_newBuffer = [MetalBackend::s_metalDevice newBufferWithLength:bytes options:MTLResourceStorageModeShared];

	return m_newBuffer ? (float *) [m_newBuffer contents] : nullptr;
}

//____ _commitBuffer() _______________________________________________________
//
// Command buffers that have encoded the old buffer keep it until they are done with it.

void MetalEdgemap::_commitBuffer()
{
	[m_buffer release];
	m_buffer = m_newBuffer;
	m_newBuffer = nil;
}

} // namespace wax
