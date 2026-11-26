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
#include "wg_menubuttoninfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo MenuButtonInfoSection::TYPEINFO = { "MenuButtonInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	MenuButtonInfoSection::MenuButtonInfoSection(const DebugTheme& theme, IDebugContext* pContext, MenuButton * pInspected)
		: TypedInfoSection<MenuButton>( theme, pContext, MenuButton::TYPEINFO.className, pInspected )
	{
		//TODO: Close on select and popup overflow (protected members)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Popup: ",         [](MenuButton* p) -> Object* { return p->popup().rawPtr(); } ),
			boolRow  ( "Open: ",          [](MenuButton* p) { return p->isOpen(); } ),
			boolRow  ( "Open on hover: ", [](MenuButton* p) { return p->openOnHover(); } ),
			boolRow  ( "Take focus: ",    [](MenuButton* p) { return p->takeFocus(); } ),
			textRow  ( "Mouse button: ",  [](MenuButton* p) { return toString(p->mouseButton()); } ),
			textRow  ( "Attach point: ",  [](MenuButton* p) { return toString(p->attachPoint()); } )
		});

		m_pLabelDrawer = _createPartDrawer("Label", &pInspected->label);
		m_pIconDrawer = _createPartDrawer("Icon", &pInspected->icon);

		pPanel->slots.pushBack({ pTable, m_pLabelDrawer, m_pIconDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MenuButtonInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void MenuButtonInfoSection::refresh()
	{
		TypedInfoSection<MenuButton>::refresh();

		_refreshPartDrawer(m_pLabelDrawer);
		_refreshPartDrawer(m_pIconDrawer);
	}

} // namespace wg
