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

		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = bPinned ? 3 : 5 ));

		char temp[256];
		int row = 0;

		pTable->slots[row][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "Mode: " ));
		pTable->slots[row][1] = TextDisplay::create( WGOVR(blueprint.listEntryText, _.display.text = bPinned ? "Pinned" : "Movable"));
		row++;

		if (bPinned)
		{
			flexPosToString(pPanelSlot->origo(), temp);
			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Origo: "));
			pTable->slots[row][1] = TextDisplay::create(WGOVR(blueprint.listEntryText, _.display.text = temp));
			row++;

			flexPosToString(pPanelSlot->hotspot(), temp);
			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Hotspot: "));
			pTable->slots[row][1] = TextDisplay::create(WGOVR(blueprint.listEntryText, _.display.text = temp));
			row++;

			Coord ofs = pPanelSlot->offset();

			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Offset X: "));
			pTable->slots[row][1] = NumberDisplay::create(WGOVR(blueprint.listEntryPts, _.display.value = ofs.x));
			row++;

			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Offset Y: "));
			pTable->slots[row][1] = NumberDisplay::create(WGOVR(blueprint.listEntryPts, _.display.value = ofs.y));
			row++;
		}
		else
		{
			flexPosToString(pPanelSlot->topLeftPin(), temp);
			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "TopLeft pin: "));
			pTable->slots[row][1] = TextDisplay::create(WGOVR(blueprint.listEntryText, _.display.text = temp));
			row++;

			flexPosToString(pPanelSlot->bottomRightPin(), temp);
			pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "BottomRight pin: "));
			pTable->slots[row][1] = TextDisplay::create(WGOVR(blueprint.listEntryText, _.display.text = temp));
			row++;
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



