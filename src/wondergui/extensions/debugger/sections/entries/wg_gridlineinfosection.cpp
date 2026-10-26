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
#include "wg_gridlineinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo GridLineInfoSection::TYPEINFO = { "GridLineInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	GridLineInfoSection::GridLineInfoSection(const DebugTheme& theme, IDebugContext* pContext, GridLine * pInspected)
		: TypedInfoSection<GridLine>( theme, pContext, "GridLine", pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			decimalRow( "Position: ",                  [](GridLine* l) { return l->pos(); } ),
			ptsRow    ( "Thickness (pts): ",           [](GridLine* l) { return l->thickness(); } ),
			boolRow   ( "Visible: ",                   [](GridLine* l) { return l->isVisible(); } ),
			textRow   ( "Label: ",                     [](GridLine* l) { return l->label(); } ),
			ptsRow    ( "Label adjustment X (pts): ",  [](GridLine* l) { return l->labelAdjustment().x; } ),
			ptsRow    ( "Label adjustment Y (pts): ",  [](GridLine* l) { return l->labelAdjustment().y; } ),
			boolRow   ( "Label at end: ",              [](GridLine* l) { return l->labelAtEnd(); } ),
			textRow   ( "Label placement: ",           [](GridLine* l) { return toString(l->labelPlacement()); } ),
			objectRow ( "Label skin: ",                [](GridLine* l) -> Object* { return l->labelSkin().rawPtr(); } ),
			objectRow ( "Text style: ",                [](GridLine* l) -> Object* { return l->textStyle().rawPtr(); } ),
			objectRow ( "Text layout: ",               [](GridLine* l) -> Object* { return l->textLayout().rawPtr(); } )
		});

		m_displayedColor = pInspected->color();
		m_pColorDrawer = _createColorDrawer("Color: ", m_displayedColor);

		pPanel->slots.pushBack({ pTable, m_pColorDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GridLineInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void GridLineInfoSection::refresh()
	{
		TypedInfoSection<GridLine>::refresh();

		_refreshColorDrawer(m_pColorDrawer, inspected()->color(), m_displayedColor);
	}

} // namespace wg
