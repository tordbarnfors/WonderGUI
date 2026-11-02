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
#ifndef WG_C_CYLINDERWARP_DOT_H
#define WG_C_CYLINDERWARP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_cylinderWarpBP _____________________________________________________

	typedef struct wg_cylinderWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wg_axis				axis;					// Axis of the cylinder. WG_AXIS_X lies down, like a slot machine reel. 0 = default (WG_AXIS_X).
		wg_sizeI			cellSize;				// 0 for width or height = calculated from precision.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		wg_sizeI			size;					// Mandatory. Size of the Warpmap in pixels, which the cylinder fills.
		wg_sizeSPX			sourceSize;				// Mandatory. Size of the source (edgemap or source rectangle) in spx.
		float				span;					// Degrees of the cylinder seen, above 0 and below 180. 0 = default (120).
		float				startAngle;				// Degrees. Where the start of the source across the axis is, 0 = facing the viewer.
		float				sweep;					// Degrees around the cylinder covered by the source. Negative goes the other way. 0 = default (360).
	} wg_cylinderWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, cell size, precision and generate are set and called through the WarpGenerator functions.

	WG_EXPORT wg_obj		wg_createCylinderWarp( const wg_cylinderWarpBP* blueprint, wg_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WG_EXPORT int			wg_setCylinderWarpAxis( wg_obj cylinderWarp, wg_axis axis );
	WG_EXPORT wg_axis		wg_cylinderWarpAxis( wg_obj cylinderWarp );

	WG_EXPORT int			wg_setCylinderWarpSpan( wg_obj cylinderWarp, float span );
	WG_EXPORT float			wg_cylinderWarpSpan( wg_obj cylinderWarp );

	WG_EXPORT int			wg_setCylinderWarpAngles( wg_obj cylinderWarp, float startAngle, float sweep );
	WG_EXPORT float			wg_cylinderWarpStartAngle( wg_obj cylinderWarp );
	WG_EXPORT float			wg_cylinderWarpSweep( wg_obj cylinderWarp );

	//.____ Drawing _______________________________________________________

	// Source offset that turns the cylinder by degrees, downwards (axis X) or to the right (axis Y) if positive,
	// as if startAngle was that much larger.

	WG_EXPORT wg_coordSPX	wg_cylinderWarpRotationOffset( wg_obj cylinderWarp, float degrees );

#ifdef __cplusplus
}
#endif

#endif
