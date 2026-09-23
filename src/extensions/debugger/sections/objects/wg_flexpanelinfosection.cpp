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
#include "wg_flexpanelinfosection.h"
#include <wg_flexpanel.h>
#include <wg_packpanel.h>



namespace wg
{

	const TypeInfo FlexPanelInfoSection::TYPEINFO = { "FlexPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	FlexPanelInfoSection::FlexPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, FlexPanel * pInspected)
		: TypedInfoSection<FlexPanel>( theme, pContext, FlexPanel::TYPEINFO.className, pInspected )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow( "Edge policy: ",           [](FlexPanel* p) { return toString(p->edgePolicy()); } ),
			ptsRow ( "Default width (pts): ",   [](FlexPanel* p) { return p->defaultSize().w; } ),
			ptsRow ( "Default height (pts): ",  [](FlexPanel* p) { return p->defaultSize().h; } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void FlexPanelInfoSection::refresh()
	{
		TypedInfoSection<FlexPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
