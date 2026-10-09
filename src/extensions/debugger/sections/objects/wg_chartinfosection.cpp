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
#include "wg_chartinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ChartInfoSection::TYPEINFO = { "ChartInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ChartInfoSection::ChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, Chart * pInspected)
		: TypedInfoSection<Chart>( theme, pContext, Chart::TYPEINFO.className, pInspected )
	{
		//TODO: Grid color, grid thickness, grid on top (private, no getters)
		//TODO: Display skin, label skin, text style, text layout (private, no getters)
		//TODO: Label placements and spacings (private, no getters)
		//TODO: Grid line details (GridLine members are protected)
		//TODO: Range transition (private)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			decimalRow( "Display ceiling: ", [](Chart* c) { return c->displayCeiling(); } ),
			decimalRow( "Display floor: ",   [](Chart* c) { return c->displayFloor(); } ),
			intRow    ( "X grid lines: ",    [](Chart* c) { return c->xLines.size(); } ),
			intRow    ( "Y grid lines: ",    [](Chart* c) { return c->yLines.size(); } )
		});

		m_pGlowDrawer = _createComponentDrawer("Glow", &pInspected->glow);

		pPanel->slots.pushBack({ pTable, m_pGlowDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ChartInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ChartInfoSection::refresh()
	{
		TypedInfoSection<Chart>::refresh();

		_refreshComponentDrawer(m_pGlowDrawer);
	}

} // namespace wg
