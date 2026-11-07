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
#ifndef WAX_C_PERSPECTIVEWARP_DOT_H
#define WAX_C_PERSPECTIVEWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_perspectiveWarpBP __________________________________________________

	typedef struct wax_perspectiveWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wax_coordSPX		corners[4];				// Mandatory. Where the corners of the source are placed: top-left, top-right, bottom-right, bottom-left.
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels.
		wax_sizeSPX			sourceSize;				// Mandatory. Size of the source (edgemap or source rectangle) in spx.
	} wax_perspectiveWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size and generate are set and called through the WarpGenerator functions. Cell size and precision don't apply.

	WAX_EXPORT wax_obj		wax_createPerspectiveWarp( const wax_perspectiveWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	// Corners are top-left, top-right, bottom-right and bottom-left. They must make a convex quadrilateral.

	WAX_EXPORT int			wax_setPerspectiveWarpCorners( wax_obj perspectiveWarp, const wax_coordSPX corners[4] );
	WAX_EXPORT void			wax_perspectiveWarpCorners( wax_obj perspectiveWarp, wax_coordSPX corners[4] );

	//.____ Misc __________________________________________________________

	// Corners of rect after rotating it in 3D (roll, then turn, then tilt, in degrees around the pivot) and seeing it
	// from an eye viewDistance in front of the screen, see PerspectiveWarp::rotated(). pEye is null for straight in
	// front of the pivot.

	WAX_EXPORT void			wax_perspectiveWarpRotated( const wax_rectSPX* rect, float tilt, float turn, float roll, wax_spx viewDistance,
														wax_coordF pivot, const wax_coordSPX* pEye, wax_coordSPX corners[4] );

#ifdef __cplusplus
}
#endif

#endif
