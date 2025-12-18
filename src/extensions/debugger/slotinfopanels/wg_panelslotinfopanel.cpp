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
#include "wg_panelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_panel.h>

namespace wg
{

	const TypeInfo PanelSlotInfoPanel::TYPEINFO = { "PanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelSlotInfoPanel::PanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, PanelSlot::TYPEINFO.className )
	{
		m_pTable = _createTable(1,2);
		_initBoolEntry(m_pTable, 0, "Visible: ");

		refresh(pStaticSlot);

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PanelSlotInfoPanel::refresh(StaticSlot * pStaticSlot)
	{
		auto pInspected = static_cast<PanelSlot*>(pStaticSlot);
		_refreshBoolEntry(m_pTable, 0, pInspected->isVisible());
	}


} // namespace wg



