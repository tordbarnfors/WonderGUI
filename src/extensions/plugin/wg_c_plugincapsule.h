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

#ifndef	WG_C_PLUGINCAPSULE_DOT_H
#define	WG_C_PLUGINCAPSULE_DOT_H
#pragma once

#include <wg_c_types.h>
#include <wg_c_geo.h>
#include <wg_plugininterface.h>

#ifdef __cplusplus
extern "C" {
#endif

	void		wg_pluginRequestRender(wg_obj pluginCapsule, wg_rectSPX rect);
	void		wg_pluginRequestResize(wg_obj	pluginCapsule);
	int			wg_isPluginVisible(wg_obj pluginCapsule);
	wg_rectSPX 	wg_pluginWindowSection(wg_obj pluginCapsule);
	int 		wg_pluginRequestFocus(wg_obj pluginCapsule);
	int 		wg_pluginReleaseFocus(wg_obj pluginCapsule);
	int 		wg_pluginRequestPreRenderCall(wg_obj pluginCapsule);
	void 		wg_pluginRequestInView(wg_obj pluginCapsule, wg_rectSPX mustHaveArea, wg_rectSPX niceToHaveArea);
	void 		wg_connectPlugin(wg_obj pluginCapsule, void * pPluginRoot, wg_pluginroot_calls* pInterface);
	void 		wg_disconnectPlugin(wg_obj pluginCapsule);


#ifdef __cplusplus
}
#endif

#endif //WG_C_PLUGINCAPSULE_DOT_H
