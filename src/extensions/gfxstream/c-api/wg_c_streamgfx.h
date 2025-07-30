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
#ifndef WG_C_STREAMGFX_DOT_H
#define WG_C_STREAMGFX_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_gfxstream.h>
#include <wg_c_surface.h>
#include <wg_c_edgemap.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_obj			wg_createStreamBackend(wg_obj streamEncoder, int maxEdges );
	int				wg_defineStreamBackendCanvasWithSurface( wg_obj streamBackend, wg_canvasRef ref, wg_obj surface );
	int				wg_defineStreamBackendCanvas( wg_obj streamBackend, wg_canvasRef ref, wg_sizeI pixelSize, wg_pixelFormat pixelFormat, int scale );

	wg_obj			wg_createStreamSurface(wg_obj streamEncoder, wg_surfaceBP* blueprint);
	wg_obj			wg_createStreamSurfaceFromBlob(wg_obj streamEncoder, const wg_surfaceBP* blueprint, wg_obj blob, int pitch);
	wg_obj			wg_createStreamSurfaceFromBitmap(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
												   wg_pixelFormat pixelFormat, int pitch, const wg_color8 * pPalette );
	wg_obj			wg_createStreamSurfaceFromRawData(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
													const wg_pixelDescription * pPixelDescription, int pitch, const wg_color8 * pPalette );

	wg_obj			wg_createStreamSurfaceFactory(wg_obj streamEncoder);


#ifdef __cplusplus
}
#endif


#endif



