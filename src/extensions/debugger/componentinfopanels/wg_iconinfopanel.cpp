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
#include "wg_iconinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo IconInfoPanel::TYPEINFO = { "IconInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	IconInfoPanel::IconInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Icon* pIcon) : DebugPanel(blueprint, pHolder, Icon::TYPEINFO.className)
	{
		m_pInspected = pIcon;

		m_pTable = _createTable(3, 2);

		_initTextEntry(m_pTable, 0, "Placement: ");
		_initPtsEntry(m_pTable, 1, "Spacing (pts): ");
		_initObjectPointerEntry(m_pTable, 2, "Skin: ");

		this->slot = m_pTable;

		refresh();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& IconInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void IconInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->placement()) );
		_refreshIntegerEntry(m_pTable, 1, m_pInspected->spacing() );
		_refreshObjectPointerEntry(m_pTable, 2, m_pInspected->skin(), m_displayedSkinPtr );
		
	}



} // namespace wg


