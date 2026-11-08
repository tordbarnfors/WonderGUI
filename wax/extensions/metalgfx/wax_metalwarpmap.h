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
#ifndef WAX_METALWARPMAP_DOT_H
#define WAX_METALWARPMAP_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wax_warpmap.h>


namespace wax
{

	class MetalWarpmap;
	typedef StrongPtr<MetalWarpmap> MetalWarpmap_p;
	typedef WeakPtr<MetalWarpmap> MetalWarpmap_wp;

	//____ MetalWarpmap ________________________________________________________
	/**
	 * @brief Warpmap for MetalBackend.
	 *
	 * The mesh is kept on the GPU as it is, in one shared buffer: the vertices (x, y, u, v, w and
	 * coverage as integers, the layout of Warpmap::Vertex), read by the vertex shader, followed by
	 * the triangles (the corners as 16-bit unsigned ints, the layout of Warpmap::Triangle), which are
	 * the index buffer. Content that changes is copied there, with nothing worked out on the CPU.
	 * Drawing only gives where to draw, the source offset and the periods. See the warp shaders for
	 * what they do with it.
	 */

	class MetalWarpmap : public Warpmap
	{
		friend class MetalBackend;
	public:

		//.____ Creation __________________________________________

		static MetalWarpmap_p	create( const Blueprint& blueprint );
		static MetalWarpmap_p	create( const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles );

		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		MetalWarpmap( const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr );
		~MetalWarpmap();

		void	_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles ) override;

		id<MTLBuffer>   m_bufferId = nil;
		uint8_t *      	m_pBuffer = nullptr;
		size_t			m_indexOffset = 0;			// Where the triangles start in the buffer, in bytes.
	};

} // namespace wax
#endif // WAX_METALWARPMAP_DOT_H
