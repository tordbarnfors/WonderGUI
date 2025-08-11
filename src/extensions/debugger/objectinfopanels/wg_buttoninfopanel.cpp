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
#include "wg_buttoninfopanel.h"
#include <wg_button.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo ButtonInfoPanel::TYPEINFO = { "ButtonInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ButtonInfoPanel::ButtonInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Button * pButton) : DebugPanel( blueprint, pHolder, Button::TYPEINFO.className )
	{
		m_pInspected = pButton;

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pTable = _createTable(1, 2);

		_setBoolEntry(m_pTable, 0, "Select on press: ", m_pInspected->selectOnPress() );

		m_pLabelDrawer = _createComponentDrawer("Label", &pButton->label);
		m_pIconDrawer = _createComponentDrawer("Icon", &pButton->icon);

		pPanel->slots.pushBack({ m_pTable, m_pLabelDrawer, m_pIconDrawer });

		slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ButtonInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ButtonInfoPanel::refresh()
	{
		_refreshBoolEntry(m_pTable, 0, m_pInspected->selectOnPress());

		_refreshComponentDrawer( static_cast<DrawerPanel*>(m_pLabelDrawer) );
		_refreshComponentDrawer(static_cast<DrawerPanel*>(m_pIconDrawer) );
	}


} // namespace wg


