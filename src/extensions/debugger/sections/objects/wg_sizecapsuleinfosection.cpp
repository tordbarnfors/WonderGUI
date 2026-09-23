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
#include "wg_sizecapsuleinfosection.h"


namespace wg
{

	const TypeInfo SizeCapsuleInfoSection::TYPEINFO = { "SizeCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SizeCapsuleInfoSection::SizeCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, SizeCapsule * pInspected)
		: TypedInfoSection<SizeCapsule>( theme, pContext, SizeCapsule::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			ptsRow( "Default width: ",  [](SizeCapsule* c) { return c->defaultSize().w; } ),
			ptsRow( "Default height: ", [](SizeCapsule* c) { return c->defaultSize().h; } ),
			ptsRow( "Min width: ",      [](SizeCapsule* c) { return c->minSize().w; } ),
			ptsRow( "Min height: ",     [](SizeCapsule* c) { return c->minSize().h; } ),
			ptsRow( "Max width: ",      [](SizeCapsule* c) { return c->maxSize().w; } ),
			ptsRow( "Max height: ",     [](SizeCapsule* c) { return c->maxSize().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SizeCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
