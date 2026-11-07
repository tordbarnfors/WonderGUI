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
#include <wax_c_warpmap.h>
#include <wax_c_internal.h>
#include <wax_warpmap.h>

#include <cstddef>

using namespace wax;

inline Warpmap* getPtr(wax_obj obj) {
	return static_cast<Warpmap*>(reinterpret_cast<Object*>(obj));
}

static_assert(sizeof(wax_warpVertex) == sizeof(Warpmap::Vertex) && offsetof(wax_warpVertex, w) == offsetof(Warpmap::Vertex, w), "wax_warpVertex differs from wax::Warpmap::Vertex!");
static_assert(sizeof(wax_warpTriangle) == sizeof(Warpmap::Triangle), "wax_warpTriangle differs in size from wax::Warpmap::Triangle!");

wax_sizeI wax_warpmapPixelSize(wax_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->pixelSize();
	return { sz.w, sz.h };
}

int wax_warpmapVertexCount(wax_obj warpmap)
{
	return getPtr(warpmap)->vertexCount();
}

int wax_warpmapTriangleCount(wax_obj warpmap)
{
	return getPtr(warpmap)->triangleCount();
}

wax_spx wax_warpmapUPeriod(wax_obj warpmap)
{
	return getPtr(warpmap)->uPeriod();
}

wax_spx wax_warpmapVPeriod(wax_obj warpmap)
{
	return getPtr(warpmap)->vPeriod();
}

int wax_setWarpmapPeriods(wax_obj warpmap, wax_spx uPeriod, wax_spx vPeriod)
{
	return getPtr(warpmap)->setPeriods(uPeriod, vPeriod);
}

int wax_importWarpmapVertices(wax_obj warpmap, int first, int count, const wax_warpVertex* pSource)
{
	return getPtr(warpmap)->importVertices(first, count, (const Warpmap::Vertex*) pSource);
}

int wax_importWarpmapTriangles(wax_obj warpmap, int first, int count, const wax_warpTriangle* pSource)
{
	return getPtr(warpmap)->importTriangles(first, count, (const Warpmap::Triangle*) pSource);
}

int wax_importWarpmapMesh(wax_obj warpmap, const wax_warpVertex* pVertices, const wax_warpTriangle* pTriangles, wax_spx uPeriod, wax_spx vPeriod)
{
	return getPtr(warpmap)->importMesh((const Warpmap::Vertex*) pVertices, (const Warpmap::Triangle*) pTriangles, uPeriod, vPeriod);
}

int wax_exportWarpmapVertices(wax_obj warpmap, int first, int count, wax_warpVertex* pDestination)
{
	return getPtr(warpmap)->exportVertices(first, count, (Warpmap::Vertex*) pDestination);
}

int wax_exportWarpmapTriangles(wax_obj warpmap, int first, int count, wax_warpTriangle* pDestination)
{
	return getPtr(warpmap)->exportTriangles(first, count, (Warpmap::Triangle*) pDestination);
}

int wax_warpmapTriangleAt(wax_obj warpmap, wax_coordSPX point)
{
	return getPtr(warpmap)->triangleAt({ point.x, point.y });
}

int wax_warpmapPositionIn(wax_obj warpmap, int triangle, double x, double y, wax_coordF* pPosition)
{
	CoordF pos;
	if (!getPtr(warpmap)->positionIn(triangle, x, y, pos))
		return 0;

	if (pPosition)
		*pPosition = { pos.x, pos.y };
	return 1;
}
