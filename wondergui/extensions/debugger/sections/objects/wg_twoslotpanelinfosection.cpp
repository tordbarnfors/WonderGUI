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
#include "wg_twoslotpanelinfosection.h"
#include <wg_twoslotpanel.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo TwoSlotPanelInfoSection::TYPEINFO = { "TwoSlotPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TwoSlotPanelInfoSection::TwoSlotPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, TwoSlotPanel * pInspected)
		: TypedInfoSection<TwoSlotPanel>( theme, pContext, TwoSlotPanel::TYPEINFO.className, pInspected )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "Axis: ",   [](TwoSlotPanel* p) { return toString(p->axis()); } ),
			objectRow( "Layout: ", [](TwoSlotPanel* p) -> Object* { return p->layout(); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TwoSlotPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TwoSlotPanelInfoSection::refresh()
	{
		TypedInfoSection<TwoSlotPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
