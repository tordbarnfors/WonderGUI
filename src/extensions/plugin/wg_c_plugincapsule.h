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
#ifndef	WG_C_PLUGINCAPSULE_DOT_H
#define	WG_C_PLUGINCAPSULE_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wax_c_geo.h>
#include <wg_plugininterface.h>

#ifdef __cplusplus
extern "C" {
#endif

	void		wg_pluginRequestRender(wax_obj pluginCapsule, wax_rectSPX rect);
	void		wg_pluginRequestResize(wax_obj	pluginCapsule);
	int			wg_isPluginVisible(wax_obj pluginCapsule);
	wax_rectSPX 	wg_pluginWindowSection(wax_obj pluginCapsule);
	int 		wg_pluginRequestFocus(wax_obj pluginCapsule, int bRaiseWindow);
	int 		wg_pluginReleaseFocus(wax_obj pluginCapsule);
	int 		wg_pluginRequestPreRenderCall(wax_obj pluginCapsule);
	void 		wg_pluginRequestInView(wax_obj pluginCapsule, wax_rectSPX mustHaveArea, wax_rectSPX niceToHaveArea);
	void 		wg_connectPlugin(wax_obj pluginCapsule, void * pPluginRoot, wg_pluginroot_calls* pInterface);
	void 		wg_disconnectPlugin(wax_obj pluginCapsule);


#ifdef __cplusplus
}
#endif

#endif //WG_C_PLUGINCAPSULE_DOT_H
