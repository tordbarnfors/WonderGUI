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
#include "wg_staticslotinfopanel.h"
#include <wg_textdisplay.h>

namespace wg
{

	const TypeInfo StaticSlotInfoPanel::TYPEINFO = { "StaticSlotInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	StaticSlotInfoPanel::StaticSlotInfoPanel(const Blueprint& blueprint, StaticSlot * pStaticSlot) : DebugPanel( blueprint )
	{
		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = 4 ));

		int row = 0;

		RectF geo = pStaticSlot->geo();

		pTable->slots[row][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "X offset (pts): " ));
		pTable->slots[row][1] = NumberDisplay::create( WGOVR(blueprint.listEntryPts, _.display.value = geo.x));
		row++;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Y offset (pts): "));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(blueprint.listEntryPts, _.display.value = geo.y));
		row++;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Width (pts): "));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(blueprint.listEntryPts, _.display.value = geo.w));
		row++;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Height (pts): "));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(blueprint.listEntryPts, _.display.value = geo.h));
		row++;

		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg



