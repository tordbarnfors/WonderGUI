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
#ifndef WAX_C_WARPMAP_DOT_H
#define WAX_C_WARPMAP_DOT_H
#pragma once

#include <wax_c_types.h>
#include <wax_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_warpmapBP __________________________________________________________

	typedef struct wax_warpmapBP_struct			// NOT BINARY EQUIVALENT!
	{
		int					cellSize;				// Pixels between nodes. 0 = default (16).
		wax_sizeI			size;					// Mandatory. Size in pixels.
		float				uPeriod;				// 0 = not periodic.
		float				vPeriod;				// 0 = not periodic.
	} wax_warpmapBP;

	//.____ Geometry ______________________________________________________

	WAX_EXPORT wax_sizeI		wax_warpmapPixelSize(wax_obj warpmap);
	WAX_EXPORT int			wax_warpmapCellSize(wax_obj warpmap);
	WAX_EXPORT wax_sizeI		wax_warpmapNodes(wax_obj warpmap);					// Nodes horizontally and vertically.
	WAX_EXPORT int			wax_warpmapNodeX(wax_obj warpmap, int column);		// Pixel position of a column of nodes.
	WAX_EXPORT int			wax_warpmapNodeY(wax_obj warpmap, int row);

	//.____ Content _______________________________________________________

	WAX_EXPORT int			wax_setWarpmapPeriods(wax_obj warpmap, float uPeriod, float vPeriod);
	WAX_EXPORT float			wax_warpmapUPeriod(wax_obj warpmap);
	WAX_EXPORT float			wax_warpmapVPeriod(wax_obj warpmap);

	// Region is given in nodes, pitch is nodes between rows in source/destination, 0 meaning region width.

	WAX_EXPORT int			wax_importWarpmapNodes(wax_obj warpmap, const wax_rectI* region, const wax_coordF* pSource, int pitch);
	WAX_EXPORT int			wax_exportWarpmapNodes(wax_obj warpmap, const wax_rectI* region, wax_coordF* pDestination, int pitch);

	//.____ Misc __________________________________________________________

	WAX_EXPORT wax_coordF		wax_warpmapPositionAt(wax_obj warpmap, double x, double y);

#ifdef __cplusplus
}
#endif

#endif
