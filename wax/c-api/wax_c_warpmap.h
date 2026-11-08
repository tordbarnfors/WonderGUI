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
		wax_sizeI			size;					// Mandatory. Size in pixels.
		int					triangles;				// Number of triangles. 0 = default (2).
		wax_spx				uPeriod;				// In spx of the source, 0 = not periodic.
		int					vertices;				// Number of vertices. 0 = default (4).
		wax_spx				vPeriod;				// In spx of the source, 0 = not periodic.
	} wax_warpmapBP;

	//____ wax_warpVertex _________________________________________________________

	typedef struct wax_warpVertex_struct		// Binary equivalent of wax::Warpmap::Vertex.
	{
		wax_spx				x, y;					// Position in spx of the Warpmap, from its top-left corner.
		wax_spx				u, v;					// Position in spx of the source.
		int					w;						// Weight for perspective, 65536 is 1.0.
		int					coverage;				// Coverage, 65536 is full, see wax::Warpmap.
	} wax_warpVertex;

	//____ wax_warpTriangle _______________________________________________________

	typedef struct wax_warpTriangle_struct		// Binary equivalent of wax::Warpmap::Triangle.
	{
		uint16_t			a, b, c;				// Indices of the corners in the vertices.
	} wax_warpTriangle;

	//.____ Geometry ______________________________________________________

	WAX_EXPORT wax_sizeI		wax_warpmapPixelSize(wax_obj warpmap);

	//.____ Content _______________________________________________________

	WAX_EXPORT int			wax_warpmapVertexCount(wax_obj warpmap);
	WAX_EXPORT int			wax_warpmapTriangleCount(wax_obj warpmap);

	WAX_EXPORT wax_spx			wax_warpmapUPeriod(wax_obj warpmap);
	WAX_EXPORT wax_spx			wax_warpmapVPeriod(wax_obj warpmap);
	WAX_EXPORT int			wax_setWarpmapPeriods(wax_obj warpmap, wax_spx uPeriod, wax_spx vPeriod);

	WAX_EXPORT int			wax_importWarpmapVertices(wax_obj warpmap, int first, int count, const wax_warpVertex* pSource);
	WAX_EXPORT int			wax_importWarpmapTriangles(wax_obj warpmap, int first, int count, const wax_warpTriangle* pSource);

	// All vertices and triangles, and periods. Either pointer can be null to keep what is there.

	WAX_EXPORT int			wax_importWarpmapMesh(wax_obj warpmap, const wax_warpVertex* pVertices, const wax_warpTriangle* pTriangles, wax_spx uPeriod, wax_spx vPeriod);

	WAX_EXPORT int			wax_exportWarpmapVertices(wax_obj warpmap, int first, int count, wax_warpVertex* pDestination);
	WAX_EXPORT int			wax_exportWarpmapTriangles(wax_obj warpmap, int first, int count, wax_warpTriangle* pDestination);

	//.____ Misc __________________________________________________________

	// Triangle that a point (spx of the Warpmap) is inside of, -1 if none.

	WAX_EXPORT int			wax_warpmapTriangleAt(wax_obj warpmap, wax_coordSPX point);

	// Position at x,y (pixels of the Warpmap) as given by a triangle, in spx of the source with fractions.
	// Returns 0 beyond the horizon of the triangle or if it has no area.

	WAX_EXPORT int			wax_warpmapPositionIn(wax_obj warpmap, int triangle, double x, double y, wax_coordF* pPosition);

#ifdef __cplusplus
}
#endif

#endif
