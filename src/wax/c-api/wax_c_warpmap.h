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
#ifndef WG_C_WARPMAP_DOT_H
#define WG_C_WARPMAP_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>
#include <wg_c_geo.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_warpmapBP __________________________________________________________

	typedef struct wg_warpmapBP_struct			// NOT BINARY EQUIVALENT!
	{
		int					cellSize;				// Pixels between nodes. 0 = default (16).
		wg_sizeI			size;					// Mandatory. Size in pixels.
		float				uPeriod;				// 0 = not periodic.
		float				vPeriod;				// 0 = not periodic.
	} wg_warpmapBP;

	//.____ Geometry ______________________________________________________

	WG_EXPORT wg_sizeI		wg_warpmapPixelSize(wg_obj warpmap);
	WG_EXPORT int			wg_warpmapCellSize(wg_obj warpmap);
	WG_EXPORT wg_sizeI		wg_warpmapNodes(wg_obj warpmap);					// Nodes horizontally and vertically.
	WG_EXPORT int			wg_warpmapNodeX(wg_obj warpmap, int column);		// Pixel position of a column of nodes.
	WG_EXPORT int			wg_warpmapNodeY(wg_obj warpmap, int row);

	//.____ Content _______________________________________________________

	WG_EXPORT int			wg_setWarpmapPeriods(wg_obj warpmap, float uPeriod, float vPeriod);
	WG_EXPORT float			wg_warpmapUPeriod(wg_obj warpmap);
	WG_EXPORT float			wg_warpmapVPeriod(wg_obj warpmap);

	// Region is given in nodes, pitch is nodes between rows in source/destination, 0 meaning region width.

	WG_EXPORT int			wg_importWarpmapNodes(wg_obj warpmap, const wg_rectI* region, const wg_coordF* pSource, int pitch);
	WG_EXPORT int			wg_exportWarpmapNodes(wg_obj warpmap, const wg_rectI* region, wg_coordF* pDestination, int pitch);

	//.____ Misc __________________________________________________________

	WG_EXPORT wg_coordF		wg_warpmapPositionAt(wg_obj warpmap, double x, double y);

#ifdef __cplusplus
}
#endif

#endif
