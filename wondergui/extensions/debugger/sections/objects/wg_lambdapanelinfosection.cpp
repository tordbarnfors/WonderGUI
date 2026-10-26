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
#include "wg_lambdapanelinfosection.h"
#include <wg_lambdapanel.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo LambdaPanelInfoSection::TYPEINFO = { "LambdaPanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	LambdaPanelInfoSection::LambdaPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, LambdaPanel * pInspected)
		: TypedInfoSection<LambdaPanel>( theme, pContext, LambdaPanel::TYPEINFO.className, pInspected )
	{
		//TODO: Min, max and default size (no public getters).

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow( "Edge policy: ", [](LambdaPanel* p) { return toString(p->edgePolicy()); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LambdaPanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void LambdaPanelInfoSection::refresh()
	{
		TypedInfoSection<LambdaPanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
