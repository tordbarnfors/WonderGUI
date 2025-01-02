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
#ifndef WG_C_SOFTGFX_DOT_H
#define WG_C_SOFTGFX_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_surface.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_obj	wg_createSoftGfxDevice(void);
	int		wg_defineSoftGfxDeviceCanvas( wg_obj device, wg_canvasRef ref, wg_obj softSurface );

	wg_obj	wg_createSoftBackend(void);
	int		wg_defineSoftBackendCanvas( wg_obj backend, wg_canvasRef ref, wg_obj softSurface );


	wg_obj	wg_createSoftSurface(const wg_surfaceBP* blueprint);
	wg_obj	wg_createSoftSurfaceFromBlob(const wg_surfaceBP* blueprint, wg_obj blob, int pitch);
	wg_obj	wg_createSoftSurfaceFromBitmap(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
										   wg_pixelFormat pixelFormat, int pitch, const wg_color8 * pPalette );
	wg_obj	wg_createSoftSurfaceFromRawData(wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
											const wg_pixelDescription * pPixelDescription, int pitch, const wg_color8 * pPalette );

	wg_obj	wg_createSoftSurfaceFactory(void);


#ifdef __cplusplus
}
#endif

#endif // WG_C_SOFTGFX_DOT_H
