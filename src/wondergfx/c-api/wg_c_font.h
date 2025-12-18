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
#ifndef WG_C_FONT_DOT_H
#define WG_C_FONT_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_color.h>
#include <wg_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_glyph ______________________________________________________________

	typedef struct wg_glyph_struct		// NOT BINARY EQUIVALENT!!!
	{
		wg_spx		advance;
		uint32_t	kerningIndex;
		void *		fontRef;

		// These below are not updated by getGlyphWithoutBitmap().

		wg_obj		surface;
		wg_rectSPX	rect;
		wg_spx		bearingX;
		wg_spx		bearingY;
	} wg_glyph;


	//____ Rendering ____________________________________________

	WG_EXPORT int		wg_setFontSize(wg_obj font, wg_spx size);
	WG_EXPORT wg_spx	wg_fontSize(wg_obj font);
	WG_EXPORT void		wg_getGlyphWithoutBitmap(wg_obj font, uint16_t chr, wg_glyph * pGlyph);
	WG_EXPORT void		wg_getGlyphWithBitmap(wg_obj font, uint16_t chr, wg_glyph * pGlyph);
	WG_EXPORT wg_spx	wg_getKerning(wg_obj font, wg_glyph * pLeftGlyph, wg_glyph * p_rightGlyph);

	WG_EXPORT wg_spx	wg_lineGap(wg_obj font);
	WG_EXPORT wg_spx	wg_whitespaceAdvance(wg_obj font);
	WG_EXPORT wg_spx	wg_maxAdvance(wg_obj font);
	WG_EXPORT wg_spx	wg_maxAscend(wg_obj font);
	WG_EXPORT wg_spx 	wg_maxDescend(wg_obj font);

	//____ Misc ___________________________________________________________

	WG_EXPORT int		wg_nbGlyphs(wg_obj font);
	WG_EXPORT int		wg_hasGlyphs(wg_obj font);
	WG_EXPORT int		wg_hasGlyph(wg_obj font, uint16_t chr);
	WG_EXPORT int		wg_isMonospace(wg_obj font);
	WG_EXPORT int		wg_isMonochrome(wg_obj font);

	WG_EXPORT wg_obj	wg_getBackupFont(wg_obj font);

	WG_EXPORT wg_obj	wg_createSurfaceFromGlyph(wg_obj font, uint16_t chr, const wg_surfaceBP* blueprint, wg_obj surfaceFactory);



#ifdef __cplusplus
}
#endif

#endif
