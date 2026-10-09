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
#include "wg_lineeditorinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo LineEditorInfoSection::TYPEINFO = { "LineEditorInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	LineEditorInfoSection::LineEditorInfoSection(const DebugTheme& theme, IDebugContext* pContext, LineEditor * pInspected)
		: TypedInfoSection<LineEditor>( theme, pContext, LineEditor::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			intRow ( "Default length (chars): ", [](LineEditor* e) { return e->defaultLengthInChars(); } ),
			textRow( "Return key action: ",      [](LineEditor* e) { return toString(e->returnKeyAction()); } )
		});

		m_pEditorDrawer = _createComponentDrawer("Editor", &pInspected->editor);

		pPanel->slots.pushBack({ pTable, m_pEditorDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LineEditorInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void LineEditorInfoSection::refresh()
	{
		TypedInfoSection<LineEditor>::refresh();

		_refreshComponentDrawer(m_pEditorDrawer);
	}

} // namespace wg
