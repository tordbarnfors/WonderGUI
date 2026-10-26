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

using namespace wg;

inline Warpmap* getPtr(wg_obj obj) {
	return static_cast<Warpmap*>(reinterpret_cast<Object*>(obj));
}

static_assert(sizeof(wg_coordF) == sizeof(CoordF), "wg_coordF differs in size from wg::CoordF!");

wg_sizeI wg_warpmapPixelSize(wg_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->pixelSize();
	return { sz.w, sz.h };
}

int wg_warpmapCellSize(wg_obj warpmap)
{
	return getPtr(warpmap)->cellSize();
}

wg_sizeI wg_warpmapNodes(wg_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->nodes();
	return { sz.w, sz.h };
}

int wg_warpmapNodeX(wg_obj warpmap, int column)
{
	return getPtr(warpmap)->nodeX(column);
}

int wg_warpmapNodeY(wg_obj warpmap, int row)
{
	return getPtr(warpmap)->nodeY(row);
}

int wg_setWarpmapPeriods(wg_obj warpmap, float uPeriod, float vPeriod)
{
	return getPtr(warpmap)->setPeriods(uPeriod, vPeriod);
}

float wg_warpmapUPeriod(wg_obj warpmap)
{
	return getPtr(warpmap)->uPeriod();
}

float wg_warpmapVPeriod(wg_obj warpmap)
{
	return getPtr(warpmap)->vPeriod();
}

int wg_importWarpmapNodes(wg_obj warpmap, const wg_rectI* region, const wg_coordF* pSource, int pitch)
{
	return getPtr(warpmap)->importNodes(*(const RectI*)region, (const CoordF*)pSource, pitch);
}

int wg_exportWarpmapNodes(wg_obj warpmap, const wg_rectI* region, wg_coordF* pDestination, int pitch)
{
	return getPtr(warpmap)->exportNodes(*(const RectI*)region, (CoordF*)pDestination, pitch);
}

wg_coordF wg_warpmapPositionAt(wg_obj warpmap, double x, double y)
{
	CoordF pos = getPtr(warpmap)->positionAt(x, y);
	return { pos.x, pos.y };
}
