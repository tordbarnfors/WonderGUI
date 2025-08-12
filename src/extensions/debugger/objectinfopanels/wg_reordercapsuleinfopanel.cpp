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
#include "wg_reordercapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ReorderCapsuleInfoPanel::TYPEINFO = { "ReorderCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ReorderCapsuleInfoPanel::ReorderCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, ReorderCapsule * pCapsule) : DebugPanel( blueprint, pHolder, ReorderCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pTable = _createTable(4,2);

		_initBoolEntry(m_pTable, 0, "Drag outside: ");
		_initObjectPointerEntry(m_pTable, 1, "Transition: ");
		_initIntegerEntry(m_pTable, 2, "Transition delay (microsec): ");
		_initObjectPointerEntry(m_pTable, 3, "Transition skin: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ReorderCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ReorderCapsuleInfoPanel::refresh()
	{
		_refreshBoolEntry(m_pTable, 0, m_pInspected->dragOutside());
		_refreshObjectPointerEntry(m_pTable, 1, m_pInspected->transition(), m_displayedTransitionPtr);
		_refreshIntegerEntry(m_pTable, 2, m_pInspected->transitionDelay());
		_refreshObjectPointerEntry(m_pTable, 3, m_pInspected->transitionSkin(), m_displayedTransitionSkinPtr);
	}

} // namespace wg


