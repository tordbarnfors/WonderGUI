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
#include <wg_c_warpmapfactory.h>
#include <wg_c_internal.h>
#include <wg_warpmapfactory.h>

using namespace wg;

inline WarpmapFactory* getPtr(wg_obj obj) {
	return static_cast<WarpmapFactory*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createWarpmap(wg_obj factory, const wg_warpmapBP* pCBP)
{
	Warpmap::Blueprint	cppBP;
	convertWarpmapBlueprint(&cppBP, pCBP);

	auto p = getPtr(factory)->createWarpmap(cppBP);
	if (!p)
		return nullptr;

	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}

wg_obj wg_createWarpmapFromNodes(wg_obj factory, const wg_warpmapBP* pCBP, const wg_coordF* pNodes, int pitch)
{
	Warpmap::Blueprint	cppBP;
	convertWarpmapBlueprint(&cppBP, pCBP);

	auto p = getPtr(factory)->createWarpmap(cppBP, (const CoordF*) pNodes, pitch);
	if (!p)
		return nullptr;

	p->retain();
	return (wg_obj) static_cast<Object*>(p.rawPtr());
}
