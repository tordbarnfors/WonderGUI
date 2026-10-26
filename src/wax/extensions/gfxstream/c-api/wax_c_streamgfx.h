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
#ifndef WAX_C_STREAMGFX_DOT_H
#define WAX_C_STREAMGFX_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>
#include <wax_c_gfxstream.h>
#include <wax_c_surface.h>
#include <wax_c_edgemap.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct wax_mirrorSurfaceBP_struct	// NOT BINARY EQUIVALENT!
	{
		wax_canvasRef	canvasRef;				// Stream to specified reference, otherwise create surface and stream to that.
		wax_obj			encoder;				// Required
		wax_obj			surface;				// Required
		int				streamOnCreate;			// If true, surface content will be streamed immediately after creation.
	} wax_mirrorSurfaceBP;



	WAX_EXPORT wax_obj	wax_createStreamBackend(wax_obj streamEncoder, int maxEdges );
	WAX_EXPORT int		wax_defineStreamBackendCanvasWithSurface( wax_obj streamBackend, wax_canvasRef ref, wax_obj surface );
	WAX_EXPORT int		wax_defineStreamBackendCanvas( wax_obj streamBackend, wax_canvasRef ref, wax_sizeI pixelSize, wax_pixelFormat pixelFormat, int scale,
													  wax_colorSpace colorSpace, wax_byteOrder byteOrder );

    WAX_EXPORT void      wax_encodeCanvasList(wax_obj streamBackend);
    WAX_EXPORT void      wax_encodeTick(wax_obj streamBackend, int32_t microsecPassed);        // Microsec since last tick (or beginning of stream if first

	WAX_EXPORT wax_obj	wax_createRemoteSurface(wax_obj streamEncoder, wax_surfaceBP* blueprint);
	WAX_EXPORT wax_obj	wax_createRemoteSurfaceFromBlob(wax_obj streamEncoder, const wax_surfaceBP* blueprint, wax_obj blob, int pitch);
	WAX_EXPORT wax_obj	wax_createRemoteSurfaceFromBitmap(wax_obj streamEncoder, wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
												   wax_pixelFormat pixelFormat, int pitch, const wax_color8 * pPalette );
	WAX_EXPORT wax_obj	wax_createRemoteSurfaceFromRawData(wax_obj streamEncoder, wax_obj factory, const wax_surfaceBP* blueprint, const uint8_t* pPixels,
													const wax_pixelDescription * pPixelDescription, int pitch, const wax_color8 * pPalette );



	WAX_EXPORT wax_obj	wax_createRemoteSurfaceFactory(wax_obj streamEncoder);

	WAX_EXPORT void      wax_setRememberRemoteSurfacesCreated(wax_obj remoteSurfaceFactory, int bRemember);
	WAX_EXPORT void      wax_recreateRemoteSurfaces(wax_obj remoteSurfaceFactory);

	WAX_EXPORT wax_obj	wax_createMirrorSurface(wax_obj streamEncoder, wax_mirrorSurfaceBP* blueprint);

	WAX_EXPORT wax_obj	wax_createMirrorSurfaceFactory(wax_obj streamEncoder, wax_obj backingSurfaceFactory );

	WAX_EXPORT void      wax_setRememberMirrorSurfacesCreated(wax_obj mirrorSurfaceFactory, int bRemember);
	WAX_EXPORT void      wax_streamMirrorSurfacesAsNew(wax_obj remoteSurfaceFactory);


	WAX_EXPORT wax_obj	wax_createStreamEdgemap( wax_obj streamEncoder, const wax_edgemapBP* blueprint );
	WAX_EXPORT wax_obj	wax_createStreamEdgemapFromFloats( wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch);
	WAX_EXPORT wax_obj	wax_createStreamEdgemapFromSpx( wax_obj streamEncoder, const wax_edgemapBP* blueprint, wax_sampleOrigo origo, const wax_spx * pSamples, int edges, int edgePitch, int samplePitch);

	WAX_EXPORT wax_obj	wax_createStreamEdgemapFactory(wax_obj streamEncoder);
	WAX_EXPORT wax_obj	wax_createStreamWarpmapFactory(wax_obj streamEncoder);


#ifdef __cplusplus
}
#endif


#endif



