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
#ifndef WAX_C_STREAMTRIMBACKEND_DOT_H
#define WAX_C_STREAMTRIMBACKEND_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

WAX_EXPORT wax_obj	wax_createStreamTrimBackend(wax_obj backend);

WAX_EXPORT void		wax_addNonMaskingSession(wax_obj streamTrimBackend);
WAX_EXPORT void		wax_addFullyMaskingSession(wax_obj streamTrimBackend,  wax_canvasRef canvasRef, wax_obj canvasSurface );
WAX_EXPORT void		wax_addMaskingSession(wax_obj streamTrimBackend,  wax_canvasRef canvasRef, wax_obj canvasSurface, int nMaskingRects, const wax_rectSPX * pMaskingRects );
WAX_EXPORT void		wax_clearSessionMasks(wax_obj streamTrimBackend);
WAX_EXPORT void		wax_setTrimLevel(wax_obj streamTrimBackend, int level);

#ifdef __cplusplus
}
#endif


#endif




