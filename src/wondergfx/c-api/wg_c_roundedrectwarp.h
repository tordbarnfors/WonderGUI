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
#ifndef WG_C_ROUNDEDRECTWARP_DOT_H
#define WG_C_ROUNDEDRECTWARP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_roundedRectWarpBP __________________________________________________

	typedef struct wg_roundedRectWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wg_spx				bottomDistance;			// Distance from the outline of the bottom of the source, positive outwards.
		int					cellSize;				// 0 = calculated from precision.
		wg_spx				cornerRadius;			// Radius of the corners of the outline, at most half its shorter side.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		wg_rectSPX			rect;					// Mandatory. The outline, relative to the top-left corner of the Warpmap.
		wg_sizeI			size;					// Mandatory. Size of the Warpmap in pixels.
		wg_sizeSPX			sourceSize;				// Mandatory. Size of the source (edgemap or source rectangle) in spx.
		float				start;					// Where the left side of the source is placed, as a part of the outline from the middle of the top side.
		float				sweep;					// Part of the outline covered by the width of the source. Negative goes counter clockwise. 0 = default (1.0).
		wg_spx				topDistance;			// Distance from the outline of the top of the source, positive outwards.
	} wg_roundedRectWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.
	// Size, source size, cell size, precision and generate are set and called through the WarpGenerator functions.

	WG_EXPORT wg_obj		wg_createRoundedRectWarp( const wg_roundedRectWarpBP* blueprint, wg_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WG_EXPORT int			wg_setRoundedRectWarpOutline( wg_obj roundedRectWarp, const wg_rectSPX* rect, wg_spx cornerRadius );
	WG_EXPORT wg_rectSPX		wg_roundedRectWarpRect( wg_obj roundedRectWarp );
	WG_EXPORT wg_spx		wg_roundedRectWarpCornerRadius( wg_obj roundedRectWarp );

	WG_EXPORT int			wg_setRoundedRectWarpDistances( wg_obj roundedRectWarp, wg_spx topDistance, wg_spx bottomDistance );
	WG_EXPORT wg_spx		wg_roundedRectWarpTopDistance( wg_obj roundedRectWarp );
	WG_EXPORT wg_spx		wg_roundedRectWarpBottomDistance( wg_obj roundedRectWarp );

	WG_EXPORT int			wg_setRoundedRectWarpSpan( wg_obj roundedRectWarp, float start, float sweep );
	WG_EXPORT float			wg_roundedRectWarpStart( wg_obj roundedRectWarp );
	WG_EXPORT float			wg_roundedRectWarpSweep( wg_obj roundedRectWarp );

#ifdef __cplusplus
}
#endif

#endif
