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

#include <wg_metalwarpmapfactory.h>
#include <wg_metalwarpmap.h>

namespace wg
{
	const TypeInfo MetalWarpmapFactory::TYPEINFO = { "MetalWarpmapFactory", &WarpmapFactory::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MetalWarpmapFactory::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ createWarpmap() ____________________________________________________

	Warpmap_p MetalWarpmapFactory::createWarpmap( const Warpmap::Blueprint& blueprint )
	{
		return MetalWarpmap::create(blueprint);
	}

	Warpmap_p MetalWarpmapFactory::createWarpmap( const Warpmap::Blueprint& blueprint, const CoordF * pNodes, int pitch )
	{
		return MetalWarpmap::create(blueprint, pNodes, pitch);
	}

} // namespace wg
