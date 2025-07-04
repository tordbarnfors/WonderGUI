/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#include "wg_objectinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ObjectInfoPanel::TYPEINFO = { "ObjectInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ObjectInfoPanel::ObjectInfoPanel(const Blueprint& blueprint, Object * pObject) : DebugPanel( blueprint )
	{
		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = 3 ));

		pTable->slots[0][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "Refcount: " ));
		pTable->slots[0][1] = NumberDisplay::create( WGOVR( blueprint.listEntryInteger, _.display.value = pObject->refcount() ));

		// Weak pointers is -1 since our DebugOverlay has one weak pointer to object that doesn't count.

		pTable->slots[1][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Weak pointers: "));
		pTable->slots[1][1] = NumberDisplay::create(WGOVR(blueprint.listEntryInteger, _.display.value = pObject->weakPointers() -1));

		pTable->slots[2][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Finalizer: "));
		pTable->slots[2][1] = NumberDisplay::create( WGOVR( blueprint.listEntryPointer, _.display.value = pObject->finalizer()));

		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ObjectInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


