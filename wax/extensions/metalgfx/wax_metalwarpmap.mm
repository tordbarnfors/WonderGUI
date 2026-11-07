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

#include <wax_metalwarpmap.h>
#include <wax_metalbackend.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace wax
{

const TypeInfo MetalWarpmap::TYPEINFO = { "MetalWarpmap", &Warpmap::TYPEINFO };


//____ create() ______________________________________________________________

MetalWarpmap_p MetalWarpmap::create( const Blueprint& bp )
{
	if( !_validateBlueprint(bp) )
		return nullptr;

	return MetalWarpmap_p( new MetalWarpmap(bp) );
}

MetalWarpmap_p MetalWarpmap::create( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles )
{
	if( !pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles) )
		return nullptr;

	return MetalWarpmap_p( new MetalWarpmap(bp, pVertices, pTriangles) );
}

//____ constructor ___________________________________________________________

MetalWarpmap::MetalWarpmap( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles ) : Warpmap(bp, pVertices, pTriangles)
{
	static_assert(sizeof(Vertex) == 5 * sizeof(int32_t) && sizeof(Triangle) == 3 * sizeof(uint32_t), "Vertices and triangles are uploaded as they are");

	// Triangles start at an aligned offset after the vertices. A buffer can't be empty.

	m_indexOffset = (size_t(m_nVertices) * sizeof(Vertex) + 255) & ~size_t(255);
	size_t bytes = std::max(m_indexOffset + size_t(m_nTriangles) * sizeof(Triangle), size_t(256));

	m_bufferId = [MetalBackend::metalDevice() newBufferWithLength:bytes options:MTLResourceStorageModeShared];
	m_pBuffer = (uint8_t *)[m_bufferId contents];

	_updated(0, m_nVertices, 0, m_nTriangles);
}

//____ destructor ____________________________________________________________

MetalWarpmap::~MetalWarpmap()
{
	[m_bufferId release];
	m_bufferId = nil;
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& MetalWarpmap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ _updated() ____________________________________________________________
//
// Copies what changed to the buffer. Periods are given when drawing.

void MetalWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
{
	if( !m_pBuffer )
		return;

	if( nVertices > 0 )
		std::memcpy(m_pBuffer + size_t(firstVertex) * sizeof(Vertex), m_pVertices + firstVertex, size_t(nVertices) * sizeof(Vertex));

	if( nTriangles > 0 )
		std::memcpy(m_pBuffer + m_indexOffset + size_t(firstTriangle) * sizeof(Triangle), m_pTriangles + firstTriangle, size_t(nTriangles) * sizeof(Triangle));
}

} // namespace wax
