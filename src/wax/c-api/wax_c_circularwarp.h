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
#ifndef WG_C_CIRCULARWARP_DOT_H
#define WG_C_CIRCULARWARP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_circularWarpBP _____________________________________________________

	typedef struct wg_circularWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wg_spx				bottomRadius;			// Radius the bottom of the source is placed at.
		int					cellSize;				// 0 = calculated from precision and detailRadius.
		wg_coordF			center;					// Center of circle, relative to size. Usually { 0.5f, 0.5f }.
		wg_spx				detailRadius;			// Smallest radius where precision is needed. 0 = smallest of the radii.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		wg_sizeI			size;					// Mandatory. Size of the Warpmap in pixels.
		float				startAngle;				// Degrees. Where the left side of the source is placed. 0 = up, clockwise.
		float				sweep;					// Degrees covered by the width of the source. Negative goes counter clockwise.
		wg_spx				topRadius;				// Radius the top of the source is placed at.
	} wg_circularWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.

	WG_EXPORT wg_obj		wg_createCircularWarp( const wg_circularWarpBP* blueprint, wg_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WG_EXPORT int			wg_setCircularWarpSize( wg_obj circularWarp, wg_sizeI size );
	WG_EXPORT wg_sizeI		wg_circularWarpSize( wg_obj circularWarp );

	WG_EXPORT int			wg_setCircularWarpCenter( wg_obj circularWarp, wg_coordF center );
	WG_EXPORT wg_coordF		wg_circularWarpCenter( wg_obj circularWarp );

	WG_EXPORT int			wg_setCircularWarpRadii( wg_obj circularWarp, wg_spx topRadius, wg_spx bottomRadius );
	WG_EXPORT wg_spx		wg_circularWarpTopRadius( wg_obj circularWarp );
	WG_EXPORT wg_spx		wg_circularWarpBottomRadius( wg_obj circularWarp );

	WG_EXPORT int			wg_setCircularWarpAngles( wg_obj circularWarp, float startAngle, float sweep );
	WG_EXPORT float			wg_circularWarpStartAngle( wg_obj circularWarp );
	WG_EXPORT float			wg_circularWarpSweep( wg_obj circularWarp );

	//.____ Precision _____________________________________________________

	WG_EXPORT int			wg_setCircularWarpCellSize( wg_obj circularWarp, int cellSize );
	WG_EXPORT int			wg_circularWarpCellSize( wg_obj circularWarp );

	WG_EXPORT int			wg_setCircularWarpPrecision( wg_obj circularWarp, float precision, wg_spx detailRadius );

	//.____ Content _______________________________________________________

	// Updates the Warpmap from current settings and returns it. The Warpmap belongs to the
	// CircularWarp, retain it to keep it after a refresh that replaces it.

	WG_EXPORT wg_obj		wg_refreshCircularWarp( wg_obj circularWarp );

#ifdef __cplusplus
}
#endif

#endif
