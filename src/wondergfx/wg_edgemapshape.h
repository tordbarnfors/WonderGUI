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

		std::vector<Piece>	m_pieces;
	};

} // namespace wg
#endif //WG_EDGEMAPSHAPE_DOT_H
