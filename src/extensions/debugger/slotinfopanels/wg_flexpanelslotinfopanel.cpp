/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#include "wg_flexpanelslotinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_flexpanel.h>

namespace wg
{

	const TypeInfo FlexPanelSlotInfoPanel::TYPEINFO = { "FlexPanelSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	FlexPanelSlotInfoPanel::FlexPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, FlexPanelSlot::TYPEINFO.className )
	{
		m_pInspected = static_cast<FlexPanelSlot*>(pStaticSlot);

		m_pTable = _createTable( 7, 2 );

		int row = 0;

		_initTextEntry(m_pTable, row++, "Mode: ");
		_initTextEntry(m_pTable, row++, "Origo: ");
		_initTextEntry(m_pTable, row++, "Hotspot: ");

		_initPtsEntry(m_pTable, row++, "Offset X (pts): ");
		_initPtsEntry(m_pTable, row++, "Offset Y (pts): ");
		_initTextEntry(m_pTable, row++, "TopLeft pin: ");
		_initTextEntry(m_pTable, row++, "BottomRight pin: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FlexPanelSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ flexPosToString() __________________________________________________

	void FlexPanelSlotInfoPanel::flexPosToString(FlexPos pos, char * pString)
	{
		sprintf(pString, "Relative (%f,%f) + (%f,%f) pts", pos.origo.x, pos.origo.y, pos.offset.x, pos.offset.y);
	}

	//____ refresh() _____________________________________________________________

	void FlexPanelSlotInfoPanel::refresh()
	{
		bool bPinned = m_pInspected->isPinned();

		char temp[256];
		int row = 0;

		_refreshTextEntry(m_pTable, row++, bPinned ? "Pinned" : "Movable");

		flexPosToString(m_pInspected->origo(), temp);
		_refreshTextEntry(m_pTable, row++, temp);

		flexPosToString(m_pInspected->hotspot(), temp);
		_refreshTextEntry(m_pTable, row++, temp);

		Coord ofs = m_pInspected->offset();

		_refreshPtsEntry(m_pTable, row++, ofs.x);
		_refreshPtsEntry(m_pTable, row++, ofs.y);

		flexPosToString(m_pInspected->topLeftPin(), temp);
		_refreshTextEntry(m_pTable, row++, temp);

		flexPosToString(m_pInspected->bottomRightPin(), temp);
		_refreshTextEntry(m_pTable, row++, temp);

		m_pTable->rows[1].setVisible(bPinned);
		m_pTable->rows[2].setVisible(bPinned);
		m_pTable->rows[3].setVisible(!bPinned);
		m_pTable->rows[4].setVisible(!bPinned);
		m_pTable->rows[5].setVisible(!bPinned);
		m_pTable->rows[6].setVisible(!bPinned);

	}

} // namespace wg



