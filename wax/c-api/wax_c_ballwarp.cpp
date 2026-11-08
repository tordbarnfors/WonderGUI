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
#include <wax_c_ballwarp.h>
#include <wax_ballwarp.h>

using namespace wax;

inline BallWarp* getPtr(wax_obj obj) {
	return static_cast<BallWarp*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createBallWarp( const wax_ballWarpBP* pCBP, wax_obj warpmapFactory )
{
	BallWarp::Blueprint bp;

	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.roll = pCBP->roll;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };
	bp.tilt = pCBP->tilt;

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = BallWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setBallWarpOrientation( wax_obj ballWarp, float tilt, float roll )
{
	return getPtr(ballWarp)->setOrientation(tilt, roll);
}

float wax_ballWarpTilt( wax_obj ballWarp )
{
	return getPtr(ballWarp)->tilt();
}

float wax_ballWarpRoll( wax_obj ballWarp )
{
	return getPtr(ballWarp)->roll();
}

wax_spx wax_ballWarpRadius( wax_obj ballWarp )
{
	return getPtr(ballWarp)->radius();
}

wax_coordSPX wax_ballWarpRotationOffset( wax_obj ballWarp, float degrees )
{
	CoordSPX offset = getPtr(ballWarp)->rotationOffset(degrees);
	return { offset.x, offset.y };
}
