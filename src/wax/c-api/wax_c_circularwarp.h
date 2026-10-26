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
#ifndef WAX_C_CIRCULARWARP_DOT_H
#define WAX_C_CIRCULARWARP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_circularWarpBP _____________________________________________________

	typedef struct wax_circularWarpBP_struct		// NOT BINARY EQUIVALENT!
	{
		wax_spx				bottomRadius;			// Radius the bottom of the source is placed at.
		int					cellSize;				// 0 = calculated from precision and detailRadius.
		wax_coordF			center;					// Center of circle, relative to size. Usually { 0.5f, 0.5f }.
		wax_spx				detailRadius;			// Smallest radius where precision is needed. 0 = smallest of the radii.
		float				precision;				// Max position error in pixels when cellSize is calculated. 0 = default (0.25).
		wax_sizeI			size;					// Mandatory. Size of the Warpmap in pixels.
		float				startAngle;				// Degrees. Where the left side of the source is placed. 0 = up, clockwise.
		float				sweep;					// Degrees covered by the width of the source. Negative goes counter clockwise.
		wax_spx				topRadius;				// Radius the top of the source is placed at.
	} wax_circularWarpBP;

	// Factory is the WarpmapFactory to create the Warpmap with, the default one if null.

	WAX_EXPORT wax_obj		wax_createCircularWarp( const wax_circularWarpBP* blueprint, wax_obj warpmapFactory );

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setCircularWarpSize( wax_obj circularWarp, wax_sizeI size );
	WAX_EXPORT wax_sizeI		wax_circularWarpSize( wax_obj circularWarp );

	WAX_EXPORT int			wax_setCircularWarpCenter( wax_obj circularWarp, wax_coordF center );
	WAX_EXPORT wax_coordF		wax_circularWarpCenter( wax_obj circularWarp );

	WAX_EXPORT int			wax_setCircularWarpRadii( wax_obj circularWarp, wax_spx topRadius, wax_spx bottomRadius );
	WAX_EXPORT wax_spx		wax_circularWarpTopRadius( wax_obj circularWarp );
	WAX_EXPORT wax_spx		wax_circularWarpBottomRadius( wax_obj circularWarp );

	WAX_EXPORT int			wax_setCircularWarpAngles( wax_obj circularWarp, float startAngle, float sweep );
	WAX_EXPORT float			wax_circularWarpStartAngle( wax_obj circularWarp );
	WAX_EXPORT float			wax_circularWarpSweep( wax_obj circularWarp );

	//.____ Precision _____________________________________________________

	WAX_EXPORT int			wax_setCircularWarpCellSize( wax_obj circularWarp, int cellSize );
	WAX_EXPORT int			wax_circularWarpCellSize( wax_obj circularWarp );

	WAX_EXPORT int			wax_setCircularWarpPrecision( wax_obj circularWarp, float precision, wax_spx detailRadius );

	//.____ Content _______________________________________________________

	// Updates the Warpmap from current settings and returns it. The Warpmap belongs to the
	// CircularWarp, retain it to keep it after a refresh that replaces it.

	WAX_EXPORT wax_obj		wax_refreshCircularWarp( wax_obj circularWarp );

#ifdef __cplusplus
}
#endif

#endif
