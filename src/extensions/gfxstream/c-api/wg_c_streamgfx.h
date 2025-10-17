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

	typedef struct wg_mirrorSurfaceBP_struct	// NOT BINARY EQUIVALENT!
	{
		wg_canvasRef	canvasRef;				// Stream to specified reference, otherwise create surface and stream to that.
		wg_obj			encoder;				// Required
		wg_obj			surface;				// Required
		int				streamOnCreate;			// If true, surface content will be streamed immediately after creation.
	} wg_mirrorSurfaceBP;



	WG_EXPORT wg_obj	wg_createStreamBackend(wg_obj streamEncoder, int maxEdges );
	WG_EXPORT int		wg_defineStreamBackendCanvasWithSurface( wg_obj streamBackend, wg_canvasRef ref, wg_obj surface );
	WG_EXPORT int		wg_defineStreamBackendCanvas( wg_obj streamBackend, wg_canvasRef ref, wg_sizeI pixelSize, wg_pixelFormat pixelFormat, int scale );

    WG_EXPORT void      wg_encodeCanvasList(wg_obj streamBackend);
    WG_EXPORT void      wg_encodeTick(wg_obj streamBackend, int32_t microsecPassed);        // Microsec since last tick (or beginning of stream if first

	WG_EXPORT wg_obj	wg_createRemoteSurface(wg_obj streamEncoder, wg_surfaceBP* blueprint);
	WG_EXPORT wg_obj	wg_createRemoteSurfaceFromBlob(wg_obj streamEncoder, const wg_surfaceBP* blueprint, wg_obj blob, int pitch);
	WG_EXPORT wg_obj	wg_createRemoteSurfaceFromBitmap(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
												   wg_pixelFormat pixelFormat, int pitch, const wg_color8 * pPalette );
	WG_EXPORT wg_obj	wg_createRemoteSurfaceFromRawData(wg_obj streamEncoder, wg_obj factory, const wg_surfaceBP* blueprint, const uint8_t* pPixels,
													const wg_pixelDescription * pPixelDescription, int pitch, const wg_color8 * pPalette );



	WG_EXPORT wg_obj	wg_createRemoteSurfaceFactory(wg_obj streamEncoder);

	WG_EXPORT void      wg_setRememberRemoteSurfacesCreated(wg_obj remoteSurfaceFactory, int bRemember);
	WG_EXPORT void      wg_recreateRemoteSurfaces(wg_obj remoteSurfaceFactory);

	WG_EXPORT wg_obj	wg_createMirrorSurface(wg_obj streamEncoder, wg_mirrorSurfaceBP* blueprint);

	WG_EXPORT wg_obj	wg_createMirrorSurfaceFactory(wg_obj streamEncoder, wg_obj backingSurfaceFactory );

	WG_EXPORT void      wg_setRememberMirrorSurfacesCreated(wg_obj mirrorSurfaceFactory, int bRemember);
	WG_EXPORT void      wg_streamMirrorSurfacesAsNew(wg_obj remoteSurfaceFactory);


	WG_EXPORT wg_obj	wg_createStreamEdgemap( wg_obj streamEncoder, const wg_edgemapBP* blueprint );
	WG_EXPORT wg_obj	wg_createStreamEdgemapFromFloats( wg_obj streamEncoder, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch);
	WG_EXPORT wg_obj	wg_createStreamEdgemapFromSpx( wg_obj streamEncoder, const wg_edgemapBP* blueprint, wg_sampleOrigo origo, const wg_spx * pSamples, int edges, int edgePitch, int samplePitch);

	WG_EXPORT wg_obj	wg_createStreamEdgemapFactory(wg_obj streamEncoder);


#ifdef __cplusplus
}
#endif


#endif



