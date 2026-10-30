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
#include <wg_c_roundedrectwarp.h>
#include <wg_roundedrectwarp.h>

using namespace wg;

inline RoundedRectWarp* getPtr(wg_obj obj) {
	return static_cast<RoundedRectWarp*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createRoundedRectWarp( const wg_roundedRectWarpBP* pCBP, wg_obj warpmapFactory )
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
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

int wg_setRoundedRectWarpOutline( wg_obj roundedRectWarp, const wg_rectSPX* rect, wg_spx cornerRadius )
{
	return getPtr(roundedRectWarp)->setOutline({ rect->x, rect->y, rect->w, rect->h }, cornerRadius);
}

wg_rectSPX wg_roundedRectWarpRect( wg_obj roundedRectWarp )
{
	RectSPX r = getPtr(roundedRectWarp)->rect();
	return { r.x, r.y, r.w, r.h };
}

wg_spx wg_roundedRectWarpCornerRadius( wg_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->cornerRadius();
}

int wg_setRoundedRectWarpDistances( wg_obj roundedRectWarp, wg_spx topDistance, wg_spx bottomDistance )
{
	return getPtr(roundedRectWarp)->setDistances(topDistance, bottomDistance);
}

wg_spx wg_roundedRectWarpTopDistance( wg_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->topDistance();
}

wg_spx wg_roundedRectWarpBottomDistance( wg_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->bottomDistance();
}

int wg_setRoundedRectWarpSpan( wg_obj roundedRectWarp, float start, float sweep )
{
	return getPtr(roundedRectWarp)->setSpan(start, sweep);
}

float wg_roundedRectWarpStart( wg_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->start();
}

float wg_roundedRectWarpSweep( wg_obj roundedRectWarp )
{
	return getPtr(roundedRectWarp)->sweep();
}
