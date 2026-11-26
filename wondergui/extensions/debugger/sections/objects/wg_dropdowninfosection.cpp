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
#include "wg_dropdowninfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo DropdownInfoSection::TYPEINFO = { "DropdownInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	DropdownInfoSection::DropdownInfoSection(const DebugTheme& theme, IDebugContext* pContext, Dropdown * pInspected)
		: TypedInfoSection<Dropdown>( theme, pContext, Dropdown::TYPEINFO.className, pInspected )
	{
		//TODO: List canvas, marked entry and open state (private members)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Entry skin: ",           [](Dropdown* s) -> Object* { return s->entrySkin().rawPtr(); } ),
			objectRow( "Entry style: ",          [](Dropdown* s) -> Object* { return s->entryStyle().rawPtr(); } ),
			objectRow( "Entry text layout: ",    [](Dropdown* s) -> Object* { return s->entryTextLayout().rawPtr(); } ),
			objectRow( "List skin: ",            [](Dropdown* s) -> Object* { return s->listSkin().rawPtr(); } ),
			intRow   ( "Number of entries: ",    [](Dropdown* s) { return s->entries.size(); } ),
			intRow   ( "Selected entry index: ", [](Dropdown* s) { return s->selectedEntryIndex(); } ),
			intRow   ( "Selected entry id: ",    [](Dropdown* s) { return s->selectedEntryId(); } )
		});

		m_pDisplayDrawer = _createPartDrawer("Display", &pInspected->display);

		pPanel->slots.pushBack({ pTable, m_pDisplayDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DropdownInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void DropdownInfoSection::refresh()
	{
		TypedInfoSection<Dropdown>::refresh();

		_refreshPartDrawer(m_pDisplayDrawer);
	}

} // namespace wg
