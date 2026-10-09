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
#include "wg_popupopenerinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo PopupOpenerInfoSection::TYPEINFO = { "PopupOpenerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PopupOpenerInfoSection::PopupOpenerInfoSection(const DebugTheme& theme, IDebugContext* pContext, PopupOpener * pInspected)
		: TypedInfoSection<PopupOpener>( theme, pContext, PopupOpener::TYPEINFO.className, pInspected )
	{
		//TODO: Close on select and popup overflow (protected members)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Popup: ",         [](PopupOpener* p) -> Object* { return p->popup().rawPtr(); } ),
			boolRow  ( "Open: ",          [](PopupOpener* p) { return p->isOpen(); } ),
			boolRow  ( "Open on hover: ", [](PopupOpener* p) { return p->openOnHover(); } ),
			textRow  ( "Mouse button: ",  [](PopupOpener* p) { return toString(p->mouseButton()); } ),
			textRow  ( "Attach point: ",  [](PopupOpener* p) { return toString(p->attachPoint()); } )
		});

		m_pLabelDrawer = _createComponentDrawer("Label", &pInspected->label);
		m_pIconDrawer = _createComponentDrawer("Icon", &pInspected->icon);

		pPanel->slots.pushBack({ pTable, m_pLabelDrawer, m_pIconDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PopupOpenerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PopupOpenerInfoSection::refresh()
	{
		TypedInfoSection<PopupOpener>::refresh();

		_refreshComponentDrawer(m_pLabelDrawer);
		_refreshComponentDrawer(m_pIconDrawer);
	}

} // namespace wg
