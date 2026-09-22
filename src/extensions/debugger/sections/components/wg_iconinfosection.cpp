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
#include "wg_iconinfosection.h"

namespace wg
{

	const TypeInfo IconInfoSection::TYPEINFO = { "IconInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	IconInfoSection::IconInfoSection(const DebugTheme& theme, IDebugContext* pContext, Icon * pInspected)
		: TypedInfoSection<Icon>( theme, pContext, Icon::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow  ( "Placement: ",      [](Icon* i) { return toString(i->placement()); } ),
			ptsRow   ( "Spacing (pts): ",  [](Icon* i) { return i->spacing(); } ),
			objectRow( "Skin: ",           [](Icon* i) -> Object* { return i->skin(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& IconInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
