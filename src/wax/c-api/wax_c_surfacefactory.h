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
#ifndef WAX_C_SURFACEFACTORY_DOT_H
#define WAX_C_SURFACEFACTORY_DOT_H
#pragma once

#include <wax_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

//.____ Geometry _________________________________________________

WAX_EXPORT wax_sizeI		wax_maxSurfaceSize(wax_obj surfaceFactory);

//.____ Misc _______________________________________________________

WAX_EXPORT wax_obj	wax_createSurface(wax_obj factory, const wax_surfaceBP* blueprint);
WAX_EXPORT wax_obj	wax_createSurfaceFromBlob(wax_obj factory, const wax_surfaceBP* blueprint, wax_obj blob, int pitch);
WAX_EXPORT wax_obj	wax_createSurfaceFromBitmap(	wax_obj factory, const wax_surfaceBP* blueprint, 
												const uint8_t* pPixels, wax_pixelFormat pixelFormat, 
												int pitch, const wax_color8 * pPalette, int paletteSize );
WAX_EXPORT wax_obj	wax_createSurfaceFromRawData(wax_obj factory, const wax_surfaceBP* blueprint, 
												const uint8_t* pPixels,	const wax_pixelDescription * pPixelDescription, 
												int pitch, const wax_color8 * pPalette, int paletteSize );

#ifdef __cplusplus
}
#endif

#endif
