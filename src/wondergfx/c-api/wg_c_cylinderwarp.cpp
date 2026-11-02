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
#include <wg_c_cylinderwarp.h>
#include <wg_cylinderwarp.h>

using namespace wg;

inline CylinderWarp* getPtr(wg_obj obj) {
	return static_cast<CylinderWarp*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createCylinderWarp( const wg_cylinderWarpBP* pCBP, wg_obj warpmapFactory )
{
	CylinderWarp::Blueprint bp;

	if (pCBP->axis != WG_AXIS_UNDEFINED)
		bp.axis = (Axis) pCBP->axis;
	bp.cellSize = { pCBP->cellSize.w, pCBP->cellSize.h };
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };
	if (pCBP->span != 0.f)
		bp.span = pCBP->span;
	bp.startAngle = pCBP->startAngle;
	if (pCBP->sweep != 0.f)
		bp.sweep = pCBP->sweep;

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = CylinderWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

int wg_setCylinderWarpAxis( wg_obj cylinderWarp, wg_axis axis )
{
	return getPtr(cylinderWarp)->setAxis((Axis) axis);
}

wg_axis wg_cylinderWarpAxis( wg_obj cylinderWarp )
{
	return (wg_axis) getPtr(cylinderWarp)->axis();
}

int wg_setCylinderWarpSpan( wg_obj cylinderWarp, float span )
{
	return getPtr(cylinderWarp)->setSpan(span);
}

float wg_cylinderWarpSpan( wg_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->span();
}

int wg_setCylinderWarpAngles( wg_obj cylinderWarp, float startAngle, float sweep )
{
	return getPtr(cylinderWarp)->setAngles(startAngle, sweep);
}

float wg_cylinderWarpStartAngle( wg_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->startAngle();
}

float wg_cylinderWarpSweep( wg_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->sweep();
}

wg_coordSPX wg_cylinderWarpRotationOffset( wg_obj cylinderWarp, float degrees )
{
	CoordSPX offset = getPtr(cylinderWarp)->rotationOffset(degrees);
	return { offset.x, offset.y };
}
