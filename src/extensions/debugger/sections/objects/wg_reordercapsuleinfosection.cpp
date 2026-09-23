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
#include "wg_reordercapsuleinfosection.h"


namespace wg
{

	const TypeInfo ReorderCapsuleInfoSection::TYPEINFO = { "ReorderCapsuleInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ReorderCapsuleInfoSection::ReorderCapsuleInfoSection(const DebugTheme& theme, IDebugContext* pContext, ReorderCapsule * pInspected)
		: TypedInfoSection<ReorderCapsule>( theme, pContext, ReorderCapsule::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			boolRow  ( "Drag outside: ",                 [](ReorderCapsule* c) { return c->dragOutside(); } ),
			objectRow( "Transition: ",                   [](ReorderCapsule* c) -> Object* { return c->transition(); } ),
			intRow   ( "Transition delay (microsec): ",  [](ReorderCapsule* c) { return c->transitionDelay(); } ),
			objectRow( "Transition skin: ",              [](ReorderCapsule* c) -> Object* { return c->transitionSkin(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ReorderCapsuleInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
