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
#ifndef WG_EDGEMAPSHAPE_DOT_H
#define WG_EDGEMAPSHAPE_DOT_H
#pragma once

#include <vector>
#include <functional>
#include <climits>

#include <wg_object.h>
#include <wg_edgemap.h>
#include <wg_edgemapfactory.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>

namespace wg
{
	class GfxDevice;

	class EdgemapShape;
	typedef	StrongPtr<EdgemapShape>	EdgemapShape_p;
	typedef	WeakPtr<EdgemapShape>	EdgemapShape_wp;

	//____ EdgemapShape _______________________________________________________
	/**
	 * @brief Base class for shapes made into edgemaps.
	 *
	 * The shape is made into edgemaps when created or updated, so drawing it is just drawing
	 * edgemaps. With a StreamEdgemapFactory the edgemaps are created on the receiving end
	 * of the stream, which then only needs to draw them.
	 *
	 * A shape consists of one or more pieces, each an edgemap drawn flipped at a position
	 * of the canvas. Pieces don't overlap.
	 *
	 * Positions are given in spx, but edgemaps are drawn at whole pixels, so positions of
	 * pieces are always whole pixels and the offset given to draw() is rounded.
	 *
	 * Subclasses create and update the pieces.
	 */

	class EdgemapShape : public Object
	{
	public:

		struct Piece
		{
			Edgemap_p	pEdgemap;
			CoordSPX	pos;						// Top-left corner on canvas, after flip. Whole pixels.
			GfxFlip		flip = GfxFlip::None;
		};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		RectSPX		bounds() const;

		//.____ Content _____________________________________________________

		const std::vector<Piece>&	pieces() const { return m_pieces; }

		//.____ Rendering ___________________________________________________

		void		draw( GfxDevice * pDevice, CoordSPX offset = { 0,0 } ) const;

	protected:
		EdgemapShape() {}
		virtual ~EdgemapShape() {}

		static RectSPX			_pieceRect( const Piece& piece );
		static EdgemapFactory_p	_factoryOrDefault( EdgemapFactory * pFactory );

		//.____ Building pieces _____________________________________________
		//
		// A subclass can describe its shape by the regions met along lines through it and let
		// _buildPieces() make the pieces. The shape is cut into cells along a grid. A cell becomes
		// part of a piece whose edgemap either runs along canvas rows (drawn transposed) or along
		// canvas columns. Samples are taken along the lines between its rows or columns, and the
		// regions between them are the segments of the edgemap.
		//
		// Since edges are straight between samples, the grid should keep curves within cells
		// running mostly along the lines and boundaries across the lines on the grid.

		struct Region
		{
			uint32_t	key;			// Identifies the region. Unique along a line, the same on all lines through a piece.
			int			color;			// Index of the region's color, -1 if nothing is drawn.
			double		end;			// Where it ends along the line, in pixels from the start of the line.
		};

		// Regions along a line through a piece: a row at y = pos from x = begin to x = end if bRow,
		// otherwise a column at x = pos from y = begin to y = end. Lines at the edges of a piece are
		// moved a tiny bit into it.

		using RegionFunc = std::function<void( const RectI& piece, bool bRow, double pos, double begin, double end, std::vector<Region>& regions )>;

		struct Grid
		{
			std::vector<int>	x;		// Pixel columns between cells, ascending.
			std::vector<int>	y;		// Pixel rows between cells, ascending.

			// Lines that pieces can't span, where a boundary runs exactly along a line between samples,
			// or where a piece should begin or end: rows that pieces along rows can't span and columns
			// that pieces along columns can't span, if the piece reaches between begin and end.

			struct Barrier
			{
				int		line;
				int		begin = INT_MIN;
				int		end = INT_MAX;
			};

			std::vector<Barrier>	barrierRows;
			std::vector<Barrier>	barrierColumns;

			std::function<int( const RectI& cell )>	orientation;	// 1 for rows, 0 for columns, -1 for nothing.
		};

		bool		_buildPieces( const Grid& grid, const RegionFunc& regions, const HiColor * pColors, EdgemapFactory * pFactory );

		struct Build				// A piece built, before it is made into an edgemap.
		{
			RectI					rect;					// Pixels on canvas.
			bool					bRows = true;
			bool					bEmpty = true;			// Nothing is drawn.
			std::vector<HiColor>	colors;					// Of the segments.
			std::vector<spx>		samples;				// Edges of the first sample, then of the second and so on.
		};

		struct Stored				// What the edgemap of a piece holds.
		{
			RectI					rect;
			bool					bRows = true;
			int						capacity = 0;			// Segments of the edgemap, some may be unused.
			std::vector<HiColor>	colors;
			std::vector<spx>		samples;
		};

		struct Description
		{
			const RegionFunc&		regions;
			const HiColor *			pColors;
		};

		bool		_buildPiece( const RectI& rect, bool bRows, const Description& shape, bool bForce, Build& out );
		void		_buildCell( const RectI& rect, bool bRows, const Description& shape, std::vector<Build>& out );
		bool		_applyPieces( std::vector<Build>& builds, EdgemapFactory * pFactory );

		std::vector<Piece>	m_pieces;
		std::vector<Stored>	m_stored;			// Content of the edgemap of each piece made by _buildPieces().
	};

} // namespace wg
#endif //WG_EDGEMAPSHAPE_DOT_H
