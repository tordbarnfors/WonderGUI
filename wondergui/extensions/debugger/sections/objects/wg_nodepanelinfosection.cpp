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
#include "wg_nodepanelinfosection.h"
#include <wg_nodepanel.h>
#include <wg_packpanel.h>


namespace wg
{

	//____ toString() _____________________________________________________________

	static const char* toString(NodePanel::NodeConstraint constraint)
	{
		switch (constraint)
		{
			case NodePanel::NodeConstraint::Center:
				return "Center";
			case NodePanel::NodeConstraint::Bounds:
				return "Bounds";
			default:
				return "Unknown";
		}
	}

	const TypeInfo NodePanelInfoSection::TYPEINFO = { "NodePanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	NodePanelInfoSection::NodePanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodePanel * pInspected)
		: TypedInfoSection<NodePanel>( theme, pContext, NodePanel::TYPEINFO.className, pInspected )
	{
		//TODO: Default size, normalized, drag button and node pos modifier (no public getters).

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow( "Node constraint: ",  [](NodePanel* p) { return toString(p->nodeConstraint()); } ),
			intRow ( "Selected node: ",    [](NodePanel* p) { return p->selectedNode(); } ),
			intRow ( "Number of nodes: ",  [](NodePanel* p) { return p->nodes.size(); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.begin(), pInspected->slots.end());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NodePanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void NodePanelInfoSection::refresh()
	{
		TypedInfoSection<NodePanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.begin(), inspected()->slots.end());
	}

} // namespace wg
