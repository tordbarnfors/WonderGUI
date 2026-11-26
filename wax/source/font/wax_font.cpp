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
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <wax_font.h>
#include <wax_base.h>

namespace wax
{

	const TypeInfo Font::TYPEINFO = { "Font", &Object::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Font::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ isMonochrome() ______________________________________________________

	bool Font::isMonochrome()
	{
		return true;
	}

	//____ getGlyphAsBitmap() ___________________________________________________

	Bitmap_p Font::getGlyphAsBitmap(char32_t chr, const Bitmap::Blueprint& _bp, BitmapFactory * pFactory )
	{
		Glyph glyph;
		getGlyphWithBitmap(chr, glyph);
		
		if( glyph.pBitmap == nullptr )
			return nullptr;
		
		if( pFactory == nullptr )
			pFactory = Base::defaultBitmapFactory();
		
		if( !pFactory )
			return nullptr;
		
		RectI pixelRect = glyph.rect/64;
		
		Bitmap::Blueprint bp = _bp;
		if( bp.size.isEmpty() )
			bp.size = pixelRect.size();
		
		auto pBitmap = pFactory->createBitmap(bp);
		pBitmap->copy({ 0,0 }, glyph.pBitmap, pixelRect );
		return pBitmap;
	}

	//____ setBackupFont() _______________________________________________________

	void Font::setBackupFont(Font* pFont)
	{
		Font* p = pFont;
		while(p != this && p != nullptr )
			p = p->backupFont();

		if (p == this)
		{
			Base::reportError( ErrorLevel::Error, ErrorCode::InvalidParam, "Can't set backup font, would create cyclic fallback.", this, &TYPEINFO, __func__, __FILE__, __LINE__ );
			return;
		}

		m_pBackupFont = pFont;
	}



} // namespace wax
