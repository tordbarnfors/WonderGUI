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

#ifndef WG_C_STREAMPLAYER_DOT_H
#define WG_C_STREAMPLAYER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_obj			wg_createStreamPlayer(wg_obj backend, wg_obj surfaceFactory, wg_obj edgemapFactory);

	wg_component	wg_getStreamPlayerInput(wg_obj streamPlayer);

	void			wg_resetStreamPlayer(wg_obj streamPlayer);

	void			wg_setStreamPlayerStoreDirtyRects(wg_obj streamPlayer, int bStore);
	void			wg_setStreamPlayerMaxDirtyRects(wg_obj streamPlayer, int max);

	int				wg_getStreamPlayerDirtyRects(wg_obj streamPlayer, wg_canvasRef canvas, const wg_rectI **pRects );

	void			wg_clearStreamPlayerDirtyRects(wg_obj streamPlayer);

#ifdef __cplusplus
}
#endif


#endif
