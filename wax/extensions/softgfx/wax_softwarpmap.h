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
#ifndef WAX_SOFTWARPMAP_DOT_H
#define	WAX_SOFTWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>

#include <vector>

namespace wax
{
	class SoftWarpmap;
	typedef	StrongPtr<SoftWarpmap>	SoftWarpmap_p;
	typedef	WeakPtr<SoftWarpmap>	SoftWarpmap_wp;

	//____ SoftWarpmap _________________________________________________________
	/**
	 * @brief Warpmap for SoftBackend, LinearBackend and ReferenceBackend.
	 *
	 * ReferenceBackend reads the vertices and triangles directly. For SoftBackend and LinearBackend
	 * each triangle is set up for drawing when the content changes, see Setup.
	 */

	class SoftWarpmap : public Warpmap
	{
		friend class SoftBackend;

	public:

		//.____ Creation __________________________________________

		static SoftWarpmap_p	create( const Blueprint& blueprint );
		static SoftWarpmap_p	create( const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		SoftWarpmap( const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr );
		virtual ~SoftWarpmap() {}

		void	_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles ) override;

		// A triangle set up for drawing, see wax_softwarpmap.cpp. Pixels are those of the Warpmap,
		// x and y in whole pixels, and their centers 32 spx into them.

		struct Setup
		{
			// Pixels with centers that can be inside: rows rowBeg -> rowEnd and columns colBeg ->
			// colEnd, within the Warpmap. Empty for a triangle without area.

			int			rowBeg, rowEnd, colBeg, colEnd;

			// Edge functions, edge[i] + edgeX[i] * x + edgeY[i] * y at the center of pixel x,y. The
			// center is inside where all are positive, or 0 for an edge with tie[i] set. Edge i is
			// opposite corner i and is the area of the triangle at it.

			int64_t		edge[3];
			int64_t		edgeX[3];
			int64_t		edgeY[3];
			bool		tie[3];

			// Values at the center of pixel ox,oy and their change from one pixel to the next along x
			// and y: f[axis][0] + f[axis][1] * (x - ox) + f[axis][2] * (y - oy). Without perspective
			// u (axis 0) and v (axis 1) are positions in the source, in pixels with 32 binals. With
			// perspective a position is ref + f[axis] / f[2], in pixels with 16 binals, f[2] being W.

			bool		bPerspective;
			int			ox, oy;
			int64_t		f[3][3];
			int64_t		ref[2];

			// Without perspective: bounds of the positions over the area of its pixels, in pixels of
			// the source with 16 binals. With perspective: how much W can change from one pixel to
			// the next, |dW/dx| + |dW/dy|, which gives the pixels between divisions.

			int64_t		lo[2], hi[2];
			int64_t		gradientW;
		};

		void	_setupTriangles();

		std::vector<Setup>	m_setups;
		std::vector<int>	m_order;			// Triangles with pixels, by first row.
	};

} // namespace wax
#endif //WAX_SOFTWARPMAP_DOT_H
