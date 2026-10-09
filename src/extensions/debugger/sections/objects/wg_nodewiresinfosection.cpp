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
#include "wg_nodewiresinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo NodeWiresInfoSection::TYPEINFO = { "NodeWiresInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	NodeWiresInfoSection::NodeWiresInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodeWires * pInspected)
		: TypedInfoSection<NodeWires>( theme, pContext, NodeWires::TYPEINFO.className, pInspected )
	{
		//TODO: Observed NodePanel and list of wires (private members)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			ptsRow ( "Wire stub (pts): ",      [](NodeWires* n) { return n->wireStub(); } ),
			ptsRow ( "Wire thickness (pts): ", [](NodeWires* n) { return n->wireThickness(); } ),
			boolRow( "Orthogonal: ",           [](NodeWires* n) { return n->isOrthogonal(); } ),
			textRow( "Default axis: ",         [](NodeWires* n) { return toString(n->defaultAxis()); } )
		});

		m_displayedWireColor = pInspected->wireColor();
		m_displayedAnchorInset = pInspected->anchorInset();

		m_pWireColorDrawer = _createColorDrawer("Wire color: ", m_displayedWireColor);
		m_pAnchorInsetDrawer = _createBorderDrawer("Anchor inset: ", m_displayedAnchorInset);

		pPanel->slots.pushBack({ pTable, m_pWireColorDrawer, m_pAnchorInsetDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NodeWiresInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void NodeWiresInfoSection::refresh()
	{
		TypedInfoSection<NodeWires>::refresh();

		_refreshColorDrawer(m_pWireColorDrawer, inspected()->wireColor(), m_displayedWireColor);
		_refreshBorderDrawer(m_pAnchorInsetDrawer, inspected()->anchorInset(), m_displayedAnchorInset);
	}

} // namespace wg
