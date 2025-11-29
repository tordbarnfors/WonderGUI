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
#ifndef WG_C_LINEARGFX_DOT_H
#define WG_C_LINEARGFX_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct wg_linearGfxSegment_struct
	{
		wg_rectI	rect;			// In pixels, not spx!
		uint8_t *	pBuffer;
		int 		pitch;
	} wg_linearGfxSegment;

	WG_EXPORT wg_obj 	wg_createLinearBackend(void*(*beginCanvasRenderFunc)(wg_canvasRef ref, int nBytes),
							void(*endCanvasRenderFunc)(wg_canvasRef ref, int nSegments, const wg_linearGfxSegment * pSegments) );

	WG_EXPORT int		wg_defineLinearBackendCanvas( wg_obj backend, wg_canvasRef ref, wg_sizeSPX size, wg_pixelFormat format, int scale );

	WG_EXPORT void		wg_setLinearBackendSegmentPadding( wg_obj backend, int bytes );
	WG_EXPORT int		wg_linearBackendSegmentPadding( wg_obj backend);

#ifdef __cplusplus
}
#endif

#endif // WG_C_SOFTGFX_DOT_H
