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

MetalWarpmap_p MetalWarpmap::create( const Blueprint& bp, const CoordSPX * pNodes, int pitch )
{
	if( !_validateBlueprint(bp) || !pNodes )
		return nullptr;

	auto p = MetalWarpmap_p( new MetalWarpmap(bp) );

	SizeI nodes = p->nodes();
	p->importNodes({ 0, 0, nodes.w, nodes.h }, pNodes, pitch);
	return p;
}

//____ constructor ___________________________________________________________

MetalWarpmap::MetalWarpmap( const Blueprint& bp ) : Warpmap(bp)
{
	SizeI cells = _cells();

	size_t bytes = size_t(cells.w) * cells.h * 2 * 4 * sizeof(float);

	m_bufferId = [MetalBackend::metalDevice() newBufferWithLength:bytes options:MTLResourceStorageModeShared];
	m_pBuffer = (float *)[m_bufferId contents];

	_nodesUpdated({ 0, 0, m_nodes.w, m_nodes.h });
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

//____ _nodesUpdated() _______________________________________________________
//
// Rebuilds the rows of cells touching the nodes that changed.

void MetalWarpmap::_nodesUpdated( const RectI& region )
{
	SizeI cells = _cells();

	int rowBeg = std::max(0, region.y - 1);
	int rowEnd = std::min(cells.h, region.y + region.h);

	float * pOut = m_pBuffer + rowBeg * cells.w * 2 * 4;

	for( int row = rowBeg ; row < rowEnd ; row++ )
	{
		for( int column = 0 ; column < cells.w ; column++ )
		{
			CoordSPX corners[4];
			cellCorners(column, row, corners);

			for( int i = 0 ; i < 4 ; i++ )
			{
				*pOut++ = corners[i].x / 64.f;
				*pOut++ = corners[i].y / 64.f;
			}
		}
	}
}

} // namespace wax
