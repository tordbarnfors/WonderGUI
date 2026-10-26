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
#ifndef WAX_C_SOFTGFX_DOT_H
#define WAX_C_SOFTGFX_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_surface.h>
#include <wax_c_edgemap.h>

#ifdef __cplusplus
extern "C" {
#endif

WAX_EXPORT wax_obj	wax_createSoftBackend(void);
WAX_EXPORT int		wax_defineSoftBackendCanvas( wax_obj backend, wax_canvasRef ref, wax_obj softSurface );


WAX_EXPORT wax_obj	wax_createSoftSurface(const wax_surfaceBP* blueprint);
WAX_EXPORT wax_obj	wax_createSoftSurfaceFromBlob(const wax_surfaceBP* blueprint, wax_obj blob, int pitch);
WAX_EXPORT wax_obj	wax_createSoftSurfaceFromBitmap(	wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
													wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette, int paletteSize );
WAX_EXPORT wax_obj	wax_createSoftSurfaceFromRawData(wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
													const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette, int paletteSize );

WAX_EXPORT wax_obj	wax_createSoftSurfaceFactory(void);


WAX_EXPORT wax_obj	wax_createSoftEdgemap( const wax_edgemapBP* blueprint );
WAX_EXPORT wax_obj	wax_createSoftEdgemapFromFloats( const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch);
WAX_EXPORT wax_obj	wax_createSoftEdgemapFromSpx( const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const wax_spx * pSamples, int edges, int edgePitch, int samplePitch);

WAX_EXPORT wax_obj	wax_createSoftEdgemapFactory(void);
WAX_EXPORT wax_obj	wax_createSoftWarpmapFactory(void);



#ifdef __cplusplus
}
#endif

#endif // WAX_C_SOFTGFX_DOT_H
