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
#ifndef	WG_C_PLUGINROOT_DOT_H
#define	WG_C_PLUGINROOT_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_spx 		wg_matchingPluginHeight(void* pPluginRoot, wg_spx width, int scale);
	wg_spx 		wg_matchingPluginWidth(void* pPluginRoot, wg_spx height, int scale);
	wg_sizeSPX 	wg_defaultPluginSize(void* pPluginRoot, int scale);
	wg_sizeSPX 	wg_minPluginSize(void* pPluginRoot, int scale);
	wg_sizeSPX 	wg_maxPluginSize(void* pPluginRoot, int scale);
	int			wg_markTestPlugin(void* pPluginRoot, wg_coordSPX ofs);
	void 		wg_preRenderPlugin(void* pPluginRoot);
	void 		wg_renderPlugin(void* pPluginRoot, wg_obj device, wg_rectSPX canvas, wg_rectSPX window);
	void 		wg_resizePlugin(void* pPluginRoot, wg_sizeSPX size, int scale);
	void 		wg_setPluginState(void* pPluginRoot, wg_state state);
	void 		wg_pluginReceive(void* pPluginRoot, wg_obj msg);
	void 		wg_setPluginPointerPos(void* pPluginRoot, wg_coordSPX pos, int64_t timestamp );
	void 		wg_setPluginButtonState(void* pPluginRoot, int button, int pressed, int64_t timestamp );
	void 		wg_setPluginKeyState(void* pPluginRoot, int nativeKeyCode, int pressed, int64_t timestamp );
	void 		wg_putPluginText(void* pPluginRoot, const char * pUTF8String );
	void 		wg_pluginWheelRoll(void* pPluginRoot, int wheel, wg_pts distance, int bInvert, int64_t timestamp );

	void 		wg_onPluginUpdate(void* pPluginRoot, int microPassed, int64_t microsecTimestamp);
	wg_pointerStyle wg_pluginPointerStyle(void* pPluginRoot);

#ifdef __cplusplus
}
#endif

#endif //WG_C_PLUGINROOT_DOT_H
