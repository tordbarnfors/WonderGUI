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
#include <wg_c_warpmap.h>
#include <wg_c_internal.h>
#include <wg_warpmap.h>

#include <cstddef>

using namespace wg;

inline Warpmap* getPtr(wg_obj obj) {
	return static_cast<Warpmap*>(reinterpret_cast<Object*>(obj));
}

static_assert(sizeof(wg_warpVertex) == sizeof(Warpmap::Vertex) && offsetof(wg_warpVertex, w) == offsetof(Warpmap::Vertex, w) &&
			  offsetof(wg_warpVertex, coverage) == offsetof(Warpmap::Vertex, coverage), "wg_warpVertex differs from wg::Warpmap::Vertex!");
static_assert(sizeof(wg_warpTriangle) == sizeof(Warpmap::Triangle), "wg_warpTriangle differs in size from wg::Warpmap::Triangle!");

wg_sizeI wg_warpmapPixelSize(wg_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->pixelSize();
	return { sz.w, sz.h };
}

int wg_warpmapVertexCount(wg_obj warpmap)
{
	return getPtr(warpmap)->vertexCount();
}

int wg_warpmapTriangleCount(wg_obj warpmap)
{
	return getPtr(warpmap)->triangleCount();
}

wg_spx wg_warpmapUPeriod(wg_obj warpmap)
{
	return getPtr(warpmap)->uPeriod();
}

wg_spx wg_warpmapVPeriod(wg_obj warpmap)
{
	return getPtr(warpmap)->vPeriod();
}

int wg_setWarpmapPeriods(wg_obj warpmap, wg_spx uPeriod, wg_spx vPeriod)
{
	return getPtr(warpmap)->setPeriods(uPeriod, vPeriod);
}

int wg_importWarpmapVertices(wg_obj warpmap, int first, int count, const wg_warpVertex* pSource)
{
	return getPtr(warpmap)->importVertices(first, count, (const Warpmap::Vertex*) pSource);
}

int wg_importWarpmapTriangles(wg_obj warpmap, int first, int count, const wg_warpTriangle* pSource)
{
	return getPtr(warpmap)->importTriangles(first, count, (const Warpmap::Triangle*) pSource);
}

int wg_importWarpmapMesh(wg_obj warpmap, const wg_warpVertex* pVertices, const wg_warpTriangle* pTriangles, wg_spx uPeriod, wg_spx vPeriod)
{
	return getPtr(warpmap)->importMesh((const Warpmap::Vertex*) pVertices, (const Warpmap::Triangle*) pTriangles, uPeriod, vPeriod);
}

int wg_exportWarpmapVertices(wg_obj warpmap, int first, int count, wg_warpVertex* pDestination)
{
	return getPtr(warpmap)->exportVertices(first, count, (Warpmap::Vertex*) pDestination);
}

int wg_exportWarpmapTriangles(wg_obj warpmap, int first, int count, wg_warpTriangle* pDestination)
{
	return getPtr(warpmap)->exportTriangles(first, count, (Warpmap::Triangle*) pDestination);
}

int wg_warpmapTriangleAt(wg_obj warpmap, wg_coordSPX point)
{
	return getPtr(warpmap)->triangleAt({ point.x, point.y });
}

int wg_warpmapPositionIn(wg_obj warpmap, int triangle, double x, double y, wg_coordF* pPosition)
{
	CoordF pos;
	if (!getPtr(warpmap)->positionIn(triangle, x, y, pos))
		return 0;

	if (pPosition)
		*pPosition = { pos.x, pos.y };
	return 1;
}
