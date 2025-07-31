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
#include "wg_panelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo PanelInfoPanel::TYPEINFO = { "PanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelInfoPanel::PanelInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Panel * pPanel) : DebugPanel( blueprint, pHolder )
	{
		auto pTable = _createTable(1,2);
		_setTextEntry(pTable, 0, "Mask op: ", toString(pPanel->maskOp()));
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


