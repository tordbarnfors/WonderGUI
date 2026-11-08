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

		// A bound of the pixels of a triangle on a row: the first column inside for a lower bound,
		// the first one outside for an upper bound. Stepped exactly from row to row: col by stepCol
		// and rem by -stepRem, then by one column more and the divisor where rem goes below 0.

		struct Edge
		{
			int64_t		col;			// At rowBeg.
			int32_t		rem;			// At rowBeg, 0 -> divisor - 1.
			int32_t		divisor;
			int32_t		stepCol;
			int32_t		stepRem;		// 0 -> divisor - 1.
		};

		struct Setup
		{
			// Pixels with centers that can be inside: rows rowBeg -> rowEnd and columns colBeg ->
			// colEnd, within the Warpmap and the rows that edges along a row allow.

			int			rowBeg, rowEnd, colBeg, colEnd;

			// Values at the center of pixel ox,oy and their change from one pixel to the next along x
			// and y: f[axis][0] + f[axis][1] * (x - ox) + f[axis][2] * (y - oy), u (axis 0) and v
			// (axis 1) as positions in the source in pixels with 32 binals. With perspective they are
			// in m_perspective[perspective] instead.

			int			ox, oy;
			int			perspective;	// -1 without perspective.

			// The other edges, lower bounds first.

			uint8_t		nLower, nUpper;
			Edge		edges[3];

			int64_t		f[2][3];

			// Bounds of the positions over the area of its pixels, without perspective, in pixels of
			// the source with 16 binals.

			int64_t		lo[2], hi[2];
		};

		// Values of a triangle in perspective, as f of Setup: a position is ref + f[axis] / f[2], in
		// pixels with 16 binals, f[2] being W. gradientW is how much W can change from one pixel to
		// the next, |dW/dx| + |dW/dy|, which gives the pixels between divisions.

		struct Perspective
		{
			int64_t		f[3][3];
			int64_t		ref[2];
			int64_t		gradientW;
		};

		void	_setupTriangles();

		std::vector<Setup>			m_setups;			// Triangles with pixels.
		std::vector<int>			m_order;			// Of m_setups, by first row.
		std::vector<int>			m_rowStart;			// For sorting m_order.
		std::vector<Perspective>	m_perspective;
	};

} // namespace wax
#endif //WAX_SOFTWARPMAP_DOT_H
