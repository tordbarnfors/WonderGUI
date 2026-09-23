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
#include "wg_flexpanelslotinfosection.h"
#include <wg_textdisplay.h>
#include <wg_flexpanel.h>

namespace wg
{

	//____ flexPosToString() __________________________________________________

	static String flexPosToString(const FlexPos& pos)
	{
		char temp[256];
		snprintf(temp, sizeof(temp), "Relative (%f,%f) + (%f,%f) pts", pos.origo.x, pos.origo.y, pos.offset.x, pos.offset.y);
		return String(temp);
	}


	const TypeInfo FlexPanelSlotInfoSection::TYPEINFO = { "FlexPanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	FlexPanelSlotInfoSection::FlexPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, FlexPanelSlot * pInspected)
		: TypedInfoSection<FlexPanelSlot>( theme, pContext, FlexPanelSlot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow( "Mode: ",             [](FlexPanelSlot* s) { return s->isPinned() ? "Pinned" : "Movable"; } ),
			textRow( "Origo: ",            [](FlexPanelSlot* s) { return flexPosToString(s->origo()); } ),
			textRow( "Hotspot: ",          [](FlexPanelSlot* s) { return flexPosToString(s->hotspot()); } ),
			ptsRow ( "Offset X (pts): ",   [](FlexPanelSlot* s) { return s->offset().x; } ),
			ptsRow ( "Offset Y (pts): ",   [](FlexPanelSlot* s) { return s->offset().y; } ),
			textRow( "TopLeft pin: ",      [](FlexPanelSlot* s) { return flexPosToString(s->topLeftPin()); } ),
			textRow( "BottomRight pin: ",  [](FlexPanelSlot* s) { return flexPosToString(s->bottomRightPin()); } )
		});

		_updateRowVisibility();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void FlexPanelSlotInfoSection::refresh()
	{
		TypedInfoSection<FlexPanelSlot>::refresh();

		_updateRowVisibility();
	}

	//____ _updateRowVisibility() _____________________________________________
	//
	// Movable slots are placed by origo, hotspot and offset, pinned ones by
	// their two pins. Only show the rows that apply.

	void FlexPanelSlotInfoSection::_updateRowVisibility()
	{
		bool bPinned = inspected()->isPinned();

		m_pRowTable->rows[1].setVisible(!bPinned);
		m_pRowTable->rows[2].setVisible(!bPinned);
		m_pRowTable->rows[3].setVisible(!bPinned);
		m_pRowTable->rows[4].setVisible(!bPinned);
		m_pRowTable->rows[5].setVisible(bPinned);
		m_pRowTable->rows[6].setVisible(bPinned);
	}

} // namespace wg
