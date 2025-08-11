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
#include "wg_texteditorinfopanel.h"
#include <wg_TextEditor.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo TextEditorInfoPanel::TYPEINFO = { "TextEditorInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextEditorInfoPanel::TextEditorInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, TextEditor * pTextEditor) : DebugPanel( blueprint, pHolder, TextEditor::TYPEINFO.className )
	{
		m_pInspected = pTextEditor;

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pTable = _createTable(2, 2);
		_initTextEntry(m_pTable, 0, "Return action: ");
		_initTextEntry(m_pTable, 1, "Tab action: ");


		m_pEditorDrawer = _createComponentDrawer("Editor", &pTextEditor->editor);

		pPanel->slots.pushBack({ m_pTable, m_pEditorDrawer });
		this->slot = pPanel;

		refresh();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextEditorInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TextEditorInfoPanel::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->returnKeyAction()));
		_refreshTextEntry(m_pTable, 1, toString(m_pInspected->tabKeyAction()));

		_refreshComponentDrawer( static_cast<DrawerPanel*>(m_pEditorDrawer) );
	}


} // namespace wg


