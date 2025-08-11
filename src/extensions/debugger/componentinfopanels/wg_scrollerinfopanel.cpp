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
#include "wg_scrollerinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo ScrollerInfoPanel::TYPEINFO = { "ScrollerInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollerInfoPanel::ScrollerInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Scroller* pScroller) : DebugPanel(blueprint, pHolder, Scroller::TYPEINFO.className)
	{
		m_pInspected = pScroller;

		m_pTable = _createTable(11, 2);

		int ofs = 0;
		_initTextEntry(m_pTable, ofs++, "Axis: ");
		_initBoolEntry(m_pTable, ofs++, "Jump to press: ");
		_initBoolEntry(m_pTable, ofs++, "Visible: ");

		_initObjectPointerEntry(m_pTable, ofs++, "Back skin: ");
		_initObjectPointerEntry(m_pTable, ofs++, "Bar skin: ");
		_initObjectPointerEntry(m_pTable, ofs++, "Backward button skin: ");
		_initObjectPointerEntry(m_pTable, ofs++, "Forward button skin: ");

		_initTextEntry(m_pTable, ofs++, "Back state: ");
		_initTextEntry(m_pTable, ofs++, "Bar state: ");
		_initTextEntry(m_pTable, ofs++, "Backward button state: ");
		_initTextEntry(m_pTable, ofs++, "Forward button state: ");


		this->slot = m_pTable;

		refresh();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollerInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScrollerInfoPanel::refresh()
	{
		int ofs = 0;
		_refreshTextEntry(m_pTable, ofs++, toString(m_pInspected->_axis()) );
		_refreshBoolEntry(m_pTable, ofs++, m_pInspected->jumpToPress() );
		_refreshBoolEntry(m_pTable, ofs++, m_pInspected->isVisible() );
		_refreshObjectPointerEntry(m_pTable, ofs++, m_pInspected->backSkin(), m_displayedBackgroundSkinPtr);
		_refreshObjectPointerEntry(m_pTable, ofs++, m_pInspected->barSkin(), m_displayedBarSkinPtr);
		_refreshObjectPointerEntry(m_pTable, ofs++, m_pInspected->backwardButtonSkin(), m_displayedBackwardButtonSkinPtr);
		_refreshObjectPointerEntry(m_pTable, ofs++, m_pInspected->forwardButtonSkin(), m_displayedForwardButtonSkinPtr);
		_refreshTextEntry(m_pTable, ofs++, toString( m_pInspected->_backState().value()));
		_refreshTextEntry(m_pTable, ofs++, toString( m_pInspected->_barState().value()));
		_refreshTextEntry(m_pTable, ofs++, toString( m_pInspected->_backwardButtonState().value()));
		_refreshTextEntry(m_pTable, ofs++, toString( m_pInspected->_forwardButtonState().value()));
	}



} // namespace wg


