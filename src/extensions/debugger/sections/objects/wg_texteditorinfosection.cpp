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


namespace wg
{

	const TypeInfo TextEditorInfoSection::TYPEINFO = { "TextEditorInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextEditorInfoSection::TextEditorInfoSection(const DebugTheme& theme, IDebugContext* pContext, TextEditor * pInspected)
		: TypedInfoSection<TextEditor>( theme, pContext, TextEditor::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow( "Return action: ", [](TextEditor* e) { return toString(e->returnKeyAction()); } ),
			textRow( "Tab action: ",    [](TextEditor* e) { return toString(e->tabKeyAction()); } )
		});

		m_pEditorDrawer = _createComponentDrawer("Editor", &pInspected->editor);

		pPanel->slots.pushBack({ pTable, m_pEditorDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextEditorInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TextEditorInfoSection::refresh()
	{
		TypedInfoSection<TextEditor>::refresh();

		_refreshComponentDrawer(m_pEditorDrawer);
	}

} // namespace wg
