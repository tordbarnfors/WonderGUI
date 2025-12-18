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
#include "wg_twoslotpanelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_twoslotpanel.h>

namespace wg
{

	const TypeInfo TwoSlotPanelSlotInfoPanel::TYPEINFO = { "TwoSlotPanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	TwoSlotPanelSlotInfoPanel::TwoSlotPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, TwoSlotPanel::Slot::TYPEINFO.className )
	{
		auto pInspected = static_cast<TwoSlotPanel::Slot*>(pStaticSlot);

		m_pTable = _createTable(2,2);
		_setDecimalEntry(m_pTable, 0, "Weight: ", pInspected->weight());
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TwoSlotPanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TwoSlotPanelSlotInfoPanel::refresh(StaticSlot * pStaticSlot)
	{
		auto pInspected = static_cast<TwoSlotPanel::Slot*>(pStaticSlot);

		_refreshDecimalEntry(m_pTable, 0, pInspected->weight());
	}

} // namespace wg



