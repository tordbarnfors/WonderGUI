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
#include "wg_packpanelinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo PackPanelInfoPanel::TYPEINFO = { "PackPanelInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	PackPanelInfoPanel::PackPanelInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, PackPanel * pPanel) : DebugPanel( blueprint, pHolder )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createTable(6,2);
		int row = 0;

		_setTextEntry(pTable, row++, "Axis: ", toString(pPanel->axis()));
		_setObjectPointerEntry(pTable, row++, "Layout: ", pPanel->layout(),this);
		_setPtsEntry(pTable, row++, "Spacing before (pts): ", pPanel->spacingBefore());
		_setPtsEntry(pTable, row++, "Spacing between (pts): ", pPanel->spacingBetween());
		_setPtsEntry(pTable, row++, "Spacing after (pts): ", pPanel->spacingAfter());
		_setTextEntry(pTable, row++, "Slot alignment: ", toString(pPanel->slotAlignment()));

		pBasePanel->slots << pTable;

		auto pNumberSlots = WGCREATE(NumberDisplay, _ = m_blueprint.listEntryInteger);
		pNumberSlots->display.set(pPanel->slots.size());

		auto pSlotList = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto bp = m_blueprint;

		for( int i = 0 ; i < pPanel->slots.size(); i++ )
		{
			char buf[32];
			sprintf(buf, "%d", i);

			auto pSlot = &pPanel->slots[i];

			auto pSlotContent = WGCREATE(PackPanel, _.axis = Axis::Y);

			auto pTypeInfo = &pPanel->slots[i].typeInfo();

			while( pTypeInfo != nullptr )
			{ 
				bp.classCapsule.label.text = pTypeInfo->className;
				pSlotContent->slots << m_pHolder->createSlotInfoPanel(bp, pTypeInfo, pSlot);
				pTypeInfo = pTypeInfo->pSuperClass;
			}

			auto pSlotDrawer = _createDrawer(buf, nullptr, pSlotContent);

			pSlotList->slots << pSlotDrawer;
		}

		pBasePanel->slots << _createDrawer("Slots", pNumberSlots, pSlotList);

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


