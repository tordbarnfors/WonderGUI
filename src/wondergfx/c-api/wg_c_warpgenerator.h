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
#ifndef WG_C_WARPGENERATOR_DOT_H
#define WG_C_WARPGENERATOR_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Works on all WarpGenerators, such as CircularWarp.

	//.____ Geometry ______________________________________________________

	WG_EXPORT int			wg_setWarpGeneratorSize( wg_obj warpGenerator, wg_sizeI size );
	WG_EXPORT wg_sizeI		wg_warpGeneratorSize( wg_obj warpGenerator );

	WG_EXPORT int			wg_setWarpGeneratorSourceSize( wg_obj warpGenerator, wg_sizeSPX sourceSize );
	WG_EXPORT wg_sizeSPX		wg_warpGeneratorSourceSize( wg_obj warpGenerator );

	//.____ Precision _____________________________________________________

	WG_EXPORT int			wg_setWarpGeneratorCellSize( wg_obj warpGenerator, int cellSize );	// 0 = calculated from precision.
	WG_EXPORT int			wg_warpGeneratorCellSize( wg_obj warpGenerator );					// Cell size in use, calculated or set.

	WG_EXPORT int			wg_setWarpGeneratorPrecision( wg_obj warpGenerator, float precision );
	WG_EXPORT float			wg_warpGeneratorPrecision( wg_obj warpGenerator );

	//.____ Content _______________________________________________________

	// Generates the Warpmap from current settings and returns it, see WarpGenerator::generate(). The
	// Warpmap belongs to the WarpGenerator, retain it to keep it after a generate that replaces it.
	// A Warpmap you have retained is never changed by later calls, a new one is created instead.

	WG_EXPORT wg_obj		wg_generateWarpGenerator( wg_obj warpGenerator );

#ifdef __cplusplus
}
#endif

#endif
