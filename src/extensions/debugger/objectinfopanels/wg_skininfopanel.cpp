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
#include "wg_skininfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>
#include <wg_skindisplay.h>
#include <wg_tileskin.h>
#include <wg_boxskin.h>


namespace wg
{

	const TypeInfo SkinInfoPanel::TYPEINFO = { "SkinInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinInfoPanel::SkinInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Skin * pSkin) : DebugPanel( blueprint, pHolder, Skin::TYPEINFO.className )
	{
		auto pBasePanel = WGCREATE( PackPanel, _.axis = Axis::Y );

		auto pTable = _createTable(7, 2);

		_setBoolEntry(pTable, 0, "Opaque:", pSkin->isOpaque());
		_setBoolEntry(pTable, 1, "Content shifting:", pSkin->isContentShifting());
		_setBoolEntry(pTable, 2, "Ignores value:", pSkin->_ignoresValue());
		_setBoolEntry(pTable, 3, "Ignores state:", pSkin->_ignoresState());
		_setBoolEntry(pTable, 4, "Overflows:", pSkin->_hasOverflow());
		_setIntegerEntry(pTable, 5, "Layer:", pSkin->layer());
		_setIntegerEntry(pTable, 6, "Mark alpha:", pSkin->markAlpha());
		pBasePanel->slots << pTable;

		pBasePanel->slots << _createBorderDrawer("Margin", pSkin->margin());
		pBasePanel->slots << _createBorderDrawer("Padding", pSkin->padding());
		pBasePanel->slots << _createBorderDrawer("Overflow", pSkin->overflow());
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


