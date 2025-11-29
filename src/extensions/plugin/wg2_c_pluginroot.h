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
#ifndef	WG2_C_PLUGINROOT_DOT_H
#define	WG2_C_PLUGINROOT_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	wg_spx 		wg2_matchingPluginHeight(void* pPluginRoot, wg_spx width, int scale);
	wg_spx 		wg2_matchingPluginWidth(void* pPluginRoot, wg_spx height, int scale);
	wg_sizeSPX 	wg2_defaultPluginSize(void* pPluginRoot, int scale);
	wg_sizeSPX 	wg2_minPluginSize(void* pPluginRoot, int scale);
	wg_sizeSPX 	wg2_maxPluginSize(void* pPluginRoot, int scale);
	int			wg2_markTestPlugin(void* pPluginRoot, wg_coordSPX ofs);
	void 		wg2_preRenderPlugin(void* pPluginRoot);
	void 		wg2_renderPlugin(void* pPluginRoot, wg_obj device, wg_rectSPX canvas, wg_rectSPX window);
	void 		wg2_resizePlugin(void* pPluginRoot, wg_sizeSPX size, int scale);
	void 		wg2_setPluginState(void* pPluginRoot, wg_state state);
	void 		wg2_pluginReceive(void* pPluginRoot, wg_obj msg);
	void 		wg2_setPluginPointerPos(void* pPluginRoot, wg_coordSPX pos, int64_t timestamp );
	void 		wg2_setPluginButtonState(void* pPluginRoot, int button, int pressed, int64_t timestamp );
	void 		wg2_setPluginKeyState(void* pPluginRoot, int nativeKeyCode, int pressed, int64_t timestamp );
	void 		wg2_putPluginText(void* pPluginRoot, const char * pUTF8String );
	void		wg2_pluginWheelRoll(void* pPluginRoot, int wheel, wg_pts distance, int bInvert, int64_t timestamp );
	void 		wg2_onPluginUpdate(void* pPluginRoot, int microPassed, int64_t microsecTimestamp);
	wg_pointerStyle wg2_pluginPointerStyle(void* pPluginRoot);

#ifdef __cplusplus
}
#endif

#endif //WG2_C_PLUGINROOT_DOT_H
