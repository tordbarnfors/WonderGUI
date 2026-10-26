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
#ifndef WAX_C_STREAMPLAYER_DOT_H
#define WAX_C_STREAMPLAYER_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef	void(*wax_streamPlayerFence_func)(uint16_t fenceID, uint32_t fenceValue);


	WAX_EXPORT wax_obj		wax_createStreamPlayer(wax_obj backend, wax_obj surfaceFactory, wax_obj edgemapFactory);

	WAX_EXPORT wax_component	wax_getStreamPlayerInput(wax_obj streamPlayer);

	WAX_EXPORT void			wax_resetStreamPlayer(wax_obj streamPlayer);

	WAX_EXPORT void			wax_setStreamPlayerStoreDirtyRects(wax_obj streamPlayer, int bStore);
	WAX_EXPORT void			wax_setStreamPlayerMaxDirtyRects(wax_obj streamPlayer, int max);

	WAX_EXPORT int			wax_getStreamPlayerDirtyRects(wax_obj streamPlayer, wax_canvasRef canvas, const wax_rectI **pRects );

	WAX_EXPORT void			wax_clearStreamPlayerDirtyRects(wax_obj streamPlayer);

/*WAX_EXPORT void			wax_setStreamPlayerCanvasInfoCallback( const std::function<void(const CanvasInfo * pBegin, const CanvasInfo * pEnd)>& callback ); */
	WAX_EXPORT void			wax_setStreamPlayerFenceCallback(wax_obj streamPlayer, wax_streamPlayerFence_func callback );



#ifdef __cplusplus
}
#endif


#endif
