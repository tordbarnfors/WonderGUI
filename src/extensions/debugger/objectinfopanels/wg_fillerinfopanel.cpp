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
#include "wg_fillerinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo FillerInfoPanel::TYPEINFO = { "FillerInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	FillerInfoPanel::FillerInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Filler * pFiller) : DebugPanel( blueprint, pHolder, Filler::TYPEINFO.className )
	{
		m_pInspected = pFiller;

		m_pTable = _createTable(2,2);
		_setPtsEntry(m_pTable, 0, "Default width (pts): ", pFiller->defaultSize().w);
		_setPtsEntry(m_pTable, 1, "Default height (pts): ", pFiller->defaultSize().h);
		this->slot = m_pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FillerInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void FillerInfoPanel::refresh()
	{
		_refreshPtsEntry(m_pTable, 0, m_pInspected->defaultSize().w);
		_refreshPtsEntry(m_pTable, 1, m_pInspected->defaultSize().h);
	}

} // namespace wg


