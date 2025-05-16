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


	void convertSurfaceBlueprint(wg::Surface::Blueprint* pDest, const wg_surfaceBP* pSource);
	void convertEdgemapBlueprint(wg::Edgemap::Blueprint* pDest, const wg_edgemapBP* pSource, wg::Tintmap_p * pTintmapArea );


#endif	//__cplusplus

#endif	//WG_C_INTERNAL_DOT_H
