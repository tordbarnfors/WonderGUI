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

using namespace wax;

inline Warpmap* getPtr(wax_obj obj) {
	return static_cast<Warpmap*>(reinterpret_cast<Object*>(obj));
}

static_assert(sizeof(wax_coordSPX) == sizeof(CoordSPX), "wax_coordSPX differs in size from wax::CoordSPX!");

wax_sizeI wax_warpmapPixelSize(wax_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->pixelSize();
	return { sz.w, sz.h };
}

int wax_warpmapCellSize(wax_obj warpmap)
{
	return getPtr(warpmap)->cellSize();
}

wax_sizeI wax_warpmapNodes(wax_obj warpmap)
{
	SizeI sz = getPtr(warpmap)->nodes();
	return { sz.w, sz.h };
}

int wax_warpmapNodeX(wax_obj warpmap, int column)
{
	return getPtr(warpmap)->nodeX(column);
}

int wax_warpmapNodeY(wax_obj warpmap, int row)
{
	return getPtr(warpmap)->nodeY(row);
}

int wax_setWarpmapPeriods(wax_obj warpmap, wax_spx uPeriod, wax_spx vPeriod)
{
	return getPtr(warpmap)->setPeriods(uPeriod, vPeriod);
}

wax_spx wax_warpmapUPeriod(wax_obj warpmap)
{
	return getPtr(warpmap)->uPeriod();
}

wax_spx wax_warpmapVPeriod(wax_obj warpmap)
{
	return getPtr(warpmap)->vPeriod();
}

int wax_importWarpmapNodes(wax_obj warpmap, const wax_rectI* region, const wax_coordSPX* pSource, int pitch)
{
	return getPtr(warpmap)->importNodes(*(const RectI*)region, (const CoordSPX*)pSource, pitch);
}

int wax_exportWarpmapNodes(wax_obj warpmap, const wax_rectI* region, wax_coordSPX* pDestination, int pitch)
{
	return getPtr(warpmap)->exportNodes(*(const RectI*)region, (CoordSPX*)pDestination, pitch);
}

wax_coordF wax_warpmapPositionAt(wax_obj warpmap, double x, double y)
{
	CoordF pos = getPtr(warpmap)->positionAt(x, y);
	return { pos.x, pos.y };
}
