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

	StaticSlotInfoPanel::StaticSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) : DebugPanel( blueprint, pHolder, StaticSlot::TYPEINFO.className )
	{
		auto pTable = _createTable( 5, 2);

		int row = 0;

		RectF geo = pStaticSlot->geo();

		_setObjectPointerEntry(pTable, row++, "Widget: ", pStaticSlot->widget(), this);
		_setPtsEntry(pTable, row++, "X offset (pts): ", geo.x);
		_setPtsEntry(pTable, row++, "Y offset (pts): ", geo.y);
		_setPtsEntry(pTable, row++, "Width (pts): ", geo.w);
		_setPtsEntry(pTable, row++, "Height (pts): ", geo.h);

		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg



