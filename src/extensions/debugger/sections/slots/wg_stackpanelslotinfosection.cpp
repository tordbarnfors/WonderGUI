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
#include "wg_stackpanelslotinfosection.h"
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo StackPanelSlotInfoSection::TYPEINFO = { "StackPanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	StackPanelSlotInfoSection::StackPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, StackPanelSlot * pInspected)
		: TypedInfoSection<StackPanelSlot>( theme, pContext, StackPanelSlot::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow( "Size policy: ", [](StackPanelSlot* s) { return toString(s->sizePolicy()); } ),
			textRow( "Placement: ",   [](StackPanelSlot* s) { return toString(s->placement()); } )
		});

		m_displayedMargin = pInspected->margin();
		m_pMarginDrawer = _createBorderDrawer("Margin: ", m_displayedMargin);

		pPanel->slots.pushBack({ pTable, m_pMarginDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StackPanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void StackPanelSlotInfoSection::refresh()
	{
		TypedInfoSection<StackPanelSlot>::refresh();

		_refreshBorderDrawer(m_pMarginDrawer, inspected()->margin(), m_displayedMargin);
	}

} // namespace wg
