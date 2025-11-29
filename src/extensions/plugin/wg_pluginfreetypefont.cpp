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
#include <wg_pluginfreetypefont.h>
#include <wg_pluginsurface.h>

namespace wg
{

	const TypeInfo	PluginFreeTypeFont::TYPEINFO = { "PluginFreeTypeFont", &PluginFont::TYPEINFO };


	//____ create() ___________________________________________________________

	PluginFreeTypeFont_p PluginFreeTypeFont::create(wg_obj cFreeTypeFont, PluginBitmapCache* pCache) 
	{ 
		return PluginFreeTypeFont_p(new PluginFreeTypeFont(cFreeTypeFont, pCache)); 
	}

	//____ constructor ____________________________________________________________

	PluginFreeTypeFont::PluginFreeTypeFont( wg_obj cFreeTypeFont, PluginBitmapCache * pCache ) : PluginFont(cFreeTypeFont)
	{
		m_pCache = pCache;
	}

	//____ destructor _____________________________________________________________

	PluginFreeTypeFont::~PluginFreeTypeFont()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginFreeTypeFont::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ getGlyphWithBitmap() ______________________________________________

	void PluginFreeTypeFont::getGlyphWithBitmap(uint16_t ch, Glyph& glyph)
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

			glyph.pSurface = m_pCache->wrapperSurface(cGlyph.surface);
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
