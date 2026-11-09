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
#ifndef WAX_C_CYLINDERWARP_DOT_H
#define WAX_C_CYLINDERWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_cylinderWarpBP _____________________________________________________

	typedef struct wax_cylinderWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wax_axis				axis;					// Axis of the cylinder. WAX_AXIS_X lies down, like a slot machine reel. 0 = default (WAX_AXIS_X).
		float				precision;				// Max position error in pixels, of outlines at most 0.5. 0 = default (1).
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels, which the cylinder fills.
		wax_sizeSPX			sourceSize;				// Mandatory. Size of the source (edgemap or source rectangle) in spx.
		float				span;					// Degrees of the cylinder seen, above 0 and below 180. 0 = default (120).
		float				startAngle;				// Degrees. Where the start of the source across the axis is, 0 = facing the viewer.
		float				sweep;					// Degrees around the cylinder covered by the source. Negative goes the other way. 0 = default (360).
	} wax_cylinderWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, precision and generate are set and called through the WarpGenerator functions.

	WAX_EXPORT wax_obj		wax_createCylinderWarp( const wax_cylinderWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setCylinderWarpAxis( wax_obj cylinderWarp, wax_axis axis );
	WAX_EXPORT wax_axis		wax_cylinderWarpAxis( wax_obj cylinderWarp );

	WAX_EXPORT int			wax_setCylinderWarpSpan( wax_obj cylinderWarp, float span );
	WAX_EXPORT float			wax_cylinderWarpSpan( wax_obj cylinderWarp );

	WAX_EXPORT int			wax_setCylinderWarpAngles( wax_obj cylinderWarp, float startAngle, float sweep );
	WAX_EXPORT float			wax_cylinderWarpStartAngle( wax_obj cylinderWarp );
	WAX_EXPORT float			wax_cylinderWarpSweep( wax_obj cylinderWarp );

	//.____ Drawing _______________________________________________________

	// Source offset that turns the cylinder by degrees, downwards (axis X) or to the right (axis Y) if positive,
	// as if startAngle was that much larger.

	WAX_EXPORT wax_coordSPX	wax_cylinderWarpRotationOffset( wax_obj cylinderWarp, float degrees );

#ifdef __cplusplus
}
#endif

#endif
