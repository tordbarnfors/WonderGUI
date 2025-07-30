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
#include "wg_containerinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ContainerInfoPanel::TYPEINFO = { "ContainerInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ContainerInfoPanel::ContainerInfoPanel(const Blueprint& blueprint, Container * pContainer) : DebugPanel( blueprint )
	{
		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = 5 ));

		pTable->slots[0][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "Use pick handles: " ));
		pTable->slots[0][1] = TextDisplay::create( WGOVR( blueprint.listEntryText, _.display.text = pContainer->usePickHandles() ? "true" : "false"));

		BorderSPX overflow = pContainer->_overflow();

		pTable->slots[1][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow top (spx): "));
		pTable->slots[1][1] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.top ));

		pTable->slots[2][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow right (spx): "));
		pTable->slots[2][1] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.right));

		pTable->slots[3][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow bottom (spx): "));
		pTable->slots[3][1] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.bottom));

		pTable->slots[4][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow left (spx): "));
		pTable->slots[4][1] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.left));

		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ContainerInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


