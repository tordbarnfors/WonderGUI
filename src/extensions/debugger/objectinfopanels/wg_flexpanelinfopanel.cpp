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
#include "wg_flexpanelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_flexpanel.h>
#include <wg_packpanel.h>



namespace wg
{

	const TypeInfo FlexPanelInfoPanel::TYPEINFO = { "FlexPanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	FlexPanelInfoPanel::FlexPanelInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, FlexPanel * pPanel) : DebugPanel( blueprint, pHolder )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createTable(3,2);
		int row = 0;

		_setTextEntry(pTable, row++, "Edge policy: ", toString(pPanel->edgePolicy()));
		_setPtsEntry(pTable, row++, "Default width (pts): ", pPanel->defaultSize().w);
		_setPtsEntry(pTable, row++, "Default height (pts): ", pPanel->defaultSize().h);

		pBasePanel->slots << pTable;

		pBasePanel->slots << _createSlotsDrawer("Slots", pPanel->slots.begin(), pPanel->slots.end());

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


