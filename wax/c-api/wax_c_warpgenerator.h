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
#ifndef WAX_C_WARPGENERATOR_DOT_H
#define WAX_C_WARPGENERATOR_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Works on all WarpGenerators, such as CircularWarp.

	//.____ Geometry ______________________________________________________

	WAX_EXPORT int			wax_setWarpGeneratorSize( wax_obj warpGenerator, wax_sizeI size );
	WAX_EXPORT wax_sizeI		wax_warpGeneratorSize( wax_obj warpGenerator );

	//.____ Precision _____________________________________________________

	WAX_EXPORT int			wax_setWarpGeneratorCellSize( wax_obj warpGenerator, int cellSize );	// 0 = calculated from precision.
	WAX_EXPORT int			wax_warpGeneratorCellSize( wax_obj warpGenerator );					// Cell size in use, calculated or set.

	WAX_EXPORT int			wax_setWarpGeneratorPrecision( wax_obj warpGenerator, float precision );
	WAX_EXPORT float			wax_warpGeneratorPrecision( wax_obj warpGenerator );

	//.____ Content _______________________________________________________

	// Updates the Warpmap from current settings and returns it. The Warpmap belongs to the
	// WarpGenerator, retain it to keep it after a refresh that replaces it.

	WAX_EXPORT wax_obj		wax_refreshWarpGenerator( wax_obj warpGenerator );

#ifdef __cplusplus
}
#endif

#endif
