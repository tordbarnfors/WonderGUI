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
#include "wg_editabletextinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo EditableTextInfoPanel::TYPEINFO = { "EditableTextInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	EditableTextInfoPanel::EditableTextInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, EditableText* pEditableText) : DebugPanel(blueprint, pHolder, EditableText::TYPEINFO.className)
	{
		m_pInspected = pEditableText;
		m_pTable = _createTable(3, 2);

		_initTextEntry(m_pTable, 0, "Edit mode: ");
		_initIntegerEntry(m_pTable, 1, "Max lines: ");
		_initIntegerEntry(m_pTable, 2, "Max chars: ");

		this->slot = m_pTable;

		refresh();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& EditableTextInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void EditableTextInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->editMode()) );
		_refreshIntegerEntry(m_pTable, 1, m_pInspected->maxLines());
		_refreshIntegerEntry(m_pTable, 2, m_pInspected->maxChars());
	}



} // namespace wg


