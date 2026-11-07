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
#include <wax_c_perspectivewarp.h>
#include <wax_perspectivewarp.h>

using namespace wax;

inline PerspectiveWarp* getPtr(wax_obj obj) {
	return static_cast<PerspectiveWarp*>(reinterpret_cast<Object*>(obj));
}

static CornersT<CoordSPX> toCorners( const wax_coordSPX corners[4] )
{
	CornersT<CoordSPX> c;
	for (int i = 0; i < 4; i++)
		c[i] = { corners[i].x, corners[i].y };
	return c;
}

static void fromCorners( const CornersT<CoordSPX>& c, wax_coordSPX corners[4] )
{
	for (int i = 0; i < 4; i++)
		corners[i] = { c[i].x, c[i].y };
}

wax_obj wax_createPerspectiveWarp( const wax_perspectiveWarpBP* pCBP, wax_obj warpmapFactory )
{
	PerspectiveWarp::Blueprint bp;

	bp.cellSize = { pCBP->cellSize.w, pCBP->cellSize.h };
	bp.corners = toCorners(pCBP->corners);
	if (pCBP->precision > 0.f)
		bp.precision = pCBP->precision;
	bp.size = { pCBP->size.w, pCBP->size.h };
	bp.sourceSize = { pCBP->sourceSize.w, pCBP->sourceSize.h };

	auto pFactory = static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(warpmapFactory));

	auto p = PerspectiveWarp::create(bp, pFactory);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

int wax_setPerspectiveWarpCorners( wax_obj perspectiveWarp, const wax_coordSPX corners[4] )
{
	return getPtr(perspectiveWarp)->setCorners(toCorners(corners));
}

void wax_perspectiveWarpCorners( wax_obj perspectiveWarp, wax_coordSPX corners[4] )
{
	fromCorners(getPtr(perspectiveWarp)->corners(), corners);
}

void wax_perspectiveWarpRotated( const wax_rectSPX* pRect, float tilt, float turn, float roll, wax_spx viewDistance,
								 wax_coordF pivot, const wax_coordSPX* pEye, wax_coordSPX corners[4] )
{
	std::optional<CoordSPX> eye;
	if (pEye)
		eye = CoordSPX(pEye->x, pEye->y);

	auto c = PerspectiveWarp::rotated({ pRect->x, pRect->y, pRect->w, pRect->h }, tilt, turn, roll, viewDistance, { pivot.x, pivot.y }, eye);
	fromCorners(c, corners);
}
