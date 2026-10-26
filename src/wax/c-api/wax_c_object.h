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
#ifndef WAX_C_OBJECT_DOT_H
#define WAX_C_OBJECT_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif


	WAX_EXPORT const wax_typeInfo*	wax_getTypeInfo(wax_obj);
	WAX_EXPORT int				wax_isInstanceOf(wax_obj, const wax_typeInfo* pTypeInfo);

	WAX_EXPORT void				wax_setFinalizer(wax_obj, wax_finalizer_func pFinalizer);
	WAX_EXPORT wax_finalizer_func	wax_finalizer(wax_obj);

	WAX_EXPORT void				wax_retain(wax_obj);
	WAX_EXPORT void				wax_release(wax_obj);

	WAX_EXPORT int				wax_refcount(wax_obj);


#ifdef __cplusplus
}
#endif

#endif


