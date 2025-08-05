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
#include "wg_twoslotpanelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_twoslotpanel.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo TwoSlotPanelInfoPanel::TYPEINFO = { "TwoSlotPanelPanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	TwoSlotPanelInfoPanel::TwoSlotPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, TwoSlotPanel * pPanel) : DebugPanel( blueprint, pHolder, TwoSlotPanel::TYPEINFO.className )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createTable(2,2);
		int row = 0;

		_setTextEntry(pTable, row++, "Axis: ", toString(pPanel->axis()));
		_setObjectPointerEntry(pTable, row++, "Layout: ", pPanel->layout(),this);

		pBasePanel->slots << pTable;

		pBasePanel->slots << _createSlotsDrawer("Slots", pPanel->slots.begin(), pPanel->slots.end());

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TwoSlotPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


