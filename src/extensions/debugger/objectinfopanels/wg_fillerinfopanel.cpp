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
		auto pTable = _createTable(2,2);
		_setPtsEntry(pTable, 0, "Default width (pts): ", pFiller->defaultSize().w);
		_setPtsEntry(pTable, 1, "Default height (pts): ", pFiller->defaultSize().h);
		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FillerInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


