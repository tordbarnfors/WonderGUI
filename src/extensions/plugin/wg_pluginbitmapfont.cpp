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
#include <wg_plugincalls.h>
#include <wg_pluginbitmapfont.h>
#include <wg_pluginsurface.h>

#include <cassert>

namespace wg
{

	const TypeInfo	PluginBitmapFont::TYPEINFO = { "PluginBitmapFont", &PluginFont::TYPEINFO };


	//____ constructor ____________________________________________________________

	PluginBitmapFont::PluginBitmapFont( wg_obj cBitmapFont ) : PluginFont(cBitmapFont)
	{
		wg_obj surface = PluginCalls::bitmapFont->getBitmapFontSurface(cBitmapFont);
		
		// PluginBitmapFont doesn't support backup fonts.
		assert( PluginCalls::font->getBackupFont(cBitmapFont) == 0 );
		
		m_pSurface = PluginSurface::create(surface);
	}

	//____ Destructor _____________________________________________________________

	PluginBitmapFont::~PluginBitmapFont()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginBitmapFont::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ getGlyphWithBitmap() ______________________________________________

	void PluginBitmapFont::getGlyphWithBitmap(uint16_t ch, Glyph& glyph)
	{
		wg_glyph	cGlyph;

		PluginCalls::font->getGlyphWithBitmap(m_cFont, ch, &cGlyph);

		if (cGlyph.advance != 0)
		{
			glyph.advance = cGlyph.advance;
			glyph.fontRef = cGlyph.fontRef;
			glyph.kerningIndex = cGlyph.kerningIndex;

			glyph.rect		= * (RectSPX*) &cGlyph.rect;
			glyph.bearingX	= cGlyph.bearingX;
			glyph.bearingY	= cGlyph.bearingY;

			glyph.pSurface 	= m_pSurface;
		}
		else
		{
			glyph.advance = 0;
			glyph.fontRef = 0;
			glyph.kerningIndex = 0;
			glyph.pSurface = nullptr;
		}

		return;
	}

} // namespace wg
