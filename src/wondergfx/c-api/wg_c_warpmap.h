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
		wg_sizeI			size;					// Mandatory. Size in pixels.
		int					triangles;				// Number of triangles. 0 = default (2).
		wg_spx				uPeriod;				// In spx of the source, 0 = not periodic.
		int					vertices;				// Number of vertices. 0 = default (4).
		wg_spx				vPeriod;				// In spx of the source, 0 = not periodic.
	} wg_warpmapBP;

	//____ wg_warpVertex _________________________________________________________

	typedef struct wg_warpVertex_struct		// Binary equivalent of wg::Warpmap::Vertex.
	{
		wg_spx				x, y;					// Position in spx of the Warpmap, from its top-left corner.
		wg_spx				u, v;					// Position in spx of the source.
		int					w;						// Weight for perspective, 65536 is 1.0.
		int					coverage;				// Coverage, 65536 is full, see wg::Warpmap.
	} wg_warpVertex;

	//____ wg_warpTriangle _______________________________________________________

	typedef struct wg_warpTriangle_struct		// Binary equivalent of wg::Warpmap::Triangle.
	{
		uint16_t			a, b, c;				// Indices of the corners in the vertices.
	} wg_warpTriangle;

	//.____ Geometry ______________________________________________________

	WG_EXPORT wg_sizeI		wg_warpmapPixelSize(wg_obj warpmap);

	//.____ Content _______________________________________________________

	WG_EXPORT int			wg_warpmapVertexCount(wg_obj warpmap);
	WG_EXPORT int			wg_warpmapTriangleCount(wg_obj warpmap);

	WG_EXPORT wg_spx			wg_warpmapUPeriod(wg_obj warpmap);
	WG_EXPORT wg_spx			wg_warpmapVPeriod(wg_obj warpmap);
	WG_EXPORT int			wg_setWarpmapPeriods(wg_obj warpmap, wg_spx uPeriod, wg_spx vPeriod);

	WG_EXPORT int			wg_importWarpmapVertices(wg_obj warpmap, int first, int count, const wg_warpVertex* pSource);
	WG_EXPORT int			wg_importWarpmapTriangles(wg_obj warpmap, int first, int count, const wg_warpTriangle* pSource);

	// All vertices and triangles, and periods. Either pointer can be null to keep what is there.

	WG_EXPORT int			wg_importWarpmapMesh(wg_obj warpmap, const wg_warpVertex* pVertices, const wg_warpTriangle* pTriangles, wg_spx uPeriod, wg_spx vPeriod);

	WG_EXPORT int			wg_exportWarpmapVertices(wg_obj warpmap, int first, int count, wg_warpVertex* pDestination);
	WG_EXPORT int			wg_exportWarpmapTriangles(wg_obj warpmap, int first, int count, wg_warpTriangle* pDestination);

	//.____ Misc __________________________________________________________

	// Triangle that a point (spx of the Warpmap) is inside of, -1 if none.

	WG_EXPORT int			wg_warpmapTriangleAt(wg_obj warpmap, wg_coordSPX point);

	// Position at x,y (pixels of the Warpmap) as given by a triangle, in spx of the source with fractions.
	// Returns 0 beyond the horizon of the triangle or if it has no area.

	WG_EXPORT int			wg_warpmapPositionIn(wg_obj warpmap, int triangle, double x, double y, wg_coordF* pPosition);

#ifdef __cplusplus
}
#endif

#endif
