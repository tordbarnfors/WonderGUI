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
#include <wax_c_lenswarp.h>
#include <wax_lenswarp.h>

using namespace wax;

inline LensWarp* getPtr(wax_obj obj) {
	return static_cast<LensWarp*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createLensWarp( const wax_lensWarpBP* pCBP, wax_obj warpmapFactory )
{
	LensWarp::Blueprint bp;

	bp.falloff = pCBP->falloff;
	if (pCBP->magnification != 0.f)
		bp.magnification = pCBP->magnification;
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = LensWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setLensWarpMagnification( wax_obj lensWarp, float magnification )
{
	return getPtr(lensWarp)->setMagnification(magnification);
}

float wax_lensWarpMagnification( wax_obj lensWarp )
{
	return getPtr(lensWarp)->magnification();
}

int wax_setLensWarpFalloff( wax_obj lensWarp, float falloff )
{
	return getPtr(lensWarp)->setFalloff(falloff);
}

float wax_lensWarpFalloff( wax_obj lensWarp )
{
	return getPtr(lensWarp)->falloff();
}

wax_spx wax_lensWarpRadius( wax_obj lensWarp )
{
	return getPtr(lensWarp)->radius();
}
