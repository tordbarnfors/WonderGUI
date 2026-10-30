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
#include <wax_c_warpmapfactory.h>
#include <wax_c_internal.h>
#include <wax_warpmapfactory.h>

using namespace wax;

inline WarpmapFactory* getPtr(wax_obj obj) {
	return static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(obj));
}

wax_obj wax_createWarpmap(wax_obj factory, const wax_warpmapBP* pCBP)
{
	Warpmap::Blueprint	cppBP;
	convertWarpmapBlueprint(&cppBP, pCBP);

	auto p = getPtr(factory)->createWarpmap(cppBP);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}

wax_obj wax_createWarpmapFromNodes(wax_obj factory, const wax_warpmapBP* pCBP, const wax_coordSPX* pNodes, int pitch)
{
	Warpmap::Blueprint	cppBP;
	convertWarpmapBlueprint(&cppBP, pCBP);

	auto p = getPtr(factory)->createWarpmap(cppBP, (const CoordSPX*) pNodes, pitch);
	if (!p)
		return nullptr;

	p->retain();
	return (wax_obj) static_cast<Object*>(p.rawPtr());
}
