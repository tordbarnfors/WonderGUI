/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_C_FONT_DOT_H
#define WG_C_FONT_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>
#include <wg_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif



	struct wg_glyph		// NOT BINARY EQUIVALENT!!!
	{
		wg_spx		advance;
		uint32_t	kerningIndex;
		void *		fontRef;

		// These below are not updated by getGlyphWithoutBitmap().

		wg_obj		surface;
		wg_rectSPX	rect;
		wg_spx		bearingX;
		wg_spx		bearingY;
	};

	//.____ Rendering ____________________________________________

	int				wg_setFontSize(wg_obj font, wg_spx size);
	wg_spx			wg_fontSize(wg_obj font);
	void			wg_getGlyphWithoutBitmap(wg_obj font, uint16_t chr, wg_glyph * pGlyph);
	void			wg_getGlyphWithBitmap(wg_obj font, uint16_t chr, wg_glyph * pGlyph);
	wg_spx			wg_getKerning(wg_obj font, wg_glyph * pLeftGlyph, wg_glyph * p_rightGlyph);

	wg_spx			wg_lineGap(wg_obj font);
	wg_spx			wg_whitespaceAdvance(wg_obj font);
	wg_spx			wg_maxAdvance(wg_obj font);
	wg_spx			wg_maxAscend(wg_obj font);
	wg_spx 			wg_maxDescend(wg_obj font);

	//.____ Misc ___________________________________________________________

	int				wg_nbGlyphs(wg_obj font);
	int				wg_hasGlyphs(wg_obj font);
	int				wg_hasGlyph(wg_obj font, uint16_t chr);
	int				wg_isMonospace(wg_obj font);
	int				wg_isMonochrome(wg_obj font);

	wg_obj			wg_backupFont(wg_obj font);

	wg_obj			wg_getGlyphAsSurface(wg_obj font, uint16_t chr, const wg_surfaceBP* blueprint, wg_obj surfaceFactory);



#ifdef __cplusplus
}
#endif

#endif
