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
#include <wg_plugininterface.h>

#include <wg2_c_pluginroot.h>
#include <wg2_pluginroot.h>

#include <wg_geo.h>

using namespace wg;

inline WgPluginRoot* getRoot(void * pRoot) {
	return static_cast<WgPluginRoot*>(pRoot);
}



wg_spx wg2_matchingPluginHeight(void* pPluginRoot, wg_spx width, int scale)
{
	return (wg_spx) getRoot(pPluginRoot)->_matchingHeight(spx(width), scale);
}

wg_spx wg2_matchingPluginWidth(void* pPluginRoot, wg_spx height, int scale)
{
	return (wg_spx) getRoot(pPluginRoot)->_matchingWidth(spx(height), scale);
}

wg_sizeSPX wg2_defaultPluginSize(void* pPluginRoot, int scale)
{
	SizeSPX sz = getRoot(pPluginRoot)->_defaultSize(scale);
	return { sz.w, sz.h };
}

wg_sizeSPX wg2_minPluginSize(void* pPluginRoot, int scale)
{
	SizeSPX sz = getRoot(pPluginRoot)->_minSize(scale);
	return { sz.w, sz.h };
}

wg_sizeSPX wg2_maxPluginSize(void* pPluginRoot, int scale)
{
	SizeSPX sz = getRoot(pPluginRoot)->_maxSize(scale);
	return { sz.w, sz.h };
}

int	wg2_markTestPlugin(void* pPluginRoot, wg_coordSPX ofs)
{
	return getRoot(pPluginRoot)->_markTest({ ofs.x, ofs.y });
}

void wg2_preRenderPlugin(void* pPluginRoot)
{
	getRoot(pPluginRoot)->_preRender();
}

void wg2_renderPlugin(void* pPluginRoot, wg_obj device, wg_rectSPX canvas, wg_rectSPX window)
{
	getRoot(pPluginRoot)->_render(device, {canvas.x, canvas.y, canvas.w, canvas.h}, {window.x, window.y, window.w, window.h});
}

void wg2_resizePlugin(void* pPluginRoot, wg_sizeSPX size, int scale)
{
	getRoot(pPluginRoot)->_resize({ size.w, size.h }, scale);
}

void wg2_setPluginState(void* pPluginRoot, wg_state state)
{
	getRoot(pPluginRoot)->_setState(*(State*)&state);
}

void wg2_pluginReceive(void* pPluginRoot, wg_obj msg)
{

}

void wg2_setPluginPointerPos(void* pPluginRoot, wg_coordSPX pos, int64_t timestamp )
{
	getRoot(pPluginRoot)->_setPointerPos({pos.x,pos.y}, timestamp);
}

void wg2_setPluginButtonState(void* pPluginRoot, int button, int pressed, int64_t timestamp )
{
	getRoot(pPluginRoot)->_setButtonState(button, bool(pressed), timestamp);
}

void wg2_setPluginKeyState(void* pPluginRoot, int nativeKeyCode, int pressed, int64_t timestamp )
{
	getRoot(pPluginRoot)->_setKeyState(nativeKeyCode, bool(pressed), timestamp);
}

void wg2_putPluginText(void* pPluginRoot, const char * pUTF8String )
{
	getRoot(pPluginRoot)->_putText(pUTF8String);
}

void wg2_pluginWheelRoll(void* pPluginRoot, int wheel, wg_pts distance, int bInvert, int64_t timestamp )
{
	getRoot(pPluginRoot)->_wheelRoll(wheel, float(distance), bool(bInvert), timestamp);
}

void wg2_onPluginUpdate(void* pPluginRoot, int microPassed, int64_t microsecTimestamp)
{
	getRoot(pPluginRoot)->_update(microPassed,microsecTimestamp);
}

wg_pointerStyle wg2_pluginPointerStyle(void* pPluginRoot)
{
	return (wg_pointerStyle)getRoot(pPluginRoot)->_pointerStyle();
}

