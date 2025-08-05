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
#include "wg_debugwindow.h"
#include "wg_debugpanel.h"

#include <wg_colorskin.h>
#include <wg_twoslotpanel.h>
#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_packpanel.h>
#include <wg_textdisplay.h>
#include <wg_blockskin.h>



namespace wg
{

	const TypeInfo DebugWindow::TYPEINFO = { "DebugWindow", &LabelCapsule::TYPEINFO };


	//____ constructor _____________________________________________________________

	DebugWindow::DebugWindow(const Blueprint& blueprint, IDebugger * pHolder )
		: Capsule( blueprint.mainCapsule )
		, m_pHolder(pHolder)
	{
		m_pIndentationSkin = ColorSkin::create(Color::Transparent, { 0,0,0,16 });

		setSkin(nullptr);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugWindow::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAutoRefresh() ______________________________________________________

	void DebugWindow::setAutoRefresh(bool bAutoRefresh)
	{
		if( bAutoRefresh != m_bAutoRefresh )
		{
			if( bAutoRefresh )
				_startReceiveUpdates();
			else
				_stopReceiveUpdates();

			m_bAutoRefresh = bAutoRefresh;
		}
	}

	//____ refresh() _____________________________________________________________

	void DebugWindow::refresh()
{
		auto pPanel = static_cast<PackPanel*>(slot._widget());

		for( auto& slot : pPanel->slots )
		{
			auto pDebugPanel = dynamic_cast<DebugPanel*>(slot._widget());

			if( pDebugPanel )
				pDebugPanel->refresh();
		}
	}

	//____ _refreshRecursively() _________________________________________________

	void DebugWindow::_refreshRecursively( Widget * pWidget )
	{
		auto pDebugPanel = dynamic_cast<DebugPanel*>(pWidget);
		if( pDebugPanel )
		{
			pDebugPanel->refresh();
			return;
		}

		auto pContainer = dynamic_cast<Container*>(pWidget);
		if( pContainer )
		{
			auto pChild = pContainer->firstChild();
			while( pChild )
			{
				_refreshRecursively(pChild);
				pChild = pChild->nextSibling();
			}
		}
	}

	//____ _update() _____________________________________________________________

	void DebugWindow::_update(int microPassed, int64_t microsecTimestamp)
	{
		refresh();
	}

	//____ _createButtonRow() ____________________________________________________

	PackPanel_p DebugWindow::_createButtonRow( bool bAutoRefresh, bool bRefresh )
	{
		auto pPackLayout = PackLayout::create({});

		auto& bp = m_pHolder->blueprint();

		auto pButtonRow = PackPanel::create(WGBP(PackPanel,
		  _.axis = Axis::X,
		  _.layout = pPackLayout,
		  _.skin = bp.theme->plateSkin()));

		auto pAutoRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			  _.surface = bp.icons,
			  _.firstBlock = { 48,48,16,16 } ));

