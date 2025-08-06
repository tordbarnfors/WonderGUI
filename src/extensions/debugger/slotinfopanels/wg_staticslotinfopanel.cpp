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
		m_pTable = _createTable( 5, 2);

		int row = 0;

		_initObjectPointerEntry(m_pTable, row++, "Widget: ");
		_initPtsEntry(m_pTable, row++, "X offset (pts): ");
		_initPtsEntry(m_pTable, row++, "Y offset (pts): ");
		_initPtsEntry(m_pTable, row++, "Width (pts): ");
		_initPtsEntry(m_pTable, row++, "Height (pts): ");

		refresh(pStaticSlot);

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticSlotInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void StaticSlotInfoPanel::refresh(StaticSlot * pStaticSlot)
	{
		int row = 0;

		RectF geo = pStaticSlot->geo();

		_refreshObjectPointerEntry(m_pTable, row++, pStaticSlot->widget(), m_pDisplayedChild);
		_refreshPtsEntry(m_pTable, row++, geo.x);
		_refreshPtsEntry(m_pTable, row++, geo.y);
		_refreshPtsEntry(m_pTable, row++, geo.w);
		_refreshPtsEntry(m_pTable, row++, geo.h);
	}


} // namespace wg



