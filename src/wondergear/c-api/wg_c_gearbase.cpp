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

#include <string>

#include <wg_c_gearbase.h>
#include <wg_gearbase.h>

using namespace wg;

int wg_initGearBase()
{
	return GearBase::init();
}

int wg_exitGearBase()
{
	return GearBase::exit();
}

int wg_isGearBaseInitialized()
{
	return GearBase::isInitialized();
}

void wg_setErrorHandler( void (*errorHandler)(const wg_errorInfo * pError) )
{

	GearBase::setErrorHandler( [errorHandler](Error& error)
	{
		wg_errorInfo	errorInfo;

		errorInfo.classname = error.classname;
		errorInfo.code = (wg_errorCode) error.code;
		errorInfo.file = error.file;
		errorInfo.function = error.function;
		errorInfo.object = error.pObject;
		errorInfo.line = error.line;
		errorInfo.message = error.message;
		errorInfo.severity = (wg_errorLevel) error.severity;

		errorHandler(&errorInfo);
	});
}
