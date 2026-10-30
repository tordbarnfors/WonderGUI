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
#include <wg_c_circularwarp.h>
#include <wg_circularwarp.h>

using namespace wg;

inline CircularWarp* getPtr(wg_obj obj) {
	return static_cast<CircularWarp*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createCircularWarp( const wg_circularWarpBP* pCBP, wg_obj warpmapFactory )
{
	CircularWarp::Blueprint bp;

	bp.bottomRadius = pCBP->bottomRadius;
	bp.cellSize = pCBP->cellSize;
	bp.center = { pCBP->center.x, pCBP->center.y };
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };
	bp.startAngle = pCBP->startAngle;
	bp.sweep = pCBP->sweep;
	bp.topRadius = pCBP->topRadius;

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = CircularWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

int wg_setCircularWarpCenter( wg_obj circularWarp, wg_coordF center )
{
	return getPtr(circularWarp)->setCenter({ center.x, center.y });
}

wg_coordF wg_circularWarpCenter( wg_obj circularWarp )
{
	CoordF c = getPtr(circularWarp)->center();
	return { c.x, c.y };
}

int wg_setCircularWarpRadii( wg_obj circularWarp, wg_spx topRadius, wg_spx bottomRadius )
{
	return getPtr(circularWarp)->setRadii(topRadius, bottomRadius);
}

wg_spx wg_circularWarpTopRadius( wg_obj circularWarp )
{
	return getPtr(circularWarp)->topRadius();
}

wg_spx wg_circularWarpBottomRadius( wg_obj circularWarp )
{
	return getPtr(circularWarp)->bottomRadius();
}

int wg_setCircularWarpAngles( wg_obj circularWarp, float startAngle, float sweep )
{
	return getPtr(circularWarp)->setAngles(startAngle, sweep);
}

float wg_circularWarpStartAngle( wg_obj circularWarp )
{
	return getPtr(circularWarp)->startAngle();
}

float wg_circularWarpSweep( wg_obj circularWarp )
{
	return getPtr(circularWarp)->sweep();
}
