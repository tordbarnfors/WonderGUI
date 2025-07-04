
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

namespace wg
{

	const TypeInfo WidgetInfoPanel::TYPEINFO = { "WidgetInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	WidgetInfoPanel::WidgetInfoPanel(const Blueprint& blueprint, Widget * pWidget) : DebugPanel( blueprint )
	{
		auto pTable = TablePanel::create( WGOVR(blueprint.table, _.columns = 2, _.rows = 22 ));

		int row = 0;

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Id: ");

			auto pValue = NumberDisplay::create(blueprint.listEntryInteger);
			pValue->display.set(pWidget->id());

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}


		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Width (pts): ");

			auto pValue = NumberDisplay::create(blueprint.listEntryPts);
			pValue->display.set(pWidget->size().w);

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Height (pts): ");

			auto pValue = NumberDisplay::create(blueprint.listEntryPts);
			pValue->display.set(pWidget->size().h);

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Scale: ");

			auto pValue = NumberDisplay::create(blueprint.listEntryInteger);
			pValue->display.set(pWidget->scale());

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("State: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText( toString(pWidget->state().value() ));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}


		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Baggage: ");

			auto pValue = NumberDisplay::create(blueprint.listEntryPointer);
			pValue->display.set(pWidget->baggage());

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Skin: ");

			auto pValue = NumberDisplay::create(blueprint.listEntryPointer);
			pValue->display.set(int64_t(pWidget->skin().rawPtr()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Tooltip: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(pWidget->tooltip());

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Pointer style: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(toString(pWidget->pointerStyle()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Mark policy: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(toString(pWidget->markPolicy()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Is pickable: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->isPickable()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Is pickhandle: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->isPickHandle()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Pick category: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->pickCategory()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Is drop target: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->isDropTarget()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Is tab locked: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->isTabLocked()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Is selectable: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->isSelectable()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Receiving updates: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->m_receivingUpdateCounter));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Has sticky focus: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->hasStickyFocus()));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;
		}

		{
			auto pLabel = TextDisplay::create(blueprint.listEntryLabel);
			pLabel->display.setText("Has overflow: ");

			auto pValue = TextDisplay::create(blueprint.listEntryText);
			pValue->display.setText(std::to_string(pWidget->m_bOverflow));

			pTable->slots[row][0] = pLabel;
			pTable->slots[row][1] = pValue;
			row++;

			if( m_bOverflow )
			{
				BorderSPX overflow = pWidget->_overflow();

				pTable->slots[row][0] = TextDisplay::create( WGOVR( blueprint.listEntryLabel, _.display.text = "Overflow top (spx): " ));
				pTable->slots[row][0] = NumberDisplay::create( WGOVR( blueprint.listEntrySPX, _.display.value = overflow.top ));
				row++;

				pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow right (spx): "));
				pTable->slots[row][0] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.right));
				row++;

				pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow bottom (spx): "));
				pTable->slots[row][0] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.bottom));
				row++;

				pTable->slots[row][0] = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = "Overflow bottom (spx): "));
				pTable->slots[row][0] = NumberDisplay::create(WGOVR(blueprint.listEntrySPX, _.display.value = overflow.bottom));
				row++;
			}
		}





		this->slot = pTable;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& WidgetInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


