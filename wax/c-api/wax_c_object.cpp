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
#include <wax_c_object.h>
#include <wax_object.h>

using namespace wax;

const wax_typeInfo* wax_getTypeInfo(wax_obj object)
{
	const TypeInfo& ret = reinterpret_cast<const Object*>(object)->typeInfo();
	return reinterpret_cast<const wax_typeInfo*>(&ret);
}

int wax_isInstanceOf(wax_obj object, const wax_typeInfo* pTypeInfo)
{
	return reinterpret_cast<Object*>(object)->isInstanceOf(reinterpret_cast<const TypeInfo&>(*pTypeInfo));
}

void wax_setFinalizer(wax_obj object, wax_finalizer_func pFinalizer)
{
	reinterpret_cast<Object*>(object)->setFinalizer(reinterpret_cast<void(*)(Object*)>(pFinalizer));
}

wax_finalizer_func wax_finalizer(wax_obj object)
{
	auto pFinalizer = reinterpret_cast<Object*>(object)->finalizer();
	return reinterpret_cast<wax_finalizer_func>(pFinalizer.rawPtr());
}

void wax_retain(wax_obj object)
{
	reinterpret_cast<Object*>(object)->retain();
}

void wax_release(wax_obj object)
{
	reinterpret_cast<Object*>(object)->release();
}

int wax_refcount(wax_obj object)
{
	return reinterpret_cast<Object*>(object)->refcount();
}
