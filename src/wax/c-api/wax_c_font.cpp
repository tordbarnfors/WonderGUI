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
#include <wax_c_font.h>
#include <wax_c_internal.h>
#include <wax_font.h>

using namespace wax;

inline Font* getPtr(wax_obj obj) {
	return static_cast<Font*>(reinterpret_cast<Object*>(obj));
}

int wax_setFontSize(wax_obj font, wax_spx size)
{
	return getPtr(font)->setSize((spx)size);
}


wax_spx wax_fontSize(wax_obj font)
{
	return getPtr(font)->size();
}


void wax_getGlyphWithoutBitmap(wax_obj font, uint32_t chr, wax_glyph* pGlyph)
{
	Glyph glyph;

	getPtr(font)->getGlyphWithoutBitmap(chr, glyph);

	pGlyph->advance = glyph.advance;
	pGlyph->kerningIndex = glyph.kerningIndex;
	pGlyph->fontRef = glyph.fontRef;
}


void wax_getGlyphWithBitmap(wax_obj font, uint32_t chr, wax_glyph* pGlyph)
{
	Glyph glyph;

	getPtr(font)->getGlyphWithBitmap(chr, glyph);

	pGlyph->advance = glyph.advance;
	pGlyph->kerningIndex = glyph.kerningIndex;
	pGlyph->fontRef = glyph.fontRef;
	pGlyph->bearingX = glyph.bearingX;
	pGlyph->bearingY = glyph.bearingY;
	pGlyph->rect = * (wax_rectSPX*) &glyph.rect;
	pGlyph->surface = static_cast<Object*>(glyph.pSurface.rawPtr());
}


wax_spx wax_getKerning(wax_obj font, wax_glyph* pLeftGlyph, wax_glyph* pRightGlyph)
{
	Glyph left, right;

	left.advance = pLeftGlyph->advance;
	left.kerningIndex = pLeftGlyph->kerningIndex;
	left.fontRef = pLeftGlyph->fontRef;

	right.advance = pRightGlyph->advance;
	right.kerningIndex = pRightGlyph->kerningIndex;
	right.fontRef = pRightGlyph->fontRef;

	return (wax_spx) getPtr(font)->kerning(left, right);
}


wax_spx wax_lineGap(wax_obj font)
{
	return (wax_spx)getPtr(font)->lineGap();
}


wax_spx wax_whitespaceAdvance(wax_obj font)
{
	return (wax_spx)getPtr(font)->whitespaceAdvance();
}


wax_spx wax_maxAdvance(wax_obj font)
{
	return (wax_spx)getPtr(font)->maxAdvance();
}


wax_spx wax_maxAscend(wax_obj font)
{
	return (wax_spx)getPtr(font)->maxAscend();
}


wax_spx wax_maxDescend(wax_obj font)
{
	return (wax_spx)getPtr(font)->maxDescend();
}


int wax_nbGlyphs(wax_obj font)
{
	return getPtr(font)->nbGlyphs();

}


int wax_hasGlyphs(wax_obj font)
{
	return getPtr(font)->hasGlyphs();
}


int wax_hasGlyph(wax_obj font, uint32_t chr)
{
	return getPtr(font)->hasGlyph(chr);
}


int wax_isMonospace(wax_obj font)
{
	return getPtr(font)->isMonospace();
}


int wax_isMonochrome(wax_obj font)
{
	return getPtr(font)->isMonochrome();
}


wax_obj wax_getBackupFont(wax_obj font)
{
	auto p = getPtr(font)->backupFont();
	return static_cast<Object*>(p.rawPtr());
}


wax_obj wax_createSurfaceFromGlyph(wax_obj font, uint32_t chr, const wax_surfaceBP* blueprint, wax_obj surfaceFactory)
{
	Surface::Blueprint bp;

	convertSurfaceBlueprint(&bp, blueprint);

	auto p = getPtr(font)->getGlyphAsSurface(chr, bp, static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(surfaceFactory)));
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}
