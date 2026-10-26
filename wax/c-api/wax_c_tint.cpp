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

#include <wax_c_tint.h>
#include <wax_tint.h>
#include <wax_tinttools.h>

#include <cstring>
#include <cstddef>

using namespace wax;

inline Tint* getPtr(wax_obj obj) {
	return static_cast<Tint*>(reinterpret_cast<Object*>(obj));
}

// wax_getTintBlueprint() hands out the tint's own stops.

static_assert( sizeof(wax_colorStop) == sizeof(ColorStop) && offsetof(wax_colorStop, color) == offsetof(ColorStop, color),
			   "wax_colorStop must be binary equivalent to ColorStop." );
static_assert( sizeof(wax_color) == sizeof(HiColor), "wax_color must be binary equivalent to HiColor." );

wax_tintBP wax_defaultTintBP()
{
	wax_tintBP bp;

	bp.begin = { 0.f, 0.f };
	bp.center = { 0.5f, 0.5f };
	bp.end = { 0.f, 1.f };
	bp.radius = { 0.5f, 0.5f };
	bp.radiusMode = WAX_TINTRADIUS_FIT;
	bp.shape = WAX_TINTSHAPE_LINEAR;
	bp.spread = WAX_TINTSPREAD_PAD;
	bp.nbStops = 0;
	bp.stops = nullptr;
	return bp;
}

wax_obj wax_createTint( const wax_tintBP* pBP )
{
	Tint::Blueprint bp;

	bp.begin = { pBP->begin.x, pBP->begin.y };
	bp.center = { pBP->center.x, pBP->center.y };
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

wax_obj wax_createTintMix( int nbComponents, const wax_obj* pComponents, const float* pWeights )
{
	if( nbComponents < 1 || nbComponents > Tint::c_maxMixComponents || !pComponents || !pWeights )
		return nullptr;

	Tint* components[Tint::c_maxMixComponents];
	for( int i = 0 ; i < nbComponents ; i++ )
		components[i] = getPtr(pComponents[i]);

	auto pTint = Tint::createMix( nbComponents, components, pWeights );
	if( !pTint )
		return nullptr;

	pTint->retain();
	return static_cast<Object*>(pTint.rawPtr());
}

wax_obj wax_mixTints( wax_obj fromTint, wax_obj toTint, float progress )
{
	auto pTint = Tint::mix( getPtr(fromTint), getPtr(toTint), progress );
	if( !pTint )
		return nullptr;

	pTint->retain();
	return static_cast<Object*>(pTint.rawPtr());
}

int wax_isTintOpaque( wax_obj tint )
{
	return getPtr(tint)->isOpaque();
}

int wax_isTintFlat( wax_obj tint )
{
	return getPtr(tint)->isFlat();
}

int wax_isTintMix( wax_obj tint )
{
	return getPtr(tint)->isMix();
}

wax_tintBP wax_getTintBlueprint( wax_obj tint )
{
	auto pTint = getPtr(tint);

	wax_tintBP bp;

	bp.begin = { pTint->begin().x, pTint->begin().y };
	bp.center = { pTint->center().x, pTint->center().y };
	bp.end = { pTint->end().x, pTint->end().y };
	bp.radius = { pTint->radius().w, pTint->radius().h };
	bp.radiusMode = (wax_tintRadius) pTint->radiusMode();
	bp.shape = (wax_tintShape) pTint->shape();
	bp.spread = (wax_tintSpread) pTint->spread();
	bp.nbStops = pTint->isMix() ? 0 : pTint->nbStops();
	bp.stops = pTint->isMix() ? nullptr : reinterpret_cast<const wax_colorStop*>(pTint->stops());
	return bp;
}

int wax_tintMixComponents( wax_obj tint )
{
	return getPtr(tint)->nbMixComponents();
}

wax_obj wax_tintMixComponent( wax_obj tint, int index )
{
	auto pTint = getPtr(tint);
	if( index < 0 || index >= pTint->nbMixComponents() )
		return nullptr;

	return static_cast<Object*>(pTint->mixComponent(index));
}

float wax_tintMixWeight( wax_obj tint, int index )
{
	auto pTint = getPtr(tint);
	if( index < 0 || index >= pTint->nbMixComponents() )
		return 0.f;

	return pTint->mixWeight(index);
}

wax_color wax_tintColorAt( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect )
{
	HiColor col = getPtr(tint)->colorAt( { pos.x, pos.y }, * reinterpret_cast<const RectSPX*>(pRect) );
	return * reinterpret_cast<wax_color*>(&col);
}

int wax_tintAlphaAt( wax_obj tint, wax_coordSPX pos, const wax_rectSPX* pRect )
{
	return getPtr(tint)->alpha( { pos.x, pos.y }, * reinterpret_cast<const RectSPX*>(pRect) );
}

int wax_exportTintData( wax_obj tint, void* pDest, int maxBytes )
{
	static_assert( WAX_MAX_TINT_DATA_BYTES == TintTools::c_maxSerializedTintBytes, "WAX_MAX_TINT_DATA_BYTES out of sync with TintTools." );

	uint8_t	buffer[TintTools::c_maxSerializedTintBytes];

	int bytes = TintTools::serializeTint( tint ? getPtr(tint) : nullptr, buffer );
	if( bytes > maxBytes )
		return 0;

	memcpy( pDest, buffer, bytes );
	return bytes;
}

wax_obj wax_createTintFromData( const void* pData, int bytes )
{
	if( !pData || bytes <= 0 )
		return nullptr;

	// The deserializer only learns how much it needs as it reads, so data shorter
	// than the largest tint is copied into a buffer that is, rather than read past.

	uint8_t	buffer[TintTools::c_maxSerializedTintBytes] = {};

	if( bytes < TintTools::c_maxSerializedTintBytes )
	{
		memcpy( buffer, pData, bytes );
		pData = buffer;
	}

	int bytesRead = 0;
	auto pTint = TintTools::deserializeTint( (const uint8_t*) pData, bytesRead );
	if( !pTint || bytesRead > bytes )
		return nullptr;

	pTint->retain();
	return static_cast<Object*>(pTint.rawPtr());
}
