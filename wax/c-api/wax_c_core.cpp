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
#include <string>

#include <wax_c_core.h>
#include <wax_core.h>
#include <wax_bitmapcache.h>

using namespace wax;

int wax_initCore()
{
	return Core::init();
}

int wax_exitCore()
{
	return Core::exit();
}

int wax_isCoreInitialized()
{
	return Core::isInitialized();
}

void wax_setErrorHandler( void (*errorHandler)(const wax_errorInfo * pError) )
{

	Core::setErrorHandler( [errorHandler](Error& error)
	{
		wax_errorInfo	errorInfo;

		errorInfo.classname = error.classname;
		errorInfo.code = (wax_errorCode) error.code;
		errorInfo.file = error.file;
		errorInfo.function = error.function;
		errorInfo.object = error.pObject;
		errorInfo.line = error.line;
		errorInfo.message = error.message;
		errorInfo.severity = (wax_errorLevel) error.severity;

		errorHandler(&errorInfo);
	});
}

wax_obj wax_defaultBitmapCache()
{
	BitmapCache* pCache = Core::defaultBitmapCache();
	return (wax_obj) static_cast<Object*>(pCache);
}

void wax_setDefaultSurfaceFactory( wax_obj factory )
{
	SurfaceFactory* pFactory = static_cast<SurfaceFactory*>(reinterpret_cast<Object*>(factory));
	return Core::setDefaultSurfaceFactory(pFactory);
}

wax_obj wax_defaultSurfaceFactory()
{
	SurfaceFactory * pFactory = Core::defaultSurfaceFactory();
	return (wax_obj) static_cast<Object*>(pFactory);
}

void wax_setDefaultEdgemapFactory( wax_obj factory )
{
	EdgemapFactory* pFactory = static_cast<EdgemapFactory*>(reinterpret_cast<Object*>(factory));
	return Core::setDefaultEdgemapFactory(pFactory);
}

wax_obj wax_defaultEdgemapFactory()
{
	EdgemapFactory * pFactory = Core::defaultEdgemapFactory();
	return (wax_obj) static_cast<Object*>(pFactory);
}

void wax_setDefaultWarpmapFactory( wax_obj factory )
{
	WarpmapFactory* pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(factory));
	return Core::setDefaultWarpmapFactory(pFactory);
}

wax_obj wax_defaultWarpmapFactory()
{
	WarpmapFactory * pFactory = Core::defaultWarpmapFactory();
	return (wax_obj) static_cast<Object*>(pFactory);
}

void wax_setDefaultDevice( wax_obj device )
{
	Device* pDevice = static_cast<Device*>(reinterpret_cast<Object*>(device));
	return Core::setDefaultDevice(pDevice);
}

wax_obj wax_defaultDevice()
{
	Device * pDevice = Core::defaultDevice();
	return (wax_obj) static_cast<Object*>(pDevice);
}

