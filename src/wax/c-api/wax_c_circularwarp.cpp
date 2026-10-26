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
#include <wax_c_circularwarp.h>
#include <wax_circularwarp.h>

using namespace wax;

inline CircularWarp* getPtr(wax_obj obj) {
	return static_cast<CircularWarp*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createCircularWarp( const wax_circularWarpBP* pCBP, wax_obj warpmapFactory )
{
	CircularWarp::Blueprint bp;

	bp.bottomRadius = pCBP->bottomRadius;
	bp.cellSize = pCBP->cellSize;
	bp.center = { pCBP->center.x, pCBP->center.y };
	bp.detailRadius = pCBP->detailRadius;
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.startAngle = pCBP->startAngle;
	bp.sweep = pCBP->sweep;
	bp.topRadius = pCBP->topRadius;

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = CircularWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setCircularWarpSize( wax_obj circularWarp, wax_sizeI size )
{
	return getPtr(circularWarp)->setSize({ size.w, size.h });
}

wax_sizeI wax_circularWarpSize( wax_obj circularWarp )
{
	SizeI sz = getPtr(circularWarp)->size();
	return { sz.w, sz.h };
}

int wax_setCircularWarpCenter( wax_obj circularWarp, wax_coordF center )
{
	return getPtr(circularWarp)->setCenter({ center.x, center.y });
}

wax_coordF wax_circularWarpCenter( wax_obj circularWarp )
{
	CoordF c = getPtr(circularWarp)->center();
	return { c.x, c.y };
}

int wax_setCircularWarpRadii( wax_obj circularWarp, wax_spx topRadius, wax_spx bottomRadius )
{
	return getPtr(circularWarp)->setRadii(topRadius, bottomRadius);
}

wax_spx wax_circularWarpTopRadius( wax_obj circularWarp )
{
	return getPtr(circularWarp)->topRadius();
}

wax_spx wax_circularWarpBottomRadius( wax_obj circularWarp )
{
	return getPtr(circularWarp)->bottomRadius();
}

int wax_setCircularWarpAngles( wax_obj circularWarp, float startAngle, float sweep )
{
	return getPtr(circularWarp)->setAngles(startAngle, sweep);
}

float wax_circularWarpStartAngle( wax_obj circularWarp )
{
	return getPtr(circularWarp)->startAngle();
}

float wax_circularWarpSweep( wax_obj circularWarp )
{
	return getPtr(circularWarp)->sweep();
}

int wax_setCircularWarpCellSize( wax_obj circularWarp, int cellSize )
{
	return getPtr(circularWarp)->setCellSize(cellSize);
}

int wax_circularWarpCellSize( wax_obj circularWarp )
{
	return getPtr(circularWarp)->cellSize();
}

int wax_setCircularWarpPrecision( wax_obj circularWarp, float precision, wax_spx detailRadius )
{
	return getPtr(circularWarp)->setPrecision(precision, detailRadius);
}

wax_obj wax_refreshCircularWarp( wax_obj circularWarp )
{
	Warpmap_p p = getPtr(circularWarp)->refresh();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}
