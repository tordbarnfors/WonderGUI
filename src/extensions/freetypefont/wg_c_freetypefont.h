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


#ifndef WG_C_FREETYPEFONT_DOT_H
#define WG_C_FREETYPEFONT_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	WG_FT_MONOCHROME,
	WG_FT_CRISPEDGES,
	WG_FT_BESTSHAPES

} wg_renderMode;


//____ wg_freeTypeFontBP __________________________________________________________

struct wg_freeTypeFontBP_struct			// NOT BINARY EQUIVALENT!
{
	wg_obj			backupFont;
	wg_obj			blob;
	wg_obj			cache;
	int				faceIndex;
	wg_renderMode	renderMode;
	int				stemDarkening;
	int				xDPI;
	int				yDPI;
} wg_freeTypeFontBP_default = { 0, 0, 0, 0, WG_FT_BESTSHAPES, false, 72, 72 };


typedef struct wg_freeTypeFontBP_struct	wg_freeTypeFontBP;

WG_EXPORT wg_obj	wg_createFreeTypeFont( wg_freeTypeFontBP blueprint );
WG_EXPORT wg_obj	wg_getFreeTypeFontBitmapCache( wg_obj font );


#ifdef __cplusplus
}
#endif

#endif //WG_C_FREETYPEFONT_DOT_H
