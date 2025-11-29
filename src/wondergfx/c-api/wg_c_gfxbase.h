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
#ifndef WG_C_GFXBASE_DOT_H
#define WG_C_GFXBASE_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_gearbase.h>

#ifdef __cplusplus
extern "C" {
#endif


WG_EXPORT int		wg_initGfxBase(void);
WG_EXPORT int		wg_exitGfxBase(void);
WG_EXPORT int		wg_isGfxBaseInitialized(void);

WG_EXPORT wg_obj	wg_defaultBitmapCache();

WG_EXPORT void	wg_setDefaultSurfaceFactory( wg_obj factory );
WG_EXPORT wg_obj	wg_defaultSurfaceFactory();

WG_EXPORT void	wg_setDefaultEdgemapFactory( wg_obj factory );
WG_EXPORT wg_obj	wg_defaultEdgemapFactory();

WG_EXPORT void	wg_setDefaultGfxDevice( wg_obj device );
WG_EXPORT wg_obj	wg_defaultGfxDevice();

WG_EXPORT void	wg_setDefaultToSRGB( int bSRGB );
WG_EXPORT int		wg_defaultToSRGB();


#ifdef __cplusplus
}
#endif

#endif //WG_C_GFXBASE_DOT_H
