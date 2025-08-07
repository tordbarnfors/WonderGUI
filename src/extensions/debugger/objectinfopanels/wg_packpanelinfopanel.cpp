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
		m_pInspected = pPanel;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pTable = _createTable(6,2);
		int row = 0;

		_initTextEntry(m_pTable, row++, "Axis: ");
		_initObjectPointerEntry(m_pTable, row++, "Layout: ");
		_initPtsEntry(m_pTable, row++, "Spacing before (pts): ");
		_initPtsEntry(m_pTable, row++, "Spacing between (pts): ");
		_initPtsEntry(m_pTable, row++, "Spacing after (pts): ");
		_initTextEntry(m_pTable, row++, "Slot alignment: ");

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pPanel->slots.begin(), pPanel->slots.end());

		pBasePanel->slots.pushBack({m_pTable, m_pSlotsDrawer});

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PackPanelInfoPanel::refresh()
	{
		int row = 0;
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->axis()));
		_refreshObjectPointerEntry(m_pTable, row++, m_pInspected->layout(),m_displayedLayoutPointer);
		_refreshPtsEntry(m_pTable, row++, m_pInspected->spacingBefore());
		_refreshPtsEntry(m_pTable, row++, m_pInspected->spacingBetween());
		_refreshPtsEntry(m_pTable, row++, m_pInspected->spacingAfter());
		_refreshTextEntry(m_pTable, row++, toString(m_pInspected->slotAlignment()));

		_refreshSlotsDrawer(m_pSlotsDrawer, m_pInspected->slots.begin(), m_pInspected->slots.end());
	}


} // namespace wg


