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
#ifndef WAX_C_LINEARGFX_DOT_H
#define WAX_C_LINEARGFX_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct wax_linearGfxSegment_struct
	{
		wax_rectI	rect;			// In pixels, not spx!
		uint8_t *	pBuffer;
		int 		pitch;
	} wax_linearGfxSegment;

	WAX_EXPORT wax_obj 	wax_createLinearBackend(void*(*beginCanvasRenderFunc)(wax_canvasRef ref, int nBytes),
							void(*endCanvasRenderFunc)(wax_canvasRef ref, int nSegments, const wax_linearGfxSegment * pSegments) );

	WAX_EXPORT int		wax_defineLinearBackendCanvas( wax_obj backend, wax_canvasRef ref, wax_sizeSPX size, wax_pixelFormat format, int scale,
													  wax_colorSpace colorSpace, wax_byteOrder byteOrder );

	WAX_EXPORT void		wax_setLinearBackendSegmentPadding( wax_obj backend, int bytes );
	WAX_EXPORT int		wax_linearBackendSegmentPadding( wax_obj backend);

#ifdef __cplusplus
}
#endif

#endif // WAX_C_SOFTGFX_DOT_H
