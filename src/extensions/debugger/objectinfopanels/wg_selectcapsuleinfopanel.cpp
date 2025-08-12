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
#include "wg_selectcapsuleinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo SelectCapsuleInfoPanel::TYPEINFO = { "SelectCapsuleInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	SelectCapsuleInfoPanel::SelectCapsuleInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, SelectCapsule * pCapsule) : DebugPanel( blueprint, pHolder, SelectCapsule::TYPEINFO.className )
	{
		m_pInspected = pCapsule;
		m_pTable = _createTable(2,2);

		_initTextEntry(m_pTable, 0, "Select mode: ");
		_initBoolEntry(m_pTable, 1, "Recursive: ");

		refresh();

		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SelectCapsuleInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SelectCapsuleInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->selectMode()));
		_refreshBoolEntry(m_pTable, 1, m_pInspected->isRecursive());
	}

} // namespace wg


