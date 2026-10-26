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
#ifndef	WAX_POINTERS_DOT_H
#define	WAX_POINTERS_DOT_H
#pragma once

#include <wax_object.h>
#include <wax_strongptr.h>
#include <wax_weakptr.h>
#include <wax_componentptr.h>

template <class Type_p>
Type_p wax_dynamic_cast(wax::Object * pObject) {

	return dynamic_cast<typename Type_p::raw_type>(pObject);
}

template <class Type_p>
Type_p wax_static_cast(wax::Object * pObject) {

	return static_cast<typename Type_p::raw_type>(pObject);
}


#endif //WAX_POINTERS_DOT_H

