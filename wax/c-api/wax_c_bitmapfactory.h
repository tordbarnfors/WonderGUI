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
#ifndef WAX_C_BITMAPFACTORY_DOT_H
#define WAX_C_BITMAPFACTORY_DOT_H
#pragma once

#include <wax_c_bitmap.h>

#ifdef __cplusplus
extern "C" {
#endif

//.____ Geometry _________________________________________________

WAX_EXPORT wax_sizeI		wax_maxBitmapSize(wax_obj bitmapFactory);

//.____ Misc _______________________________________________________

WAX_EXPORT wax_obj	wax_createBitmap(wax_obj factory, const wax_bitmapBP* blueprint);
WAX_EXPORT wax_obj	wax_createBitmapFromBlob(wax_obj factory, const wax_bitmapBP* blueprint, wax_obj blob, int pitch);
WAX_EXPORT wax_obj	wax_createBitmapFromBitmap(	wax_obj factory, const wax_bitmapBP* blueprint, 
												const uint8_t* pPixels, wax_pixelFormat pixelFormat, 
												int pitch, const wax_color8 * pPalette, int paletteSize );
WAX_EXPORT wax_obj	wax_createBitmapFromRawData(wax_obj factory, const wax_bitmapBP* blueprint, 
												const uint8_t* pPixels,	const wax_pixelDescription * pPixelDescription, 
												int pitch, const wax_color8 * pPalette, int paletteSize );

#ifdef __cplusplus
}
#endif

#endif
