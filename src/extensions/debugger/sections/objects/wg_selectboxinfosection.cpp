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
#include "wg_selectboxinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo SelectBoxInfoSection::TYPEINFO = { "SelectBoxInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SelectBoxInfoSection::SelectBoxInfoSection(const DebugTheme& theme, IDebugContext* pContext, SelectBox * pInspected)
		: TypedInfoSection<SelectBox>( theme, pContext, SelectBox::TYPEINFO.className, pInspected )
	{
		//TODO: List canvas, marked entry and open state (private members)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Entry skin: ",           [](SelectBox* s) -> Object* { return s->entrySkin().rawPtr(); } ),
			objectRow( "Entry style: ",          [](SelectBox* s) -> Object* { return s->entryStyle().rawPtr(); } ),
			objectRow( "Entry text layout: ",    [](SelectBox* s) -> Object* { return s->entryTextLayout().rawPtr(); } ),
			objectRow( "List skin: ",            [](SelectBox* s) -> Object* { return s->listSkin().rawPtr(); } ),
			intRow   ( "Number of entries: ",    [](SelectBox* s) { return s->entries.size(); } ),
			intRow   ( "Selected entry index: ", [](SelectBox* s) { return s->selectedEntryIndex(); } ),
			intRow   ( "Selected entry id: ",    [](SelectBox* s) { return s->selectedEntryId(); } )
		});

		m_pDisplayDrawer = _createComponentDrawer("Display", &pInspected->display);

		pPanel->slots.pushBack({ pTable, m_pDisplayDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SelectBoxInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SelectBoxInfoSection::refresh()
	{
		TypedInfoSection<SelectBox>::refresh();

		_refreshComponentDrawer(m_pDisplayDrawer);
	}

} // namespace wg
