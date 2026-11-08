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
#include <wax_c_cylinderwarp.h>
#include <wax_cylinderwarp.h>

using namespace wax;

inline CylinderWarp* getPtr(wax_obj obj) {
	return static_cast<CylinderWarp*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createCylinderWarp( const wax_cylinderWarpBP* pCBP, wax_obj warpmapFactory )
{
	CylinderWarp::Blueprint bp;

	if (pCBP->axis != WAX_AXIS_UNDEFINED)
		bp.axis = (Axis) pCBP->axis;
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
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setCylinderWarpAxis( wax_obj cylinderWarp, wax_axis axis )
{
	return getPtr(cylinderWarp)->setAxis((Axis) axis);
}

wax_axis wax_cylinderWarpAxis( wax_obj cylinderWarp )
{
	return (wax_axis) getPtr(cylinderWarp)->axis();
}

int wax_setCylinderWarpSpan( wax_obj cylinderWarp, float span )
{
	return getPtr(cylinderWarp)->setSpan(span);
}

float wax_cylinderWarpSpan( wax_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->span();
}

int wax_setCylinderWarpAngles( wax_obj cylinderWarp, float startAngle, float sweep )
{
	return getPtr(cylinderWarp)->setAngles(startAngle, sweep);
}

float wax_cylinderWarpStartAngle( wax_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->startAngle();
}

float wax_cylinderWarpSweep( wax_obj cylinderWarp )
{
	return getPtr(cylinderWarp)->sweep();
}

wax_coordSPX wax_cylinderWarpRotationOffset( wax_obj cylinderWarp, float degrees )
{
	CoordSPX offset = getPtr(cylinderWarp)->rotationOffset(degrees);
	return { offset.x, offset.y };
}
