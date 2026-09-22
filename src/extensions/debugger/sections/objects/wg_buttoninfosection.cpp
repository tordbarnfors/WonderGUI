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
#include "wg_buttoninfosection.h"
#include <wg_button.h>


namespace wg
{

	const TypeInfo ButtonInfoSection::TYPEINFO = { "ButtonInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ButtonInfoSection::ButtonInfoSection(const DebugTheme& theme, IDebugContext* pContext, Button * pInspected)
		: TypedInfoSection<Button>( theme, pContext, Button::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			boolRow( "Select on press: ", [](Button* b) { return b->selectOnPress(); } )
		});

		m_pLabelDrawer = _createComponentDrawer("Label", &pInspected->label);
		m_pIconDrawer = _createComponentDrawer("Icon", &pInspected->icon);

		pPanel->slots.pushBack({ pTable, m_pLabelDrawer, m_pIconDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ButtonInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ButtonInfoSection::refresh()
	{
		TypedInfoSection<Button>::refresh();

		_refreshComponentDrawer(m_pLabelDrawer);
		_refreshComponentDrawer(m_pIconDrawer);
	}

} // namespace wg
