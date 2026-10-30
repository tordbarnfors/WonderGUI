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
#include <wax_c_roundedrectwarp.h>
#include <wax_roundedrectwarp.h>

using namespace wax;

inline RoundedRectWarp* getPtr(wax_obj obj) {
	return static_cast<RoundedRectWarp*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createRoundedRectWarp( const wax_roundedRectWarpBP* pCBP, wax_obj warpmapFactory )
{
	RoundedRectWarp::Blueprint bp;

	bp.bottomDistance = pCBP->bottomDistance;
	bp.cellSize = pCBP->cellSize;
	bp.cornerRadius = pCBP->cornerRadius;
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.rect = { pCBP->rect.x, pCBP->rect.y, pCBP->rect.w, pCBP->rect.h };
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };
	bp.start = pCBP->start;
	if (pCBP->sweep != 0.f)
		bp.sweep = pCBP->sweep;
	bp.topDistance = pCBP->topDistance;

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = RoundedRectWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setRoundedRectWarpOutline( wax_obj roundedRectWarp, const wax_rectSPX* rect, wax_spx cornerRadius )
{
	return getPtr(roundedRectWarp)->setOutline({ rect->x, rect->y, rect->w, rect->h }, cornerRadius);
}

wax_rectSPX wax_roundedRectWarpRect( wax_obj roundedRectWarp )
{
	RectSPX r = getPtr(roundedRectWarp)->rect();
	return { r.x, r.y, r.w, r.h };
}

wax_spx wax_roundedRectWarpCornerRadius( wax_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->cornerRadius();
}

int wax_setRoundedRectWarpDistances( wax_obj roundedRectWarp, wax_spx topDistance, wax_spx bottomDistance )
{
	return getPtr(roundedRectWarp)->setDistances(topDistance, bottomDistance);
}

wax_spx wax_roundedRectWarpTopDistance( wax_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->topDistance();
}

wax_spx wax_roundedRectWarpBottomDistance( wax_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->bottomDistance();
}

int wax_setRoundedRectWarpSpan( wax_obj roundedRectWarp, float start, float sweep )
{
	return getPtr(roundedRectWarp)->setSpan(start, sweep);
}

float wax_roundedRectWarpStart( wax_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->start();
}

float wax_roundedRectWarpSweep( wax_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->sweep();
}
