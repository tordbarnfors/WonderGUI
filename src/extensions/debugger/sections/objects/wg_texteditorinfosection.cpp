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
#include "wg_texteditorinfosection.h"
#include <wg_texteditor.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>


namespace wg
{

	const TypeInfo TextEditorInfoSection::TYPEINFO = { "TextEditorInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextEditorInfoSection::TextEditorInfoSection(const DebugTheme& theme, IDebugContext* pContext, TextEditor * pTextEditor) : InfoSection( theme, pContext, TextEditor::TYPEINFO.className )
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

	const TypeInfo& TextEditorInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TextEditorInfoSection::refresh()
	{
		_refreshTextEntry(m_pTable, 0, toString(m_pInspected->returnKeyAction()));
		_refreshTextEntry(m_pTable, 1, toString(m_pInspected->tabKeyAction()));

		_refreshComponentDrawer( m_pEditorDrawer );
	}


} // namespace wg


