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
#ifndef WAX_C_FONT_DOT_H
#define WAX_C_FONT_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_color.h>
#include <wax_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_glyph ______________________________________________________________

	typedef struct wax_glyph_struct		// NOT BINARY EQUIVALENT!!!
	{
		wax_spx		advance;
		uint32_t	kerningIndex;
		void *		fontRef;

		// These below are not updated by getGlyphWithoutBitmap().

		wax_obj		surface;
		wax_rectSPX	rect;
		wax_spx		bearingX;
		wax_spx		bearingY;
	} wax_glyph;


	//____ Rendering ____________________________________________

	WAX_EXPORT int		wax_setFontSize(wax_obj font, wax_spx size);
	WAX_EXPORT wax_spx	wax_fontSize(wax_obj font);
	WAX_EXPORT void		wax_getGlyphWithoutBitmap(wax_obj font, uint32_t chr, wax_glyph * pGlyph);
	WAX_EXPORT void		wax_getGlyphWithBitmap(wax_obj font, uint32_t chr, wax_glyph * pGlyph);
	WAX_EXPORT wax_spx	wax_getKerning(wax_obj font, wax_glyph * pLeftGlyph, wax_glyph * p_rightGlyph);

	WAX_EXPORT wax_spx	wax_lineGap(wax_obj font);
	WAX_EXPORT wax_spx	wax_whitespaceAdvance(wax_obj font);
	WAX_EXPORT wax_spx	wax_maxAdvance(wax_obj font);
	WAX_EXPORT wax_spx	wax_maxAscend(wax_obj font);
	WAX_EXPORT wax_spx 	wax_maxDescend(wax_obj font);

	//____ Misc ___________________________________________________________

	WAX_EXPORT int		wax_nbGlyphs(wax_obj font);
	WAX_EXPORT int		wax_hasGlyphs(wax_obj font);
	WAX_EXPORT int		wax_hasGlyph(wax_obj font, uint32_t chr);
	WAX_EXPORT int		wax_isMonospace(wax_obj font);
	WAX_EXPORT int		wax_isMonochrome(wax_obj font);

	WAX_EXPORT wax_obj	wax_getBackupFont(wax_obj font);

	WAX_EXPORT wax_obj	wax_createSurfaceFromGlyph(wax_obj font, uint32_t chr, const wax_surfaceBP* blueprint, wax_obj surfaceFactory);



#ifdef __cplusplus
}
#endif

#endif
