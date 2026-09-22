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
#include "wg_panelinfosection.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo PanelInfoSection::TYPEINFO = { "PanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelInfoSection::PanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, Panel * pPanel) : InfoSection( theme, pContext, Panel::TYPEINFO.className )
	{
		m_pInspected = pPanel;
		m_pTable = _createTable(1,2);
		_setTextEntry(m_pTable, 0, "Mask op: ", toString(pPanel->maskOp()));
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void PanelInfoSection::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->maskOp()));
	}

} // namespace wg


