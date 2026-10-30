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
#ifndef WAX_C_ROUNDEDRECTWARP_DOT_H
#define WAX_C_ROUNDEDRECTWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_roundedRectWarpBP __________________________________________________

	typedef struct wax_roundedRectWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wax_spx				bottomDistance;			// Distance from the outline of the bottom of the source, positive outwards.
		int					cellSize;				// 0 = calculated from precision.
		wax_spx				cornerRadius;			// Radius of the corners of the outline, at most half its shorter side.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		wax_rectSPX			rect;					// Mandatory. The outline, relative to the top-left corner of the Warpmap.
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels.
		wax_sizeSPX			sourceSize;				// Mandatory. Size of the source (edgemap or source rectangle) in spx.
		float				start;					// Where the left side of the source is placed, as a part of the outline from the middle of the top side.
		float				sweep;					// Part of the outline covered by the width of the source. Negative goes counter clockwise. 0 = default (1.0).
		wax_spx				topDistance;			// Distance from the outline of the top of the source, positive outwards.
	} wax_roundedRectWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, cell size, precision and generate are set and called through the WarpGenerator functions.

	WAX_EXPORT wax_obj		wax_createRoundedRectWarp( const wax_roundedRectWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setRoundedRectWarpOutline( wax_obj roundedRectWarp, const wax_rectSPX* rect, wax_spx cornerRadius );
	WAX_EXPORT wax_rectSPX		wax_roundedRectWarpRect( wax_obj roundedRectWarp );
	WAX_EXPORT wax_spx		wax_roundedRectWarpCornerRadius( wax_obj roundedRectWarp );

	WAX_EXPORT int			wax_setRoundedRectWarpDistances( wax_obj roundedRectWarp, wax_spx topDistance, wax_spx bottomDistance );
	WAX_EXPORT wax_spx		wax_roundedRectWarpTopDistance( wax_obj roundedRectWarp );
	WAX_EXPORT wax_spx		wax_roundedRectWarpBottomDistance( wax_obj roundedRectWarp );

	WAX_EXPORT int			wax_setRoundedRectWarpSpan( wax_obj roundedRectWarp, float start, float sweep );
	WAX_EXPORT float			wax_roundedRectWarpStart( wax_obj roundedRectWarp );
	WAX_EXPORT float			wax_roundedRectWarpSweep( wax_obj roundedRectWarp );

#ifdef __cplusplus
}
#endif

#endif
