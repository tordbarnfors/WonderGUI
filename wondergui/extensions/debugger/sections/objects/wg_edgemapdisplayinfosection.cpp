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
#include "wg_edgemapdisplayinfosection.h"


namespace wg
{

	const TypeInfo EdgemapDisplayInfoSection::TYPEINFO = { "EdgemapDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	EdgemapDisplayInfoSection::EdgemapDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, EdgemapDisplay * pInspected)
		: TypedInfoSection<EdgemapDisplay>( theme, pContext, EdgemapDisplay::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			objectRow( "Edgemap: ",           [](EdgemapDisplay* e) -> Object* { return e->edgemap().rawPtr(); } ),
			textRow  ( "Edgemap placement: ", [](EdgemapDisplay* e) { return toString(e->edgemapPlacement()); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& EdgemapDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
