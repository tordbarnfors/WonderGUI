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
#include "wg_statictextinfosection.h"
#include <wg_textdisplay.h>
#include <wg_enumextras.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo StaticTextInfoSection::TYPEINFO = { "StaticTextInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	StaticTextInfoSection::StaticTextInfoSection(const DebugTheme& theme, IDebugContext* pContext, StaticText * pInspected)
		: TypedInfoSection<StaticText>( theme, pContext, StaticText::TYPEINFO.className, pInspected )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow  ( "State: ",  [](StaticText* t) { return toString(t->state().value()); } ),
			objectRow( "Style: ",  [](StaticText* t) -> Object* { return t->style(); } ),
			objectRow( "Layout: ", [](StaticText* t) -> Object* { return t->layout(); } ),
			intRow   ( "Length: ", [](StaticText* t) { return t->length(); } )
		});

		m_pTextDisplay = WGCREATE(TextDisplay, _ = theme.textField, _.display.text = pInspected->text());
		auto pPadding = WGCREATE(PaddingCapsule, _.padding = { 0,0,0,16 }, _.child = m_pTextDisplay );

		pPanel->slots.pushBack({ pTable, pPadding });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticTextInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void StaticTextInfoSection::refresh()
	{
		TypedInfoSection<StaticText>::refresh();

		m_pTextDisplay->display.setText(inspected()->text());
	}

} // namespace wg
