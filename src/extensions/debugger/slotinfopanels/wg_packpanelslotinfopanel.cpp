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
#include "wg_packpanelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo PackPanelSlotInfoPanel::TYPEINFO = { "PackPanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PackPanelSlotInfoPanel::PackPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, PackPanelSlot::TYPEINFO.className )
	{
		auto pPanelSlot = static_cast<const PackPanelSlot*>(pStaticSlot);

		auto pTable = _createTable(2,2);
		_setDecimalEntry(pTable, 0, "Weight: ", pPanelSlot->weight());
		_setDecimalEntry(pTable, 1, "Baseline: ", pPanelSlot->baseline());
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg



