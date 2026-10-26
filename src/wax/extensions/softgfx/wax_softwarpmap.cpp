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
#include <wg_softwarpmap.h>

namespace wg
{
	const TypeInfo SoftWarpmap::TYPEINFO = { "SoftWarpmap", &Warpmap::TYPEINFO };

	//____ create() ___________________________________________________________

	SoftWarpmap_p SoftWarpmap::create( const Blueprint& bp )
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return SoftWarpmap_p(new SoftWarpmap(bp));
	}

	SoftWarpmap_p SoftWarpmap::create( const Blueprint& bp, const CoordF * pNodes, int pitch )
	{
		if (!_validateBlueprint(bp) || !pNodes)
			return nullptr;

		auto p = SoftWarpmap_p(new SoftWarpmap(bp));
		SizeI nodes = p->nodes();
		p->importNodes({ 0,0,nodes.w,nodes.h }, pNodes, pitch);
		return p;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
