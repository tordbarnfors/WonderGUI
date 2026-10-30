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
#include <wg_plugincalls.h>
#include <wg_pluginwarpmap.h>
#include <wg_pluginwarpmapfactory.h>

namespace wg
{
	const TypeInfo PluginWarpmapFactory::TYPEINFO = { "PluginWarpmapFactory", &WarpmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginWarpmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createWarpmap() ____________________________________________________

	static wg_warpmapBP convertBlueprint( const Warpmap::Blueprint& bp )
	{
		wg_warpmapBP cBP;
		cBP.cellSize = bp.cellSize;
		cBP.size = { bp.size.w, bp.size.h };
		cBP.uPeriod = bp.uPeriod;
		cBP.vPeriod = bp.vPeriod;
		return cBP;
	}

	Warpmap_p PluginWarpmapFactory::createWarpmap( const Warpmap::Blueprint& blueprint )
	{
		wg_warpmapBP cBP = convertBlueprint(blueprint);

		auto obj = PluginCalls::warpmapFactory->createWarpmap(m_cFactory, &cBP);
		if (!obj)
			return nullptr;

		auto pWarpmap = PluginWarpmap::create(obj);
		PluginCalls::object->release(obj);
		return pWarpmap;
	}

	Warpmap_p PluginWarpmapFactory::createWarpmap( const Warpmap::Blueprint& blueprint, const CoordSPX * pNodes, int pitch )
	{
		wg_warpmapBP cBP = convertBlueprint(blueprint);

		auto obj = PluginCalls::warpmapFactory->createWarpmapFromNodes(m_cFactory, &cBP, (const wg_coordSPX*) pNodes, pitch);
		if (!obj)
			return nullptr;

		auto pWarpmap = PluginWarpmap::create(obj);
		PluginCalls::object->release(obj);
		return pWarpmap;
	}

} // namespace wg
