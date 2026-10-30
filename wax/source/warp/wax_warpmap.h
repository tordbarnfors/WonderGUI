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
#ifndef WAX_WARPMAP_DOT_H
#define WAX_WARPMAP_DOT_H
#pragma once

#include <wax_object.h>
#include <wax_types.h>
#include <wax_geo.h>

#include <algorithm>

namespace wax
{
	class Warpmap;
	typedef	StrongPtr<Warpmap>	Warpmap_p;
	typedef	WeakPtr<Warpmap>	Warpmap_wp;

	//____ Warpmap _____________________________________________________________
	/**
	 * @brief Bends a surface or an edgemap into another shape when drawn.
	 *
	 * A Warpmap covers a rectangle of size.w * size.h pixels with a grid of nodes, one node every
	 * cellSize pixels in both directions plus one at the right and bottom edge. Each node holds
	 * the position in the source that is drawn at that point, in spx of the source (64 per pixel of
	 * the surface or edgemap, relative to the source rectangle for blits), u along its width and v
	 * along its height. Positions between nodes are interpolated bilinearly. Where a position falls
	 * outside the source nothing is drawn.
	 *
	 * Node values are not limited to the source. Values outside it are what makes parts of the
	 * Warpmap transparent, with the border antialiased where the interpolated position crosses the
	 * border of the source. Keep values within +-maxNodeValue (16 million pixels). Values on a
	 * periodic axis are brought into 0 -> period when imported. A new Warpmap maps its rectangle
	 * straight onto the source, pixel for pixel: u = x * 64 and v = y * 64.
	 *
	 * A Warpmap is made for a source of a certain size. A source of another size is not stretched to
	 * fit, the nodes have to be changed.
	 *
	 * Smaller cells follow curved shapes more closely. A circle of radius r is off by
	 * roughly cellSize^2 / (8 * r) pixels.
	 *
	 * An axis can be periodic (uPeriod/vPeriod > 0, in spx of the source), as when u is the angle of
	 * a ring. Positions are then read modulo the period and interpolation between nodes across the
	 * seam takes the short way: the corners of a cell are taken as the values closest to its top-left
	 * corner, give or take whole periods. With a period of the size of the source it wraps around,
	 * which for an edgemap is seamless if it is tiling. A longer period leaves a gap, such as a
	 * 270 degree arc of a source w spx wide having uPeriod = w * 360 / 270.
	 *
	 * When drawn, a source offset (in spx of the source) is added to the position before reading,
	 * so a ring can be rotated or a pattern scrolled without changing the Warpmap.
	 *
	 * There is no supersampling when the Warpmap shrinks a surface. Use a mipmapped surface for that,
	 * on backends that support mipmaps.
	 *
	 * Warpmaps are created by a WarpmapFactory, which is specific to the backend, or by a generator
	 * such as CircularWarp.
	 *
	 * How a Warpmap is drawn, which all backends follow:
	 *
	 * - The Warpmap is placed on whole pixels (dest is rounded to nearest pixel), like edgemaps.
	 *   Nothing is drawn outside of its rectangle.
	 *
	 * - The position of a point (x,y) of the Warpmap is interpolated from the corners of the cell it
	 *   is in, see positionAt(). Adding the source offset gives the position in the source. A periodic
	 *   axis is then wrapped into 0 -> period.
	 *
	 * - The part of a pixel that is covered is the part of its area whose position is inside the
	 *   source: 0 -> width and 0 -> height. For an edgemap, the part covered by each segment is the
	 *   part whose position is in that segment. Coverage is then applied as for any anti-aliased edge,
	 *   see "Blending and transparency" in the guide. ReferenceBackend measures coverage by sampling
	 *   the pixel at many points, other backends estimate it from the distance to the border or edge
	 *   and how fast the position changes over the pixel.
	 *
	 * - Colors are taken at the position of the center of the pixel: the surface is sampled there
	 *   by its sample method, as if the edge pixels of the source rectangle continued outside it
	 *   (like stretchBlit() reads it), or wrapping around the surface if it is tiling. The tints of
	 *   edgemap segments are evaluated there (in pixels of the edgemap). A center in the gap between copies on a periodic axis is taken in the nearest copy,
	 *   before its start or after its end, so colors don't jump where copies meet. The device tint is
	 *   applied in canvas space as usual.
	 */

	class Warpmap : public Object
	{
	public:

		struct Blueprint
		{
			int				cellSize	= 16;				// Pixels between nodes.
			Finalizer_p		finalizer	= nullptr;
			SizeI			size;							// Mandatory. Size in pixels.
			spx				uPeriod		= 0;				// In spx of the source, 0 = not periodic.
			spx				vPeriod		= 0;				// In spx of the source, 0 = not periodic.
		};

		static const spx	maxNodeValue = 1 << 30;			// Node values and periods are kept within +-maxNodeValue.

		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		//.____ Geometry ______________________________________________________

		inline SizeI	pixelSize() const { return m_size; }
		inline int		cellSize() const { return m_cellSize; }
		inline SizeI	nodes() const { return m_nodes; }				// Nodes horizontally and vertically.

		inline int		nodeX( int column ) const { return std::min( column * m_cellSize, m_size.w ); }	// Pixel position of a column of nodes.
		inline int		nodeY( int row ) const { return std::min( row * m_cellSize, m_size.h ); }

		//.____ Content _______________________________________________________

		virtual bool	setPeriods( spx uPeriod, spx vPeriod );
		inline spx		uPeriod() const { return m_uPeriod; }
		inline spx		vPeriod() const { return m_vPeriod; }

		// Nodes are stored row by row. Region is given in nodes, pitch is nodes between rows
		// in source/destination, 0 meaning region.w.

		bool			importNodes( const RectI& region, const CoordSPX * pSource, int pitch = 0 );
		bool			exportNodes( const RectI& region, CoordSPX * pDestination, int pitch = 0 ) const;

		inline const CoordSPX*	nodeData() const { return m_pNodes; }

		//.____ Misc __________________________________________________________

		// Corners of a cell (top-left, top-right, bottom-left, bottom-right), with values on periodic
		// axes moved by whole periods to be as close as possible to the top-left corner.

		void			cellCorners( int column, int row, CoordSPX * pCorners ) const;

		// Interpolated position at point x,y (in pixels, 0 -> size) of the Warpmap, in spx of the source
		// with fractions. Not wrapped into the period.

		CoordF			positionAt( double x, double y ) const;

	protected:
		Warpmap() {};
		Warpmap( const Blueprint& bp );
		virtual ~Warpmap();

		static bool		_validateBlueprint( const Blueprint& bp );
		static spx		_unwrapTo( spx value, spx reference, spx period );

		void			_normalizeNodes( const RectI& region );

		// Called when nodes in region or the periods have changed (region is then all nodes).
		// Typically all you need to override when subclassing.

		virtual void	_nodesUpdated( const RectI& region ) = 0;

		SizeI			m_size;
		int				m_cellSize = 16;
		SizeI			m_nodes;
		spx				m_uPeriod = 0;
		spx				m_vPeriod = 0;

		CoordSPX*		m_pNodes = nullptr;					// m_nodes.w * m_nodes.h, row by row.
	};

} // namespace wax
#endif // WAX_WARPMAP_DOT_H
