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
#include <wg_c_font.h>
#include <wg_c_internal.h>
#include <wg_font.h>

using namespace wg;

inline Font* getPtr(wg_obj obj) {
	return static_cast<Font*>(reinterpret_cast<Object*>(obj));
}

int wg_setFontSize(wg_obj font, wg_spx size)
{
	return getPtr(font)->setSize((spx)size);
}


wg_spx wg_fontSize(wg_obj font)
{
	return getPtr(font)->size();
}


void wg_getGlyphWithoutBitmap(wg_obj font, uint16_t chr, wg_glyph* pGlyph)
{
	Glyph glyph;

	getPtr(font)->getGlyphWithoutBitmap(chr, glyph);

	pGlyph->advance = glyph.advance;
	pGlyph->kerningIndex = glyph.kerningIndex;
	pGlyph->fontRef = glyph.fontRef;
}


void wg_getGlyphWithBitmap(wg_obj font, uint16_t chr, wg_glyph* pGlyph)
{
	Glyph glyph;

	getPtr(font)->getGlyphWithBitmap(chr, glyph);

	pGlyph->advance = glyph.advance;
	pGlyph->kerningIndex = glyph.kerningIndex;
	pGlyph->fontRef = glyph.fontRef;
	pGlyph->bearingX = glyph.bearingX;
	pGlyph->bearingY = glyph.bearingY;
	pGlyph->rect = * (wg_rectSPX*) &glyph.rect;
	pGlyph->surface = static_cast<Object*>(glyph.pSurface.rawPtr());
}


wg_spx wg_getKerning(wg_obj font, wg_glyph* pLeftGlyph, wg_glyph* pRightGlyph)
{
	Glyph left, right;

	left.advance = pLeftGlyph->advance;
	left.kerningIndex = pLeftGlyph->kerningIndex;
	left.fontRef = pLeftGlyph->fontRef;

	right.advance = pRightGlyph->advance;
	right.kerningIndex = pRightGlyph->kerningIndex;
	right.fontRef = pRightGlyph->fontRef;

	return (wg_spx) getPtr(font)->kerning(left, right);
}


wg_spx wg_lineGap(wg_obj font)
{
	return (wg_spx)getPtr(font)->lineGap();
}


wg_spx wg_whitespaceAdvance(wg_obj font)
{
	return (wg_spx)getPtr(font)->whitespaceAdvance();
}


wg_spx wg_maxAdvance(wg_obj font)
{
	return (wg_spx)getPtr(font)->maxAdvance();
}


wg_spx wg_maxAscend(wg_obj font)
{
	return (wg_spx)getPtr(font)->maxAscend();
}


wg_spx wg_maxDescend(wg_obj font)
{
	return (wg_spx)getPtr(font)->maxDescend();
}


int wg_nbGlyphs(wg_obj font)
{
	return getPtr(font)->nbGlyphs();

}


int wg_hasGlyphs(wg_obj font)
{
	return getPtr(font)->hasGlyphs();
}


int wg_hasGlyph(wg_obj font, uint16_t chr)
{
	return getPtr(font)->hasGlyph(chr);
}


int wg_isMonospace(wg_obj font)
{
	return getPtr(font)->isMonospace();
}


int wg_isMonochrome(wg_obj font)
{
	return getPtr(font)->isMonochrome();
}


wg_obj wg_getBackupFont(wg_obj font)
{
	auto p = getPtr(font)->backupFont();
	return static_cast<Object*>(p.rawPtr());
}


wg_obj wg_createSurfaceFromGlyph(wg_obj font, uint16_t chr, const wg_surfaceBP* blueprint, wg_obj surfaceFactory)
{
	Surface::Blueprint bp;

	convertSurfaceBlueprint(&bp, blueprint);

	auto p = getPtr(font)->getGlyphAsSurface(chr, bp, static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(surfaceFactory)));
	p->retain();
	return static_cast<Object*>(p.rawPtr());
}
