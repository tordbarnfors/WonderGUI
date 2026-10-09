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
#include "wg_splitpanelinfosection.h"
#include <wg_splitpanel.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo SplitPanelInfoSection::TYPEINFO = { "SplitPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SplitPanelInfoSection::SplitPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, SplitPanel * pInspected)
		: TypedInfoSection<SplitPanel>( theme, pContext, SplitPanel::TYPEINFO.className, pInspected )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow   ( "Axis: ",                     [](SplitPanel* p) { return toString(p->axis()); } ),
			objectRow ( "Handle skin: ",              [](SplitPanel* p) -> Object* { return p->handleSkin().rawPtr(); } ),
			ptsRow    ( "Handle thickness (pts): ",   [](SplitPanel* p) { return p->handleThickness(); } ),
			decimalRow( "Resize ratio: ",             [](SplitPanel* p) { return p->resizeRatio(); } ),
			decimalRow( "Split: ",                    [](SplitPanel* p) { return p->split(); } ),
			boolRow   ( "Custom resize function: ",   [](SplitPanel* p) { return bool(p->resizeFunction()); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SplitPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SplitPanelInfoSection::refresh()
	{
		TypedInfoSection<SplitPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
