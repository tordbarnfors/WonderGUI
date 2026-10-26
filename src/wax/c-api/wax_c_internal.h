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
#ifndef WAX_C_INTERNAL_DOT_H
#define WAX_C_INTERNAL_DOT_H
#pragma once

/* Functions used internally by the C-API */


#ifdef __cplusplus

#include <wax_surface.h>
#include <wax_c_surface.h>

#include <wax_edgemap.h>
#include <wax_c_edgemap.h>

#include <wax_tint.h>

#include <wax_warpmap.h>
#include <wax_c_warpmap.h>


WAX_NO_EXPORT void convertSurfaceBlueprint(wax::Surface::Blueprint* pDest, const wax_surfaceBP* pSource);
WAX_NO_EXPORT void convertEdgemapBlueprint(wax::Edgemap::Blueprint* pDest, const wax_edgemapBP* pSource, wax::Tint_p * pSpaceForTints );	// pSpaceForTints needs room for bp.segments pointers.
WAX_NO_EXPORT void convertWarpmapBlueprint(wax::Warpmap::Blueprint* pDest, const wax_warpmapBP* pSource);


#endif	//__cplusplus

#endif	//WAX_C_INTERNAL_DOT_H
