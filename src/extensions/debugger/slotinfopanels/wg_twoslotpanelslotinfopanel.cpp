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
#include "wg_twoslotpanelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_twoslotpanel.h>

namespace wg
{

	const TypeInfo TwoSlotPanelSlotInfoPanel::TYPEINFO = { "TwoSlotPanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	TwoSlotPanelSlotInfoPanel::TwoSlotPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, TwoSlotPanel::Slot::TYPEINFO.className )
	{
		m_pInspected = static_cast<TwoSlotPanel::Slot*>(pStaticSlot);

		m_pTable = _createTable(2,2);
		_setDecimalEntry(m_pTable, 0, "Weight: ", m_pInspected->weight());
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TwoSlotPanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TwoSlotPanelSlotInfoPanel::refresh()
	{
		_refreshDecimalEntry(m_pTable, 0, m_pInspected->weight());
	}

} // namespace wg



