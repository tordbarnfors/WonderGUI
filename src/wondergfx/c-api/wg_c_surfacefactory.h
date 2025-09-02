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

#ifndef WG_C_SURFACEFACTORY_DOT_H
#define WG_C_SURFACEFACTORY_DOT_H
#pragma once

#include <wg_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

//.____ Geometry _________________________________________________

WG_EXPORT wg_sizeI		wg_maxSurfaceSize(wg_obj surfaceFactory);

//.____ Misc _______________________________________________________

WG_EXPORT wg_obj	wg_createSurface(wg_obj factory, const wg_surfaceBP* blueprint);
WG_EXPORT wg_obj	wg_createSurfaceFromBlob(wg_obj factory, const wg_surfaceBP* blueprint, wg_obj blob, int pitch);
WG_EXPORT wg_obj	wg_createSurfaceFromBitmap(	wg_obj factory, const wg_surfaceBP* blueprint, 
												const uint8_t* pPixels, wg_pixelFormat pixelFormat, 
												int pitch, const wg_color8 * pPalette, int paletteSize );
WG_EXPORT wg_obj	wg_createSurfaceFromRawData(wg_obj factory, const wg_surfaceBP* blueprint, 
												const uint8_t* pPixels,	const wg_pixelDescription * pPixelDescription, 
												int pitch, const wg_color8 * pPalette, int paletteSize );

#ifdef __cplusplus
}
#endif

#endif
