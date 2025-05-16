
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

#include <wg_c_gfxbase.h>
#include <wg_gfxbase.h>
#include <wg_bitmapcache.h>

using namespace wg;

int wg_initGfxBase()
{
	return GfxBase::init();
}

int wg_exitGfxBase()
{
	return GfxBase::exit();
}

int wg_isGfxBaseInitialized()
{
	return GfxBase::isInitialized();
}

wg_obj wg_defaultBitmapCache()
{
	BitmapCache* pCache = GfxBase::defaultBitmapCache();
	return (wg_obj) static_cast<Object*>(pCache);
}

void wg_setDefaultSurfaceFactory( wg_obj factory )
{
	SurfaceFactory* pFactory = static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(factory));
	return GfxBase::setDefaultSurfaceFactory(pFactory);
}

wg_obj wg_defaultSurfaceFactory()
{
	SurfaceFactory * pFactory = GfxBase::defaultSurfaceFactory();
	return (wg_obj) static_cast<Object*>(pFactory);
}

void wg_setDefaultEdgemapFactory( wg_obj factory )
{
	EdgemapFactory* pFactory = static_cast<EdgemapFactory*>(reinterpret_cast<Object*>(factory));
	return GfxBase::setDefaultEdgemapFactory(pFactory);
}

wg_obj wg_defaultEdgemapFactory()
{
	EdgemapFactory * pFactory = GfxBase::defaultEdgemapFactory();
	return (wg_obj) static_cast<Object*>(pFactory);
}

void wg_setDefaultGfxDevice( wg_obj device )
{
	GfxDevice* pDevice = static_cast<GfxDevice*>(reinterpret_cast<Object*>(device));
	return GfxBase::setDefaultGfxDevice(pDevice);
}

wg_obj wg_defaultGfxDevice()
{
	GfxDevice * pDevice = GfxBase::defaultGfxDevice();
	return (wg_obj) static_cast<Object*>(pDevice);
}

void wg_setDefaultToSRGB( int bSRGB )
{
	GfxBase::setDefaultToSRGB(bSRGB);
}

int wg_defaultToSRGB()
{
	return GfxBase::defaultToSRGB();
}
