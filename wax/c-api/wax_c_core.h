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
#ifndef WAX_C_CORE_DOT_H
#define WAX_C_CORE_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif


WAX_EXPORT int		wax_initCore(void);
WAX_EXPORT int		wax_exitCore(void);
WAX_EXPORT int		wax_isCoreInitialized(void);

WAX_EXPORT void		wax_setErrorHandler(void (*errorHandler)(const wax_errorInfo * pError) );

WAX_EXPORT wax_obj	wax_defaultBitmapCache();

WAX_EXPORT void	wax_setDefaultSurfaceFactory( wax_obj factory );
WAX_EXPORT wax_obj	wax_defaultSurfaceFactory();

WAX_EXPORT void	wax_setDefaultEdgemapFactory( wax_obj factory );
WAX_EXPORT wax_obj	wax_defaultEdgemapFactory();

WAX_EXPORT void	wax_setDefaultWarpmapFactory( wax_obj factory );
WAX_EXPORT wax_obj	wax_defaultWarpmapFactory();

WAX_EXPORT void	wax_setDefaultGfxDevice( wax_obj device );
WAX_EXPORT wax_obj	wax_defaultGfxDevice();



#ifdef __cplusplus
}
#endif

#endif //WAX_C_CORE_DOT_H
