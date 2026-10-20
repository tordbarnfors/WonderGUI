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
#include <wg_edgemapshape.h>
#include <wg_gfxdevice.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

namespace wg
{
	const TypeInfo EdgemapShape::TYPEINFO = { "EdgemapShape", &Object::TYPEINFO };

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& EdgemapShape::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ bounds() ______________________________________________________________
	/**
	 * @brief Rectangle covered by the pieces of the shape.
	 */

	RectSPX EdgemapShape::bounds() const
	{
		if( m_pieces.empty() )
			return RectSPX();

		RectSPX bounds = _pieceRect(m_pieces[0]);

		for( size_t i = 1 ; i < m_pieces.size() ; i++ )
			bounds.growToContain(_pieceRect(m_pieces[i]));

		return bounds;
	}

	//____ draw() ________________________________________________________________
	/**
	 * @brief Draw the shape.
	 *
	 * Draws the edgemaps of all pieces, moved by offset, which is rounded to whole pixels.
	 */

	void EdgemapShape::draw( GfxDevice * pDevice, CoordSPX offset ) const
	{
		offset = Util::roundToPixels(offset) * 64;

		for( auto& piece : m_pieces )
			pDevice->flipDrawEdgemap(piece.pos + offset, piece.pEdgemap, piece.flip);
	}

	//____ _pieceRect() __________________________________________________________

	RectSPX EdgemapShape::_pieceRect( const Piece& piece )
	{
		return { piece.pos, Util::flipSize(piece.pEdgemap->pixelSize(), piece.flip) * 64 };
	}

	//____ _factoryOrDefault() ___________________________________________________

	EdgemapFactory_p EdgemapShape::_factoryOrDefault( EdgemapFactory * pFactory )
	{
		return pFactory ? EdgemapFactory_p(pFactory) : GfxBase::defaultEdgemapFactory();
	}

} // namespace wg
