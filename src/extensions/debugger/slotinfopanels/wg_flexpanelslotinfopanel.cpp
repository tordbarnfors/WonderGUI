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

	FlexPanelSlotInfoPanel::FlexPanelSlotInfoPanel(const Blueprint& blueprint, StaticSlot * pStaticSlot) : DebugPanel( blueprint )
	{
		auto pPanelSlot = static_cast<const FlexPanelSlot*>(pStaticSlot);

		bool bPinned = pPanelSlot->isPinned();

		auto pTable = _createTable( bPinned ? 3 : 5, 2 );

		char temp[256];
		int row = 0;

		_setTextEntry(pTable, row++, "Mode: ", bPinned ? "Pinned" : "Movable");

		if (bPinned)
		{
			flexPosToString(pPanelSlot->origo(), temp);
			_setTextEntry(pTable, row++, "Origo: ", temp);

			flexPosToString(pPanelSlot->hotspot(), temp);
			_setTextEntry(pTable, row++, "Hotspot: ", temp);

			Coord ofs = pPanelSlot->offset();

			_setPtsEntry(pTable, row++, "Offset X (pts): ", ofs.x);
			_setPtsEntry(pTable, row++, "Offset Y (pts): ", ofs.y);
		}
		else
		{
			flexPosToString(pPanelSlot->topLeftPin(), temp);
			_setTextEntry(pTable, row++, "TopLeft pin: ", temp);

			flexPosToString(pPanelSlot->bottomRightPin(), temp);
			_setTextEntry(pTable, row++, "BottomRight pin: ", temp);
		}

		this->slot = pTable;
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



} // namespace wg



