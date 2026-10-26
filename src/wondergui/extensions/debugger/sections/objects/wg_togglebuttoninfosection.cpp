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
#include "wg_togglebuttoninfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	//____ toString() _____________________________________________________________

	static const char * toString(ToggleButton::ClickArea clickArea)
	{
		switch (clickArea)
		{
			case ToggleButton::ClickArea::Default:
				return "Default";
			case ToggleButton::ClickArea::Alpha:
				return "Alpha";
			case ToggleButton::ClickArea::Geo:
				return "Geo";
			case ToggleButton::ClickArea::Icon:
				return "Icon";
			case ToggleButton::ClickArea::Text:
				return "Text";
		}
		return "Unknown";
	}

	const TypeInfo ToggleButtonInfoSection::TYPEINFO = { "ToggleButtonInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ToggleButtonInfoSection::ToggleButtonInfoSection(const DebugTheme& theme, IDebugContext* pContext, ToggleButton * pInspected)
		: TypedInfoSection<ToggleButton>( theme, pContext, ToggleButton::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "Click area: ",      [](ToggleButton* b) { return toString(b->clickArea()); } ),
			boolRow  ( "Flip on release: ", [](ToggleButton* b) { return b->flipOnRelease(); } ),
			objectRow( "Toggle group: ",    [](ToggleButton* b) -> Object* { return b->toggleGroup().rawPtr(); } )
		});

		m_pLabelDrawer = _createComponentDrawer("Label", &pInspected->label);
		m_pIconDrawer = _createComponentDrawer("Icon", &pInspected->icon);

		pPanel->slots.pushBack({ pTable, m_pLabelDrawer, m_pIconDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ToggleButtonInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ToggleButtonInfoSection::refresh()
	{
		TypedInfoSection<ToggleButton>::refresh();

		_refreshComponentDrawer(m_pLabelDrawer);
		_refreshComponentDrawer(m_pIconDrawer);
	}

} // namespace wg
