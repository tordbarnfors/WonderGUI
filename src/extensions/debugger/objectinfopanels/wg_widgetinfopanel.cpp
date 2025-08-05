
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
#include "wg_widgetinfopanel.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_twoslotpanel.h>
#include <wg_enumextras.h>

namespace wg
{

	const TypeInfo WidgetInfoPanel::TYPEINFO = { "WidgetInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	WidgetInfoPanel::WidgetInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Widget * pWidget) : DebugPanel( blueprint, pHolder, Widget::TYPEINFO.className )
	{
		m_pInspected = pWidget;

		m_pInspectedsBaggage = pWidget->baggage().rawPtr();
		m_pInspectedsParent = pWidget->parent().rawPtr();
		m_pInspectedsSkin = pWidget->skin().rawPtr();

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createTable(19, 2);

		int row = 0;

		_setIntegerEntry		(pTable, row++, "Id: ", pWidget->id());
		_setPtsEntry			(pTable, row++, "Width (pts): ", pWidget->size().w);
		_setPtsEntry			(pTable, row++, "Height (pts): ", pWidget->size().h);
		_setIntegerEntry		(pTable, row++, "Scale: ", pWidget->scale());
		_setTextEntry			(pTable, row++, "State: ", toString(pWidget->state().value()) );
		_setObjectPointerEntry	(pTable, row++, "Baggage: ", pWidget->baggage().rawPtr(), pWidget);
		_setObjectPointerEntry	(pTable, row++, "Parent: ", pWidget->parent().rawPtr(), pWidget);
		_setObjectPointerEntry	(pTable, row++, "Skin: ", pWidget->skin().rawPtr(), pWidget);
		_setTextEntry			(pTable, row++, "Tooltip: ", pWidget->tooltip());
		_setTextEntry			(pTable, row++, "Pointer style: ", toString(pWidget->pointerStyle()));
		_setTextEntry			(pTable, row++, "Mark policy: ", toString(pWidget->markPolicy()));
		_setBoolEntry			(pTable, row++, "Pickable: ", pWidget->isPickable());
		_setBoolEntry			(pTable, row++, "PickHandle: ", pWidget->isPickHandle());
		_setIntegerEntry		(pTable, row++, "Pick category: ", pWidget->pickCategory());
		_setBoolEntry			(pTable, row++, "DropTarget: ", pWidget->isDropTarget());
		_setBoolEntry			(pTable, row++, "TabLocked: ", pWidget->isTabLocked());
		_setBoolEntry			(pTable, row++, "Selectable: ", pWidget->isSelectable());
		_setIntegerEntry		(pTable, row++, "Receiving updates: ", pWidget->m_receivingUpdateCounter);
		_setBoolEntry			(pTable, row++, "Sticky focus: ", pWidget->hasStickyFocus());

		pPanel->slots << pTable;
		m_pTable = pTable;

		{
			bool bOverflow = pWidget->m_bOverflow;

			auto pHeaderValue = WGCREATE(TextDisplay, _ = blueprint.listEntryText, _.display.text = bOverflow ? "true" : "false");

			TablePanel_p pOverflowTable;

			pOverflowTable = _createTable(4,2);

			BorderSPX overflow = pWidget->_overflow();

			_setSpxEntry(pOverflowTable, 0, "Top (spx): ", overflow.top);
			_setSpxEntry(pOverflowTable, 1, "Right (spx): ", overflow.right);
			_setSpxEntry(pOverflowTable, 2, "Bottom (spx): ", overflow.bottom);
			_setSpxEntry(pOverflowTable, 3, "Left (spx): ", overflow.left);

			auto pOverflowDrawer = _createDrawer("Has overflow", pHeaderValue, pOverflowTable);
			pPanel->slots << pOverflowDrawer;
		}

		auto pSlot = pWidget->_slot();
		if (pSlot)
		{
			Blueprint bp = m_pHolder->blueprint();

			auto pContentPanel = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y, _.spacingBefore = 4, _.spacingAfter = 4));

			const TypeInfo* pTypeInfo = &pSlot->typeInfo();

			while( pTypeInfo != nullptr )
			{
				bp.classCapsule.label.text = pTypeInfo->className;
				auto pInfoPanel = m_pHolder->createSlotInfoPanel(pTypeInfo, pSlot);
				if( pInfoPanel )
					pContentPanel->slots << pInfoPanel;

				pTypeInfo = pTypeInfo->pSuperClass;
			}

			char temp[64];
			sprintf(temp, "0x%p", pSlot);

			auto pHeaderValue = WGCREATE(TextDisplay, _ = blueprint.listEntryText, _.display.text = temp);
			auto pSlotDrawer = _createDrawer("Slot", pHeaderValue, pContentPanel);
			pPanel->slots << pSlotDrawer;

		}


		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& WidgetInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void WidgetInfoPanel::refresh()
	{

		int row = 0;
		auto pWidget = m_pInspected;

		_refreshIntegerEntry		(m_pTable, row++, pWidget->id());
		_refreshPtsEntry			(m_pTable, row++, pWidget->size().w);
		_refreshPtsEntry			(m_pTable, row++, pWidget->size().h);
		_refreshIntegerEntry		(m_pTable, row++, pWidget->scale());
		_refreshTextEntry			(m_pTable, row++, toString(pWidget->state().value()) );
		_refreshObjectPointerEntry	(m_pTable, row++, pWidget->baggage().rawPtr(), m_pInspectedsBaggage);
		_refreshObjectPointerEntry	(m_pTable, row++, pWidget->parent().rawPtr(), m_pInspectedsParent);
		_refreshObjectPointerEntry	(m_pTable, row++, pWidget->skin().rawPtr(), m_pInspectedsSkin);
		_refreshTextEntry			(m_pTable, row++, pWidget->tooltip());
		_refreshTextEntry			(m_pTable, row++, toString(pWidget->pointerStyle()));
		_refreshTextEntry			(m_pTable, row++, toString(pWidget->markPolicy()));
		_refreshBoolEntry			(m_pTable, row++, pWidget->isPickable());
		_refreshBoolEntry			(m_pTable, row++, pWidget->isPickHandle());
		_refreshIntegerEntry		(m_pTable, row++, pWidget->pickCategory());
		_refreshBoolEntry			(m_pTable, row++, pWidget->isDropTarget());
		_refreshBoolEntry			(m_pTable, row++, pWidget->isTabLocked());
		_refreshBoolEntry			(m_pTable, row++, pWidget->isSelectable());
		_refreshIntegerEntry		(m_pTable, row++, pWidget->m_receivingUpdateCounter);
		_refreshBoolEntry			(m_pTable, row++, pWidget->hasStickyFocus());
	}


} // namespace wg


