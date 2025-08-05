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
#include "wg_packpanelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo PackPanelInfoPanel::TYPEINFO = { "PackPanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PackPanelInfoPanel::PackPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, PackPanel * pPanel) : DebugPanel( blueprint, pHolder, PackPanel::TYPEINFO.className )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createTable(6,2);
		int row = 0;

		_setTextEntry(pTable, row++, "Axis: ", toString(pPanel->axis()));
		_setObjectPointerEntry(pTable, row++, "Layout: ", pPanel->layout(),this);
		_setPtsEntry(pTable, row++, "Spacing before (pts): ", pPanel->spacingBefore());
		_setPtsEntry(pTable, row++, "Spacing between (pts): ", pPanel->spacingBetween());
		_setPtsEntry(pTable, row++, "Spacing after (pts): ", pPanel->spacingAfter());
		_setTextEntry(pTable, row++, "Slot alignment: ", toString(pPanel->slotAlignment()));

		pBasePanel->slots << pTable;

		pBasePanel->slots << _createSlotsDrawer("Slots", pPanel->slots.begin(), pPanel->slots.end());

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


