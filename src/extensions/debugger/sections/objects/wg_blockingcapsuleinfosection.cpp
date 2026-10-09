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
#include "wg_blockingcapsuleinfosection.h"


namespace wg
{

	const TypeInfo BlockingCapsuleInfoSection::TYPEINFO = { "BlockingCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	BlockingCapsuleInfoSection::BlockingCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, BlockingCapsule * pInspected)
		: TypedInfoSection<BlockingCapsule>( theme, pContext, BlockingCapsule::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			boolRow( "Active: ",                      [](BlockingCapsule* c) { return c->isActive(); } ),
			boolRow( "Inverted: ",                    [](BlockingCapsule* c) { return c->isInverted(); } ),
			boolRow( "Auto deactivate: ",             [](BlockingCapsule* c) { return c->isAutoDeactivate(); } ),
			intRow ( "Number of blocked widgets: ",   [](BlockingCapsule* c) { return c->blockedWidgets.size(); } ),
			intRow ( "Number of blocked areas: ",     [](BlockingCapsule* c) { return c->blockedAreas.size(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BlockingCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
