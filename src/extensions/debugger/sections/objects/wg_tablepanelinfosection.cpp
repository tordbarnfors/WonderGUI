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
#include "wg_tablepanelinfosection.h"
#include <wg_tablepanel.h>
#include <wg_packpanel.h>


namespace wg
{

	//____ countVisible() _________________________________________________________

	template<class EntryVector>
	static int countVisible(const EntryVector& entries)
	{
		int nb = 0;
		for (auto& entry : entries)
			if (entry.isVisible())
				nb++;
		return nb;
	}

	const TypeInfo TablePanelInfoSection::TYPEINFO = { "TablePanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TablePanelInfoSection::TablePanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, TablePanel * pInspected)
		: TypedInfoSection<TablePanel>( theme, pContext, TablePanel::TYPEINFO.className, pInspected )
	{
		//TODO: Row/column spacing and row skins (no public getters).
		//TODO: Per row/column weight, min/max size and visibility.

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			intRow   ( "Rows: ",             [](TablePanel* p) { return p->rows.size(); } ),
			intRow   ( "Visible rows: ",     [](TablePanel* p) { return countVisible(p->rows); } ),
			intRow   ( "Columns: ",          [](TablePanel* p) { return p->columns.size(); } ),
			intRow   ( "Visible columns: ",  [](TablePanel* p) { return countVisible(p->columns); } ),
			objectRow( "Row layout: ",       [](TablePanel* p) -> Object* { return p->rowLayout().rawPtr(); } ),
			objectRow( "Column layout: ",    [](TablePanel* p) -> Object* { return p->columnLayout().rawPtr(); } )
		});

		m_pSlotsDrawer = _createSlotsDrawer("Slots", pInspected->slots.data(), pInspected->slots.data() + pInspected->slots.slots());

		pBasePanel->slots.pushBack({ pTable, m_pSlotsDrawer });
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TablePanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void TablePanelInfoSection::refresh()
	{
		TypedInfoSection<TablePanel>::refresh();

		_refreshSlotsDrawer(m_pSlotsDrawer, inspected()->slots.data(), inspected()->slots.data() + inspected()->slots.slots());
	}

} // namespace wg
