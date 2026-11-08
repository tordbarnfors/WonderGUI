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
#ifndef WAX_C_LENSWARP_DOT_H
#define WAX_C_LENSWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_lensWarpBP _________________________________________________________

	typedef struct wax_lensWarpBP_struct			// NOT BINARY EQUIVALENT!
	{
		float				falloff;				// Part of the radius, 0 to 1, over which magnification falls off to none at the rim.
		float				magnification;			// In the middle of the lens, 1 or more. 0 = default (2).
		float				precision;				// Max position error in pixels. 0 = default (0.25).
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels, which the lens is centered in.
		wax_sizeSPX			sourceSize;				// Mandatory. Size of the source under the lens in spx, normally size * 64.
	} wax_lensWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, precision and generate are set and called through the WarpGenerator functions.

	WAX_EXPORT wax_obj		wax_createLensWarp( const wax_lensWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setLensWarpMagnification( wax_obj lensWarp, float magnification );
	WAX_EXPORT float		wax_lensWarpMagnification( wax_obj lensWarp );

	WAX_EXPORT int			wax_setLensWarpFalloff( wax_obj lensWarp, float falloff );
	WAX_EXPORT float		wax_lensWarpFalloff( wax_obj lensWarp );

	WAX_EXPORT wax_spx		wax_lensWarpRadius( wax_obj lensWarp );

#ifdef __cplusplus
}
#endif

#endif
