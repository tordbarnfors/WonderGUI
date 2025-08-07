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

	FlexPanelInfoPanel::FlexPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, FlexPanel * pPanel) : DebugPanel( blueprint, pHolder, FlexPanel::TYPEINFO.className )
	{
		m_pInspected = pPanel;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pTable = _createTable(3,2);

		_setTextEntry(m_pTable, 0, "Edge policy: ", toString(pPanel->edgePolicy()));
		_setPtsEntry(m_pTable, 1, "Default width (pts): ", pPanel->defaultSize().w);
		_setPtsEntry(m_pTable, 2, "Default height (pts): ", pPanel->defaultSize().h);

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pPanel->slots.begin(), pPanel->slots.end());

		pBasePanel->slots.pushBack({m_pTable,m_pSlotsDrawer});

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void FlexPanelInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->edgePolicy()));
		_refreshPtsEntry(m_pTable, 1, m_pInspected->defaultSize().w);
		_refreshPtsEntry(m_pTable, 2, m_pInspected->defaultSize().h);

		_refreshSlotsDrawer(m_pSlotsDrawer, m_pInspected->slots.begin(), m_pInspected->slots.end());
	}

} // namespace wg