		auto pRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			  _.surface = bp.icons,
			  _.firstBlock = { 0,0,16,16 } ));

		auto pAutoRefreshButton = ToggleButton::create(WGOVR(bp.theme->toggleButton(), _.icon.skin = pAutoRefreshIcon));
		auto pRefreshButton = Button::create(WGOVR(bp.theme->pushButton(), _.icon.skin = pRefreshIcon));

		Base::msgRouter()->addRoute(pAutoRefreshButton, MsgType::Toggle, [this](Msg* _pMsg) {

			auto pMsg = static_cast<ToggleMsg*>(_pMsg);
			setAutoRefresh(pMsg->isChecked());
		});

		Base::msgRouter()->addRoute(pRefreshButton, MsgType::Select, [this](Msg* _pMsg) {
			refresh();
		});


		pButtonRow->slots.pushBack( { pAutoRefreshButton, pRefreshButton } );

		return pButtonRow;
	}



	//____ _createTable() ________________________________________________________

	TablePanel_p DebugWindow::_createTable(int rows, int columns)
	{
		return WGCREATE(TablePanel, _ = m_pHolder->blueprint().table, _.columns = columns, _.rows = rows, _.skin = m_pIndentationSkin);
	}

	//____ _createDrawer() ________________________________________________________

	DrawerPanel_p DebugWindow::_createDrawer(const CharSeq& label, Widget* pHeaderValue, Widget* pContent)
	{
		auto pDrawer = WGCREATE(DrawerPanel, _ = m_pHolder->blueprint().theme->treeListDrawer(), _.skin = m_pIndentationSkin, _.buttonOfs.x -= 16);

		auto pHeaderPanel = WGCREATE(TwoSlotPanel, _.axis = Axis::X);
		pHeaderPanel->slots[0] = WGCREATE(TextDisplay, _ = m_pHolder->blueprint().listEntryLabel, _.display.text = label);
		pHeaderPanel->slots[1] = pHeaderValue;

		pDrawer->slots[0] = pHeaderPanel;
		pDrawer->slots[1] = pContent;

		return pDrawer;
	}

	//____ _createBorderDrawer() ________________________________________________________

	DrawerPanel_p DebugWindow::_createBorderDrawer(const CharSeq& label, const Border& border)
	{
		bool bEmpty = border.isEmpty();

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pHolder->blueprint().listEntryText, _.display.text = bEmpty ? "none" : "");

		TablePanel_p pContentTable;

		if (true)
		{
			pContentTable = _createTable(4, 2);

			_setSpxEntry(pContentTable, 0, "Top (pts): ", border.top);
			_setSpxEntry(pContentTable, 1, "Right (pts): ", border.right);
			_setSpxEntry(pContentTable, 2, "Bottom (pts): ", border.bottom);
			_setSpxEntry(pContentTable, 3, "Left (pts): ", border.left);
		}

		return _createDrawer(label, pHeaderValue, pContentTable);
	}

	//____ _createComponentDrawer() _________________________________________________

	DrawerPanel_p DebugWindow::_createComponentDrawer(const CharSeq& label, Component* pComponent)
	{
		auto bp = m_pHolder->blueprint();

		auto pComponentParts = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTypeInfo = &pComponent->typeInfo();

		while (pTypeInfo != nullptr)
		{
			bp.classCapsule.label.text = pTypeInfo->className;
			pComponentParts->slots << m_pHolder->createComponentInfoPanel(pTypeInfo, pComponent);
			pTypeInfo = pTypeInfo->pSuperClass;
		}

		auto pDrawer = _createDrawer(label, nullptr, pComponentParts);
		return pDrawer;
	}

	//____ _createObjectHeader() ______________________________________________

	Widget_p DebugWindow::_createObjectHeader(Object* pObject)
	{

		auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
			_.display.text = pObject->typeInfo().className,
			_.display.style = m_pHolder->blueprint().theme->heading5Style()
		));

		char temp[64];
		sprintf(temp, " 0x%x", reinterpret_cast<std::uintptr_t>(pObject));

		CharBuffer buf(64);
		buf.pushBack(temp);
		buf.setStyle(m_pHolder->blueprint().theme->defaultStyle());
		
		pDisplay->display.append(&buf);
		return pDisplay;
	}


	//___ _setTextEntry() _________________________________________________

	void DebugWindow::_setTextEntry(TablePanel* pTable, int row, const char* pLabel, const CharSeq& string)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryText, _.display.text = string));
	}

	//___ _setIntegerEntry() _________________________________________________

	void DebugWindow::_setIntegerEntry(TablePanel* pTable, int row, const char* pLabel, int value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pHolder->blueprint().listEntryInteger, _.display.value = value));
	}

	//___ _setDecimalEntry() _________________________________________________

	void DebugWindow::_setDecimalEntry(TablePanel* pTable, int row, const char* pLabel, float value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pHolder->blueprint().listEntryDecimal, _.display.value = value));
	}

	//___ _setPtsEntry() _________________________________________________

	void DebugWindow::_setPtsEntry(TablePanel* pTable, int row, const char* pLabel, pts value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pHolder->blueprint().listEntryPts, _.display.value = value));
	}

	//___ _setSpxEntry() _________________________________________________

	void DebugWindow::_setSpxEntry(TablePanel* pTable, int row, const char* pLabel, spx value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pHolder->blueprint().listEntrySPX, _.display.value = value));
	}

	//___ _setBoolEntry() _________________________________________________

	void DebugWindow::_setBoolEntry(TablePanel* pTable, int row, const char* pLabel, bool value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryText, _.display.text = value ? "true" : "false"));
	}

	//___ _setPointerEntry() _________________________________________________

	void DebugWindow::_setPointerEntry(TablePanel* pTable, int row, const char* pLabel, void* pPointer)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		char temp[32] = "null";
		if( pPointer )
			sprintf(temp, " 0x%x", reinterpret_cast<std::uintptr_t>(pPointer));

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryText, _.display.text = temp));
	}

	//___ _setObjectPointerEntry() _________________________________________________

	void DebugWindow::_setObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel, Object* pPointer, Object * pSource)
	{
		if( row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;


		CharBuffer	buff(128);

		if (pPointer)
		{
			buff.pushBack(pPointer->typeInfo().className);
			buff.setStyle(m_pHolder->blueprint().theme->finePrintStyle(), 0, 1000);

			int ofs = buff.nbChars();

			char temp[32];
			if(pPointer)
				sprintf(temp, " 0x%x", reinterpret_cast<std::uintptr_t>(pPointer));
			buff.pushBack(temp);

			TextLink_p 	pLink = TextLink::create();
			IDebugger*	pHolder = m_pHolder;

			Base::msgRouter()->addRoute(pLink, MsgType::MouseClick, [pPointer, pHolder](Msg* pMsg) {
				pHolder->objectSelected(pPointer, nullptr);
			});

			TextStyle_p pStyle = WGCREATE(TextStyle, _.link = pLink, _.color = Color::DarkRed, _.decoration = TextDecoration::Underline);

			buff.setStyle(pStyle, ofs, 1000);
		}
		else
			buff.pushBack("null");

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryLabel, _.display.text = pLabel ));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pHolder->blueprint().listEntryText, _.display.text = &buff, _.markPolicy = MarkPolicy::Geometry));
	}

	//____ _refreshTextEntry() ___________________________________________________

	void DebugWindow::_refreshTextEntry(TablePanel* pTable, int row, const CharSeq& string)
	{
		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(string);
	}

	//____ _refreshIntegerEntry() ________________________________________________

	void DebugWindow::_refreshIntegerEntry(TablePanel * pTable, int row, int value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshDecimalEntry() ________________________________________________

	void DebugWindow::_refreshDecimalEntry(TablePanel* pTable, int row, float value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshPtsEntry() ____________________________________________________

	void DebugWindow::_refreshPtsEntry(TablePanel* pTable, int row, pts value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshSpxEntry() ____________________________________________________

	void DebugWindow::_refreshSpxEntry(TablePanel* pTable, int row, spx value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshBoolEntry() ___________________________________________________

	void DebugWindow::_refreshBoolEntry(TablePanel* pTable, int row, bool value)
	{
		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(value? "true" : "false" );
	}

	//____ _refreshPointerEntry() ________________________________________________

	void DebugWindow::_refreshPointerEntry(TablePanel* pTable, int row, void* pPointer, void*& pSavedPointer)
	{
		if( pPointer == pSavedPointer )
			return;

		pSavedPointer = pPointer;

		char temp[32] = "null";
		if( pPointer )
			sprintf(temp, " 0x%x", reinterpret_cast<std::uintptr_t>(pPointer));

		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(temp);
	}

	//____ _refreshObjectPointerEntry() __________________________________________

	void DebugWindow::_refreshObjectPointerEntry(TablePanel* pTable, int row, Object * pPointer, Object_p& pSavedPointer)
	{
		if( pPointer == pSavedPointer )
			return;

		if( pSavedPointer )
		{
			//TODO: We need to remove the route somehow.
		}

		pSavedPointer = pPointer;

		CharBuffer	buff(128);

		if (pPointer)
		{
			buff.pushBack(pPointer->typeInfo().className);
			buff.setStyle(m_pHolder->blueprint().theme->finePrintStyle(), 0, 1000);

			int ofs = buff.nbChars();

			char temp[32];
			if(pPointer)
				sprintf(temp, " 0x%x", reinterpret_cast<std::uintptr_t>(pPointer));
			buff.pushBack(temp);

			TextLink_p 	pLink = TextLink::create();
			IDebugger*	pHolder = m_pHolder;

			Base::msgRouter()->addRoute(pLink, MsgType::MouseClick, [pPointer, pHolder](Msg* pMsg) {
				pHolder->objectSelected(pPointer, nullptr);
			});

			TextStyle_p pStyle = WGCREATE(TextStyle, _.link = pLink, _.color = Color::DarkRed, _.decoration = TextDecoration::Underline);

			buff.setStyle(pStyle, ofs, 1000);
		}
		else
			buff.pushBack("null");

		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(&buff);
	}



} // namespace wg



