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
#include "wg_statictextinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo StaticTextInfoPanel::TYPEINFO = { "StaticTextInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	StaticTextInfoPanel::StaticTextInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticText* pStaticText) : DebugPanel(blueprint, pHolder, StaticText::TYPEINFO.className)
	{
		m_pInspected = pStaticText;

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y );

		m_pTable = _createTable(4, 2);

		_setTextEntry(m_pTable, 0, "State: ", toString(pStaticText->state().value()) );
		_setObjectPointerEntry(m_pTable, 1, "Style: ", pStaticText->style(), this );
		_setObjectPointerEntry(m_pTable, 2, "Layout: ", pStaticText->layout(), this);
		_setIntegerEntry(m_pTable, 3, "Length: ", pStaticText->length());

		m_pTextDisplay = WGCREATE(TextDisplay, _ = blueprint.textField, _.display.text = pStaticText->text());
		auto pPadding = WGCREATE(PaddingCapsule, _.padding = { 0,0,0,16 }, _.child = m_pTextDisplay );

		pPanel->slots.pushBack({m_pTable, pPadding});
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticTextInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void StaticTextInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->state().value()) );
		_refreshObjectPointerEntry(m_pTable, 1, m_pInspected->style(), m_displayedStylePtr );
		_refreshObjectPointerEntry(m_pTable, 2, m_pInspected->layout(), m_displayedLayoutPtr );
		_refreshIntegerEntry(m_pTable, 3, m_pInspected->length());

		m_pTextDisplay->display.setText(m_pInspected->text());
	}



} // namespace wg


