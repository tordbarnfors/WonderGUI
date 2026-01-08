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
#ifndef WG_C_GRADYENT_DOT_H
#define WG_C_GRADYENT_DOT_H
#pragma once

#include <wg_c_geartypes.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif

WG_EXPORT wg_obj	wg_createGradyent(wg_color top, wg_color bottom, wg_color left, wg_color right);

#ifdef __cplusplus
}
#endif

#endif
