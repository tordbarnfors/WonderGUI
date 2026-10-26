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
#ifndef WAX_C_FREETYPEFONT_DOT_H
#define WAX_C_FREETYPEFONT_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	WAX_FT_MONOCHROME,
	WAX_FT_CRISPEDGES,
	WAX_FT_BESTSHAPES

} wax_renderMode;


//____ wax_freeTypeFontBP __________________________________________________________

struct wax_freeTypeFontBP_struct			// NOT BINARY EQUIVALENT!
{
	wax_obj			backupFont;
	wax_obj			blob;
	wax_obj			cache;
	int				faceIndex;
	wax_renderMode	renderMode;
	int				stemDarkening;
	int				xDPI;
	int				yDPI;
} wax_freeTypeFontBP_default = { 0, 0, 0, 0, WAX_FT_BESTSHAPES, false, 72, 72 };


typedef struct wax_freeTypeFontBP_struct	wax_freeTypeFontBP;

WAX_EXPORT wax_obj	wax_createFreeTypeFont( wax_freeTypeFontBP blueprint );
WAX_EXPORT wax_obj	wax_getFreeTypeFontBitmapCache( wax_obj font );


#ifdef __cplusplus
}
#endif

#endif //WAX_C_FREETYPEFONT_DOT_H
