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
	 * the position in the source that is drawn at that point, normalized so that 0.0 -> 1.0 covers
	 * the source, u along its width and v along its height. Positions between nodes are interpolated
	 * bilinearly. Where a position falls outside 0.0 -> 1.0 nothing is drawn.
	 *
	 * Node values are floats and not limited to 0.0 -> 1.0. Values outside that range are what makes
	 * parts of the Warpmap transparent, with the border antialiased where the interpolated position
	 * crosses 0.0 or 1.0. Values on a periodic axis are brought into 0.0 -> period when imported.
	 * Backends convert to fixed point per cell corner when drawing, not per pixel. A new Warpmap
	 * maps its rectangle straight onto the source, u = x / size.w and v = y / size.h.
	 *
	 * Smaller cells follow curved shapes more closely. A circle of radius r is off by
	 * roughly cellSize^2 / (8 * r) pixels.
	 *
	 * An axis can be periodic (uPeriod/vPeriod > 0), as when u is the angle of a ring. Positions are
	 * then read modulo the period and interpolation between nodes across the seam takes the short way:
	 * the corners of a cell are taken as the values closest to its top-left corner, give or take whole
	 * periods. With a period of 1.0 the source wraps around, which for an edgemap is seamless if it is
	 * tiling. A period larger than 1.0 leaves a gap, such as a 270 degree arc having uPeriod = 360/270.
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
	 *   is in, see positionAt(). Adding the source offset and multiplying by the size of the source
	 *   gives the position in the source, in its pixels. A periodic axis is then wrapped into
	 *   0 -> period * size.
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
			float			uPeriod		= 0.f;				// 0 = not periodic.
			float			vPeriod		= 0.f;				// 0 = not periodic.
		};

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

		virtual bool	setPeriods( float uPeriod, float vPeriod );
		inline float	uPeriod() const { return m_uPeriod; }
		inline float	vPeriod() const { return m_vPeriod; }

		// Nodes are stored row by row. Region is given in nodes, pitch is nodes between rows
		// in source/destination, 0 meaning region.w.

		bool			importNodes( const RectI& region, const CoordF * pSource, int pitch = 0 );
		bool			exportNodes( const RectI& region, CoordF * pDestination, int pitch = 0 ) const;

		inline const CoordF*	nodeData() const { return m_pNodes; }

		//.____ Misc __________________________________________________________

		// Corners of a cell (top-left, top-right, bottom-left, bottom-right), with values on periodic
		// axes moved by whole periods to be as close as possible to the top-left corner.

		void			cellCorners( int column, int row, CoordF * pCorners ) const;

		// Interpolated, normalized position at point x,y (in pixels, 0 -> size) of the Warpmap. Not
		// wrapped into the period.

		CoordF			positionAt( double x, double y ) const;

	protected:
		Warpmap() {};
		Warpmap( const Blueprint& bp );
		virtual ~Warpmap();

		static bool		_validateBlueprint( const Blueprint& bp );
		static float	_unwrapTo( float value, float reference, float period );

		void			_normalizeNodes( const RectI& region );

		// Called when nodes in region or the periods have changed (region is then all nodes).
		// Typically all you need to override when subclassing.

		virtual void	_nodesUpdated( const RectI& region ) = 0;

		SizeI			m_size;
		int				m_cellSize = 16;
		SizeI			m_nodes;
		float			m_uPeriod = 0.f;
		float			m_vPeriod = 0.f;

		CoordF*			m_pNodes = nullptr;					// m_nodes.w * m_nodes.h, row by row.
	};

} // namespace wax
#endif // WAX_WARPMAP_DOT_H
