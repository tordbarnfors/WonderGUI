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
#ifndef WAX_C_BALLWARP_DOT_H
#define WAX_C_BALLWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_ballWarpBP _________________________________________________________

	typedef struct wax_ballWarpBP_struct			// NOT BINARY EQUIVALENT!
	{
		wax_sizeI			cellSize;				// Length of pieces along the equator and a meridian. 0 = calculated from precision.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		float				roll;					// Degrees the axis of the ball is turned clockwise on screen.
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels, which the ball is centered in.
		wax_sizeSPX			sourceSize;				// Mandatory. Size of the map (edgemap or source rectangle) in spx.
		float				tilt;					// Degrees the north pole leans towards the viewer, -90 to 90.
	} wax_ballWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, cell size, precision and generate are set and called through the WarpGenerator functions.

	WAX_EXPORT wax_obj		wax_createBallWarp( const wax_ballWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setBallWarpOrientation( wax_obj ballWarp, float tilt, float roll );
	WAX_EXPORT float		wax_ballWarpTilt( wax_obj ballWarp );
	WAX_EXPORT float		wax_ballWarpRoll( wax_obj ballWarp );

	WAX_EXPORT wax_spx		wax_ballWarpRadius( wax_obj ballWarp );

	//.____ Drawing _______________________________________________________

	// Source offset that turns the ball by degrees around its axis, the way the Earth turns (from
	// west to east) if positive.

	WAX_EXPORT wax_coordSPX	wax_ballWarpRotationOffset( wax_obj ballWarp, float degrees );

#ifdef __cplusplus
}
#endif

#endif
