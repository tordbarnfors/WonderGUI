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

	StaticTextInfoPanel::StaticTextInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, StaticText* pStaticText) : DebugPanel(blueprint, pHolder)
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y );

		auto pTable = _createTable(4, 2);

		_setTextEntry(pTable, 0, "State: ", toString(pStaticText->state().value()) );
		_setObjectPointerEntry(pTable, 1, "Style: ", pStaticText->style(), this );
		_setObjectPointerEntry(pTable, 2, "Layout: ", pStaticText->layout(), this);
		_setIntegerEntry(pTable, 3, "Length: ", pStaticText->length());

		pPanel->slots << pTable;

		pPanel->slots << WGCREATE(TextDisplay, _ = m_blueprint.textField, _.display.text = pStaticText->text());

		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticTextInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


