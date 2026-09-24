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

#include <wg_c_tint.h>
#include <wg_tint.h>

using namespace wg;

inline Tint* getPtr(wg_obj obj) {
	return static_cast<Tint*>(reinterpret_cast<Object*>(obj));
}

wg_tintBP wg_defaultTintBP()
{
	wg_tintBP bp;

	bp.begin = { 0.f, 0.f };
	bp.center = { 0.5f, 0.5f };
	bp.colorSpace = WG_COLORSPACE_LINEAR;
	bp.end = { 0.f, 1.f };
	bp.radius = { 0.5f, 0.5f };
	bp.radiusMode = WG_TINTRADIUS_FIT;
	bp.shape = WG_TINTSHAPE_LINEAR;
	bp.spread = WG_TINTSPREAD_PAD;
	bp.nbStops = 0;
	bp.stops = nullptr;
	return bp;
}

wg_obj wg_createTint( const wg_tintBP* pBP )
{
	Tint::Blueprint bp;

	bp.begin = { pBP->begin.x, pBP->begin.y };
	bp.center = { pBP->center.x, pBP->center.y };
	bp.colorSpace = (ColorSpace) pBP->colorSpace;
	bp.end = { pBP->end.x, pBP->end.y };
	bp.radius = { pBP->radius.w, pBP->radius.h };
	bp.radiusMode = (TintRadius) pBP->radiusMode;
	bp.shape = (TintShape) pBP->shape;
	bp.spread = (TintSpread) pBP->spread;

	for( int i = 0 ; i < pBP->nbStops ; i++ )
		bp.stops.push_back( { pBP->stops[i].pos, * reinterpret_cast<const HiColor*>(&pBP->stops[i].color) } );

	auto pTint = Tint::create(bp);
	if( !pTint )
		return nullptr;

	pTint->retain();
	return static_cast<Object*>(pTint.rawPtr());
}

wg_obj wg_blendTints( wg_obj fromTint, wg_obj toTint, float progress )
{
	auto pTint = Tint::blend( getPtr(fromTint), getPtr(toTint), progress );
	if( !pTint )
		return nullptr;

	pTint->retain();
	return static_cast<Object*>(pTint.rawPtr());
}

int wg_isTintOpaque( wg_obj tint )
{
	return getPtr(tint)->isOpaque();
}

int wg_isTintFlat( wg_obj tint )
{
	return getPtr(tint)->isFlat();
}

int wg_isTintMix( wg_obj tint )
{
	return getPtr(tint)->isMix();
}

wg_color wg_tintColorAt( wg_obj tint, wg_coordSPX pos, const wg_rectSPX* pRect )
{
	HiColor col = getPtr(tint)->colorAt( { pos.x, pos.y }, * reinterpret_cast<const RectSPX*>(pRect) );
	return * reinterpret_cast<wg_color*>(&col);
}
