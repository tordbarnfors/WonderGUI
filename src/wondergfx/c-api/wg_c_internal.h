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
#ifndef WG_C_INTERNAL_DOT_H
#define WG_C_INTERNAL_DOT_H
#pragma once

/* Functions used internally by the C-API */


#ifdef __cplusplus

#include <wg_surface.h>
#include <wg_c_surface.h>

#include <wg_edgemap.h>
#include <wg_c_edgemap.h>

#include <wg_tintmap.h>


WG_NO_EXPORT void convertSurfaceBlueprint(wg::Surface::Blueprint* pDest, const wg_surfaceBP* pSource);
WG_NO_EXPORT void convertEdgemapBlueprint(wg::Edgemap::Blueprint* pDest, const wg_edgemapBP* pSource, wg::Tintmap_p * pTintmapArea );


#endif	//__cplusplus

#endif	//WG_C_INTERNAL_DOT_H
