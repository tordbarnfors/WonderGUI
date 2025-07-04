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
#include "wg_panelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_panel.h>

namespace wg
{

	const TypeInfo PanelSlotInfoPanel::TYPEINFO = { "PanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelSlotInfoPanel::PanelSlotInfoPanel(const Blueprint& blueprint, StaticSlot * pStaticSlot) : DebugPanel( blueprint )
	{
		auto pPanelSlot = static_cast<const PanelSlot*>(pStaticSlot);


		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = 1 ));

		pTable->slots[0][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "Visible: " ));
		pTable->slots[0][1] = NumberDisplay::create( WGOVR(blueprint.listEntryInteger, _.display.value = pPanelSlot->isVisible() ));

		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg



