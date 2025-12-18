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
#include <wg_dummyfont.h>

namespace wg
{
	const TypeInfo DummyFont::TYPEINFO = { "DummyFont", &Font::TYPEINFO };

	//____ constructor ____________________________________________________________

	DummyFont::DummyFont() : Font(nullptr)
	{
	}

	//____ Destructor _____________________________________________________________

	DummyFont::~DummyFont()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DummyFont::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ nbGlyphs() _________________________________________________________

	int DummyFont::nbGlyphs()
	{
		return 0;
	}

	//____ hasGlyphs() ________________________________________________________

	bool DummyFont::hasGlyphs()
	{
		return false;
	}

	//____ isMonospace() ______________________________________________________

	bool DummyFont::isMonospace()
	{
		return true;
	}

	//____ isMonochrome() _____________________________________________________

	bool DummyFont::isMonochrome()
	{
		return true;
	}

	//____ hasGlyph() _________________________________________________________

	bool DummyFont::hasGlyph( uint16_t chr )
	{
		return false;
	}

	//____ setSize() _________________________________________________________

	bool DummyFont::setSize( spx size )
	{
		return false;
	}

	//____ size() _____________________________________________________________

	spx DummyFont::size()
	{
		return 64;
	}

	//____ getGlyphWithoutBitmap() ____________________________________________

	void DummyFont::getGlyphWithoutBitmap(uint16_t chr, Glyph& glyph)
	{
		glyph.fontRef = nullptr;
		glyph.advance = 0;
		glyph.kerningIndex = 0;
	}

	//____ getGlyphWithBitmap() _______________________________________________

	void DummyFont::getGlyphWithBitmap(uint16_t chr, Glyph& glyph)
	{
		glyph.fontRef = nullptr;
		glyph.advance = 0;
		glyph.kerningIndex = 0;
		glyph.pSurface = nullptr;
	}


	//____ kerning() __________________________________________________________

	spx DummyFont::kerning( Glyph& leftGlyph, Glyph& rightGlyph )
	{
		return 0;
	}

	//____ lineGap() __________________________________________________________

	spx DummyFont::lineGap()
	{
		return 64;
	}

	//____ whitespaceAdvance() ________________________________________________

	spx DummyFont::whitespaceAdvance()
	{
		return 64;
	}

	//____ maxAdvance() _______________________________________________________

	spx DummyFont::maxAdvance()
	{
		return 64;
	}

	//____ maxAscend() ________________________________________________________

	spx DummyFont::maxAscend()
	{
		return 64;
	}

	//____ maxDescend() _______________________________________________________

	spx DummyFont::maxDescend()
	{
		return 0;
	}
}
