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
#include "wg_packpanelinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo PackPanelInfoSection::TYPEINFO = { "PackPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PackPanelInfoSection::PackPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, PackPanel * pInspected)
		: TypedInfoSection<PackPanel>( theme, pContext, PackPanel::TYPEINFO.className, pInspected )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "Axis: ",                   [](PackPanel* p) { return toString(p->axis()); } ),
			objectRow( "Layout: ",                 [](PackPanel* p) -> Object* { return p->layout(); } ),
			ptsRow   ( "Spacing before (pts): ",   [](PackPanel* p) { return p->spacingBefore(); } ),
			ptsRow   ( "Spacing between (pts): ",  [](PackPanel* p) { return p->spacingBetween(); } ),
			ptsRow   ( "Spacing after (pts): ",    [](PackPanel* p) { return p->spacingAfter(); } ),
			textRow  ( "Slot alignment: ",         [](PackPanel* p) { return toString(p->slotAlignment()); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PackPanelInfoSection::refresh()
	{
		TypedInfoSection<PackPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
