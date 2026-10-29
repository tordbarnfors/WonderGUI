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
#ifndef WAX_EDGEMAPSHAPE_DOT_H
#define WAX_EDGEMAPSHAPE_DOT_H
#pragma once

#include <vector>
#include <functional>

#include <wax_object.h>
#include <wax_edgemap.h>
#include <wax_edgemapfactory.h>
#include <wax_types.h>
#include <wax_geo.h>

namespace wax
{
	class Device;

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

		void		draw( Device * pDevice, CoordSPX offset = { 0,0 } ) const;

	protected:
		EdgemapShape() {}
		virtual ~EdgemapShape() {}

		static RectSPX			_pieceRect( const Piece& piece );
		static EdgemapFactory_p	_factoryOrDefault( EdgemapFactory * pFactory );

		//.____ Building pieces _____________________________________________
		//
		// A subclass can describe its shape by the regions met along lines through it and let
		// _buildPieces() make the edgemaps of the pieces it lays out. The edgemap of a piece runs
		// either along canvas rows (drawn transposed) or along canvas columns. Samples are taken
		// along the lines between its rows or columns, and the regions between them become the
		// segments of the edgemap.
		//
		// Since edges are straight between samples, pieces should be laid out so that curves run
		// mostly along their lines.

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

		struct Layout
		{
			RectI		rect;			// Pixels on canvas.
			bool		bRows = true;	// Edgemap runs along canvas rows, otherwise along columns.
		};

		enum class BuildResult
		{
			Ok,
			TooManySegments,			// Regions don't fit the segments of an edgemap. Nothing is changed.
			Failed						// An edgemap couldn't be created.
		};

		BuildResult	_buildPieces( const std::vector<Layout>& layout, const RegionFunc& regions, const HiColor * pColors, EdgemapFactory * pFactory );

		struct Build				// A piece built, before it is made into an edgemap.
		{
			RectI					rect;
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

		bool		_buildPiece( const Layout& layout, const RegionFunc& regions, const HiColor * pColors, Build& out );
		bool		_applyPieces( std::vector<Build>& builds, EdgemapFactory * pFactory );

		std::vector<Piece>	m_pieces;
		std::vector<Stored>	m_stored;			// Content of the edgemap of each piece made by _buildPieces().
	};

} // namespace wax
#endif //WAX_EDGEMAPSHAPE_DOT_H
