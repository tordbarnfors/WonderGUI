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
#include <wg_c_plugincapsule.h>
#include <wg_plugincapsule.h>

using namespace wg;

inline PluginCapsule* getCapsule(wg_obj obj) {
	return static_cast<PluginCapsule*>(reinterpret_cast<Object*>(obj));
}

void wg_pluginRequestRender(wg_obj pluginCapsule, wg_rectSPX rect)
{
	getCapsule(pluginCapsule)->_pluginRequestRender({ rect.x, rect.y, rect.w, rect.h });
}

void wg_pluginRequestResize(wg_obj	pluginCapsule)
{
	getCapsule(pluginCapsule)->_pluginRequestResize();
}

int wg_isPluginVisible(wg_obj pluginCapsule)
{
	return getCapsule(pluginCapsule)->_isPluginVisible();
}

wg_rectSPX wg_pluginWindowSection(wg_obj pluginCapsule)
{
	auto rect = getCapsule(pluginCapsule)->_pluginWindowSection();

	return { rect.x, rect.y, rect.w, rect.h };
}

int wg_pluginRequestFocus(wg_obj pluginCapsule, int bRaiseWindow)
{
	return getCapsule(pluginCapsule)->_pluginRequestFocus(bool(bRaiseWindow));
}

int wg_pluginReleaseFocus(wg_obj pluginCapsule)
{
	return getCapsule(pluginCapsule)->_pluginReleaseFocus();
}

int wg_pluginRequestPreRenderCall(wg_obj pluginCapsule)
{
	return 	getCapsule(pluginCapsule)->_requestPreRenderCall();
}

void wg_pluginRequestInView(wg_obj pluginCapsule, wg_rectSPX mustHaveArea, wg_rectSPX niceToHaveArea)
{
	return getCapsule(pluginCapsule)->_pluginRequestInView( {mustHaveArea.x, mustHaveArea.y, mustHaveArea.w, mustHaveArea.h},
														{niceToHaveArea.x, niceToHaveArea.y, niceToHaveArea.w, niceToHaveArea.h} );
}

void wg_connectPlugin(wg_obj pluginCapsule, void * pPluginRoot, wg_pluginroot_calls* pInterface)
{
	return getCapsule(pluginCapsule)->_connectPlugin(pPluginRoot,pInterface);
}

void wg_disconnectPlugin(wg_obj pluginCapsule)
{
	return getCapsule(pluginCapsule)->_disconnectPlugin();
}


