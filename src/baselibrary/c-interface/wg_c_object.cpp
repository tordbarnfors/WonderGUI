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

#include <wg_c_object.h>
#include <wg_object.h>

using namespace wg;

const wg_typeInfo* wg_getTypeInfo(wg_obj object)
{
	const TypeInfo& ret = reinterpret_cast<const Object*>(object)->typeInfo();
	return reinterpret_cast<const wg_typeInfo*>(&ret);
}

int wg_isInstanceOf(wg_obj object, const wg_typeInfo* pTypeInfo)
{
	return reinterpret_cast<Object*>(object)->isInstanceOf(reinterpret_cast<const TypeInfo&>(*pTypeInfo));
}

void wg_setFinalizer(wg_obj object, wg_finalizer_func pFinalizer)
{
	reinterpret_cast<Object*>(object)->setFinalizer(reinterpret_cast<Finalizer_p>(pFinalizer));
}

wg_finalizer_func wg_finalizer(wg_obj object)
{
	auto pFinalizer = reinterpret_cast<Object*>(object)->finalizer();
	return reinterpret_cast<wg_finalizer_func>(pFinalizer);
}

void wg_retain(wg_obj object)
{
	reinterpret_cast<Object*>(object)->retain();
}

void wg_release(wg_obj object)
{
	reinterpret_cast<Object*>(object)->release();
}

int wg_refcount(wg_obj object)
{
	return reinterpret_cast<Object*>(object)->refcount();
}
