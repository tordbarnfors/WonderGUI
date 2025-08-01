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

	PanelSlotInfoPanel::PanelSlotInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder )
	{
		auto pPanelSlot = static_cast<const PanelSlot*>(pStaticSlot);

		auto pTable = _createTable(1,2);
		_setBoolEntry(pTable, 0, "Visible: ", pPanelSlot->isVisible());
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg



