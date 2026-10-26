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
#include "wg_widgetinfosection.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_enumextras.h>

namespace wg
{

	const TypeInfo WidgetInfoSection::TYPEINFO = { "WidgetInfoSection", &InfoSection::TYPEINFO };


	//____ slotAddressString() ___________________________________________________

	static String slotAddressString(StaticSlot* pSlot)
	{
		if( !pSlot )
			return String("null");

		char temp[64];
		snprintf(temp, sizeof(temp), "0x%p", (void*) pSlot);
		return String(temp);
	}


	//____ constructor _____________________________________________________________

	WidgetInfoSection::WidgetInfoSection(const DebugTheme& theme, IDebugContext* pContext, Widget * pWidget)
		: TypedInfoSection<Widget>( theme, pContext, Widget::TYPEINFO.className, pWidget )
	{
		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pPanel->slots << _createRows({
			intRow   ( "Id: ",                [](Widget* w) { return w->id(); } ),
			ptsRow   ( "Width (pts): ",       [](Widget* w) { return w->size().w; } ),
			ptsRow   ( "Height (pts): ",      [](Widget* w) { return w->size().h; } ),
			intRow   ( "Scale: ",             [](Widget* w) { return w->scale(); } ),
			textRow  ( "State: ",             [](Widget* w) { return toString(w->state().value()); } ),
			objectRow( "Baggage: ",           [](Widget* w) -> Object* { return w->baggage().rawPtr(); } ),
			objectRow( "Parent: ",            [](Widget* w) -> Object* { return w->parent().rawPtr(); } ),
			objectRow( "Skin: ",              [](Widget* w) -> Object* { return w->skin().rawPtr(); } ),
			textRow  ( "Tooltip: ",           [](Widget* w) { return w->tooltip(); } ),
			textRow  ( "Pointer style: ",     [](Widget* w) { return toString(w->pointerStyle()); } ),
			textRow  ( "Mark policy: ",       [](Widget* w) { return toString(w->markPolicy()); } ),
			boolRow  ( "Pickable: ",          [](Widget* w) { return w->isPickable(); } ),
			boolRow  ( "PickHandle: ",        [](Widget* w) { return w->isPickHandle(); } ),
			intRow   ( "Pick category: ",     [](Widget* w) { return w->pickCategory(); } ),
			boolRow  ( "DropTarget: ",        [](Widget* w) { return w->isDropTarget(); } ),
			boolRow  ( "TabLocked: ",         [](Widget* w) { return w->isTabLocked(); } ),
			boolRow  ( "Selectable: ",        [](Widget* w) { return w->isSelectable(); } ),
			intRow   ( "Receiving updates: ", [](Widget* w) { return w->m_receivingUpdateCounter; } ),
			boolRow  ( "Sticky focus: ",      [](Widget* w) { return w->hasStickyFocus(); } )
		});

		{
			m_pOverflowHeaderValue = WGCREATE(TextDisplay, _ = theme.listEntryText);

			m_pOverflowTable = _createTable(4,2);

			_initSpxEntry(m_pOverflowTable, 0, "Top (spx): ");
			_initSpxEntry(m_pOverflowTable, 1, "Right (spx): ");
			_initSpxEntry(m_pOverflowTable, 2, "Bottom (spx): ");
			_initSpxEntry(m_pOverflowTable, 3, "Left (spx): ");

			_refreshOverflow();

			pPanel->slots << _createDrawer("Has overflow", m_pOverflowHeaderValue, m_pOverflowTable);
		}

		{
			// The widget can be moved to another slot (or lose its slot) while we
			// are showing it, so the sections in here are rebuilt whenever the
			// slot pointer changes.

			m_pSlotInfoSectionsContainer = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y, _.spacingBefore = 4, _.spacingAfter = 4));
			m_pSlotHeaderValue = WGCREATE(TextDisplay, _ = theme.listEntryText, _.display.text = "null");

			_refreshSlotDrawer();

			pPanel->slots << _createDrawer("Slot", m_pSlotHeaderValue, m_pSlotInfoSectionsContainer);
		}

		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& WidgetInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void WidgetInfoSection::refresh()
	{
		TypedInfoSection<Widget>::refresh();
		_refreshOverflow();
		_refreshSlotDrawer();
	}

	//____ _refreshSlotDrawer() __________________________________________________

	void WidgetInfoSection::_refreshSlotDrawer()
	{
		auto pSlot = inspected()->_slot();

		// A new slot (or no slot at all) needs a new set of sections, since it
		// might not even be of the same slot class as the previous one.

		if( pSlot != m_pDisplayedSlot )
		{
			m_pDisplayedSlot = pSlot;
			m_pSlotHeaderValue->display.setText(slotAddressString(pSlot));

			m_pSlotInfoSectionsContainer->slots.clear();

			if( pSlot )
				m_pContext->addSlotInfoSections(m_pSlotInfoSectionsContainer, pSlot);

			return;										// Sections are up to date, they were just created.
		}

		for( auto& slot : m_pSlotInfoSectionsContainer->slots )
		{
			auto pInfoSection = dynamic_cast<InfoSection*>(slot._widget());
			if( pInfoSection )
			{
				pInfoSection->setInspectedSlot(pSlot);
				pInfoSection->refresh();
			}
		}
	}


	//____ _refreshOverflow() ____________________________________________________

	void WidgetInfoSection::_refreshOverflow()
	{
		auto pWidget = inspected();

		m_pOverflowHeaderValue->display.setText(pWidget->_hasOverflow() ? "true" : "false");

		BorderSPX overflow = pWidget->_overflow();

		_refreshSpxEntry(m_pOverflowTable, 0, overflow.top);
		_refreshSpxEntry(m_pOverflowTable, 1, overflow.right);
		_refreshSpxEntry(m_pOverflowTable, 2, overflow.bottom);
		_refreshSpxEntry(m_pOverflowTable, 3, overflow.left);
	}

} // namespace wg
