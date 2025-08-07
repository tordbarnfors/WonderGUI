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
#include "wg_textdisplayinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo TextDisplayInfoPanel::TYPEINFO = { "TextDisplayInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextDisplayInfoPanel::TextDisplayInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, TextDisplay * pTextDisplay) : DebugPanel( blueprint, pHolder, TextDisplay::TYPEINFO.className )
	{
		this->slot = _createComponentDrawer("Display", &pTextDisplay->display);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextDisplayInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TextDisplayInfoPanel::refresh()
	{
		_refreshComponentDrawer( static_cast<DrawerPanel*>(slot._widget()) );
	}


} // namespace wg


