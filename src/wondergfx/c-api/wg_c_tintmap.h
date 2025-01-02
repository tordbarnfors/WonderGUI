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

#ifndef WG_C_TINTMAP_DOT_H
#define WG_C_TINTMAP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>
#include <wg_c_color.h>

#ifdef __cplusplus
extern "C" {
#endif

int		wg_isTintmapOpaque( wg_obj tintmap );
int		wg_isTintmapVertical( wg_obj tintmap );
int		wg_isTintmapHorizontal( wg_obj tintmap );

void 	wg_exportHorizontalColors( wg_obj tintmap, wg_spx length, wg_color* pOutput);
void 	wg_exportVerticalColors( wg_obj tintmap, wg_spx length, wg_color* pOutput);

#ifdef __cplusplus
}
#endif

#endif

