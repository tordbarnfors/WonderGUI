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
#include "wg_infosection.h"

#include <wg_colorskin.h>
#include <wg_twoslotpanel.h>
#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_packpanel.h>
#include <wg_textdisplay.h>

#include <cinttypes>

namespace wg
{

	const TypeInfo InfoSection::TYPEINFO = { "InfoSection", &LabelCapsule::TYPEINFO };


	//____ constructor _____________________________________________________________

	InfoSection::InfoSection(const DebugTheme& theme, IDebugContext * pContext, const char * pLabel )
		: LabelCapsule( theme.classCapsule )
		, m_pContext(pContext)
	{
		m_pIndentationSkin = ColorSkin::create(Color::Transparent, { 0,0,0,16 });

		label.setText(pLabel);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& InfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAutoRefresh() ______________________________________________________

	void InfoSection::setAutoRefresh(bool bAutoRefresh)
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

	void InfoSection::refresh()
	{
	}

	void InfoSection::refresh(StaticSlot * pSlot)
	{
	}

	//____ _update() _____________________________________________________________

	void InfoSection::_update(int microPassed, int64_t microsecTimestamp)
	{
		refresh();
	}

	//____ _createTable() ________________________________________________________

	TablePanel_p InfoSection::_createTable(int rows, int columns)
	{
		return WGCREATE(TablePanel, _ = m_pContext->theme().table, _.columns = columns, _.rows = rows, _.skin = m_pIndentationSkin);
	}

	//____ _createDrawer() ________________________________________________________

	DrawerPanel_p InfoSection::_createDrawer(const CharSeq& label, Widget* pHeaderValue, Widget* pContent)
	{
		auto pDrawer = WGCREATE(dbgkit::TreeListDrawer, _.skin = m_pIndentationSkin, _.buttonOfs.x -= 16);

		auto pHeaderPanel = WGCREATE(TwoSlotPanel, _.axis = Axis::X);
		pHeaderPanel->slots[0] = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryLabel, _.display.text = label);
		pHeaderPanel->slots[1] = pHeaderValue;

		pDrawer->slots[0] = pHeaderPanel;
		pDrawer->slots[1] = pContent;

		return pDrawer;
	}

	//____ _setDrawerHeaderValue() __________________________________________

	void InfoSection::_setDrawerHeaderValue(DrawerPanel* pDrawer, Widget* pHeaderValue)
	{
		auto pHeaderPanel = static_cast<TwoSlotPanel*>(pDrawer->slots[0]._widget());
		pHeaderPanel->slots[1] = pHeaderValue;
	}

	//____ _createColorDrawer() _________________________________________________

	DrawerPanel_p InfoSection::_createColorDrawer(const CharSeq& label, const HiColor& color)
	{
		bool bValid = color.isValid();
		bool bUndefined = color.isUndefined();

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = bValid ? "" : (bUndefined ? "undefined" : "invalid"));

		TablePanel_p pContentTable;

		if (true)
		{
			pContentTable = _createTable(4, 2);

			_setIntegerEntry(pContentTable, 0, "Red: ", color.r);
			_setIntegerEntry(pContentTable, 1, "Green: ", color.g);
			_setIntegerEntry(pContentTable, 2, "Blue: ",  color.b);
			_setIntegerEntry(pContentTable, 3, "Alpha: ", color.a);
		}

		return _createDrawer(label, pHeaderValue, pContentTable);
	}

	//____ _refreshColorDrawer() _____________________________________________

	void InfoSection::_refreshColorDrawer(DrawerPanel* pColorDrawer, const HiColor& color, HiColor& displayedColor )
	{
		if( color == displayedColor )
			return;

		bool bValid = color.isValid();
		bool bUndefined = color.isUndefined();

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = bValid ? "" : (bUndefined ? "undefined" : "invalid"));

		_setDrawerHeaderValue(pColorDrawer, pHeaderValue);

		auto pTable = static_cast<TablePanel*>(pColorDrawer->slots[1]._widget());

		_refreshIntegerEntry(pTable, 0, color.r);
		_refreshIntegerEntry(pTable, 1, color.g);
		_refreshIntegerEntry(pTable, 2, color.b);
		_refreshIntegerEntry(pTable, 3, color.a);

		displayedColor = color;
	}

	//____ _createRectDrawer() _________________________________________________

	DrawerPanel_p InfoSection::_createRectDrawer(const CharSeq& label, const Rect& rect)
	{
		bool bEmpty = rect.isEmpty();
		bool bValid = rect.isValid();

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = bValid ? (bEmpty ? "empty" : "") : "invalid" );

		TablePanel_p pContentTable;

		if (true)
		{
			pContentTable = _createTable(4, 2);

			_setPtsEntry(pContentTable, 0, "X (pts): ", rect.x);
			_setPtsEntry(pContentTable, 1, "Y (pts): ", rect.y);
			_setPtsEntry(pContentTable, 2, "W (pts): ", rect.w);
			_setPtsEntry(pContentTable, 3, "H (pts): ", rect.h);
		}

		return _createDrawer(label, pHeaderValue, pContentTable);
	}

	//____ _refreshRectDrawer() _____________________________________________

	void InfoSection::_refreshRectDrawer(DrawerPanel* pRectDrawer, const Rect& rect, Rect& displayedRect)
	{
		if (rect == displayedRect)
			return;

		bool bEmpty = rect.isEmpty();
		bool bValid = rect.isValid();

		if (bEmpty != displayedRect.isEmpty())
		{
			auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = bValid ? (bEmpty ? "empty" : "") : "invalid");
			_setDrawerHeaderValue(pRectDrawer, pHeaderValue);
		}

		auto pTable = static_cast<TablePanel*>(pRectDrawer->slots[1]._widget());

		_refreshPtsEntry(pTable, 0, rect.x);
		_refreshPtsEntry(pTable, 1, rect.y);
		_refreshPtsEntry(pTable, 2, rect.w);
		_refreshPtsEntry(pTable, 3, rect.h);

		displayedRect = rect;
	}


	//____ _createBorderDrawer() ________________________________________________________

	DrawerPanel_p InfoSection::_createBorderDrawer(const CharSeq& label, const Border& border)
	{
		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = border.isEmpty() ? "none" : "");

		TablePanel_p pContentTable;

		if (true)
		{
			pContentTable = _createTable(4, 2);

			_setPtsEntry(pContentTable, 0, "Top (pts): ", border.top);
			_setPtsEntry(pContentTable, 1, "Right (pts): ", border.right);
			_setPtsEntry(pContentTable, 2, "Bottom (pts): ", border.bottom);
			_setPtsEntry(pContentTable, 3, "Left (pts): ", border.left);
		}

		return _createDrawer(label, pHeaderValue, pContentTable);
	}

	//____ _refreshBorderDrawer() _____________________________________________

	void InfoSection::_refreshBorderDrawer(DrawerPanel* pBorderDrawer, const Border& border, Border& displayedBorder)
	{
		if( border == displayedBorder )
			return;

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_pContext->theme().listEntryText, _.display.text = border.isEmpty() ? "none" : "");
		_setDrawerHeaderValue(pBorderDrawer, pHeaderValue);

		auto pTable = static_cast<TablePanel*>(pBorderDrawer->slots[1]._widget());

		_refreshPtsEntry(pTable, 0, border.top);
		_refreshPtsEntry(pTable, 1, border.right);
		_refreshPtsEntry(pTable, 2, border.bottom);
		_refreshPtsEntry(pTable, 3, border.left);

		displayedBorder = border;
	}


	//____ _createComponentDrawer() _________________________________________________

	DrawerPanel_p InfoSection::_createComponentDrawer(const CharSeq& label, Component* pComponent)
	{
		auto pComponentParts = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTypeInfo = &pComponent->typeInfo();

		while (pTypeInfo != nullptr)
		{
			auto pInfoSection = m_pContext->createComponentInfoSection(pTypeInfo, pComponent);

			if (pInfoSection)
				pComponentParts->slots << pInfoSection;

			pTypeInfo = pTypeInfo->pSuperClass;
		}

		auto pDrawer = _createDrawer(label, nullptr, pComponentParts);
		return pDrawer;
	}

	//____ _refreshComponentDrawer() _____________________________________________

	void InfoSection::_refreshComponentDrawer(DrawerPanel * pComponentDrawer)
	{
		auto pPanel = static_cast<PackPanel*>(pComponentDrawer->slots[1]._widget());

		for( auto& slot : pPanel->slots )
		{
			auto pInfoSection = dynamic_cast<InfoSection*>(slot._widget());
			if( pInfoSection )
				pInfoSection->refresh();
		}
	}

	//____ _createObjectHeader() ______________________________________________

	Widget_p InfoSection::_createObjectHeader(Object* pObject)
	{

		auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
			_.display.text = pObject->typeInfo().className,
			_.display.style = dbgkit::TextStyles::Heading5
		));

		char temp[64];
		std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pObject));

		CharBuffer buf(64);
		buf.pushBack(temp);
		buf.setStyle(dbgkit::TextStyles::Default);
		
		pDisplay->display.append(&buf);
		return pDisplay;
	}


	//___ _setTextEntry() _________________________________________________

	void InfoSection::_setTextEntry(TablePanel* pTable, int row, const char* pLabel, const CharSeq& string)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = string));
	}

	//___ _setIntegerEntry() _________________________________________________

	void InfoSection::_setIntegerEntry(TablePanel* pTable, int row, const char* pLabel, int value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pContext->theme().listEntryInteger, _.display.value = value));
	}

	//___ _setDecimalEntry() _________________________________________________

	void InfoSection::_setDecimalEntry(TablePanel* pTable, int row, const char* pLabel, float value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pContext->theme().listEntryDecimal, _.display.value = value));
	}

	//___ _setPtsEntry() _________________________________________________

	void InfoSection::_setPtsEntry(TablePanel* pTable, int row, const char* pLabel, pts value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pContext->theme().listEntryPts, _.display.value = value));
	}

	//___ _setSpxEntry() _________________________________________________

	void InfoSection::_setSpxEntry(TablePanel* pTable, int row, const char* pLabel, spx value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_pContext->theme().listEntrySPX, _.display.value = value));
	}

	//___ _setBoolEntry() _________________________________________________

	void InfoSection::_setBoolEntry(TablePanel* pTable, int row, const char* pLabel, bool value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = value ? "true" : "false"));
	}

	//___ _setPointerEntry() _________________________________________________

	void InfoSection::_setPointerEntry(TablePanel* pTable, int row, const char* pLabel, void* pPointer)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		char temp[32] = "null";
		if( pPointer )
			std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pPointer));

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = temp));
	}

	//___ _setObjectPointerEntry() _________________________________________________

	void InfoSection::_setObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel, Object* pPointer, Object * pSource)
	{
		if( row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;


		CharBuffer	buff(128);

		if (pPointer)
		{
			buff.pushBack(pPointer->typeInfo().className);
			buff.setStyle(dbgkit::TextStyles::FinePrint, 0, 1000);

			int ofs = buff.nbChars();

			char temp[32];
			if(pPointer)
				std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pPointer));
			buff.pushBack(temp);

			TextLink_p 	pLink = TextLink::create();
			IDebugContext*		pContext = m_pContext;

			Base::msgRouter()->addRoute(pLink, MsgType::MouseClick, [pPointer, pContext](Msg* pMsg) {
				pContext->objectSelected(pPointer, nullptr);
			});

			TextStyle_p pStyle = WGCREATE(TextStyle, _.link = pLink, _.color = Color::DarkRed, _.decoration = TextDecoration::Underline);

			buff.setStyle(pStyle, ofs, 1000);
		}
		else
			buff.pushBack("null");

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel ));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = &buff, _.markPolicy = MarkPolicy::Geometry));
	}




	//___ _initTextEntry() _________________________________________________

	void InfoSection::_initTextEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(m_pContext->theme().listEntryText);
	}

	//___ _initIntegerEntry() _________________________________________________

	void InfoSection::_initIntegerEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(m_pContext->theme().listEntryInteger);
	}

	//___ _initDecimalEntry() _________________________________________________

	void InfoSection::_initDecimalEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(m_pContext->theme().listEntryDecimal);
	}

	//___ _initPtsEntry() _________________________________________________

	void InfoSection::_initPtsEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(m_pContext->theme().listEntryPts);
	}

	//___ _initSpxEntry() _________________________________________________

	void InfoSection::_initSpxEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(m_pContext->theme().listEntrySPX);
	}

	//___ _initBoolEntry() _________________________________________________

	void InfoSection::_initBoolEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = "false" ));
	}

	//___ _initPointerEntry() _________________________________________________

	void InfoSection::_initPointerEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = "null" ));
	}

	//___ _initObjectPointerEntry() _________________________________________________

	void InfoSection::_initObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel)
	{
		if( row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryLabel, _.display.text = pLabel ));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_pContext->theme().listEntryText, _.display.text = "null", _.markPolicy = MarkPolicy::Geometry));
	}

	//____ _refreshTextEntry() ___________________________________________________

	void InfoSection::_refreshTextEntry(TablePanel* pTable, int row, const CharSeq& string)
	{
		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(string);
	}

	//____ _refreshIntegerEntry() ________________________________________________

	void InfoSection::_refreshIntegerEntry(TablePanel * pTable, int row, int value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshDecimalEntry() ________________________________________________

	void InfoSection::_refreshDecimalEntry(TablePanel* pTable, int row, float value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshPtsEntry() ____________________________________________________

	void InfoSection::_refreshPtsEntry(TablePanel* pTable, int row, pts value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshSpxEntry() ____________________________________________________

	void InfoSection::_refreshSpxEntry(TablePanel* pTable, int row, spx value)
	{
		static_cast<NumberDisplay*>(pTable->slots[row][1]._widget())->display.set(value);
	}

	//____ _refreshBoolEntry() ___________________________________________________

	void InfoSection::_refreshBoolEntry(TablePanel* pTable, int row, bool value)
	{
		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(value? "true" : "false" );
	}

	//____ _refreshPointerEntry() ________________________________________________

	void InfoSection::_refreshPointerEntry(TablePanel* pTable, int row, void* pPointer, void*& pSavedPointer)
	{
		if( pPointer == pSavedPointer )
			return;

		pSavedPointer = pPointer;

		char temp[32] = "null";
		if( pPointer )
			std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pPointer));

		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(temp);
	}

	//____ _refreshObjectPointerEntry() __________________________________________

	void InfoSection::_refreshObjectPointerEntry(TablePanel* pTable, int row, Object * pPointer, Object_p& pSavedPointer)
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
			buff.setStyle(dbgkit::TextStyles::FinePrint, 0, 1000);

			int ofs = buff.nbChars();

			char temp[32];
			if(pPointer)
				std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pPointer));
			buff.pushBack(temp);

			TextLink_p 	pLink = TextLink::create();
			IDebugContext*	pContext = m_pContext;

			Base::msgRouter()->addRoute(pLink, MsgType::MouseClick, [pPointer, pContext](Msg* pMsg) {
				pContext->objectSelected(pPointer, nullptr);
			});

			TextStyle_p pStyle = WGCREATE(TextStyle, _.link = pLink, _.color = Color::DarkRed, _.decoration = TextDecoration::Underline);

			buff.setStyle(pStyle, ofs, 1000);
		}
		else
			buff.pushBack("null");

		static_cast<TextDisplay*>(pTable->slots[row][1]._widget())->display.setText(&buff);
	}


	//____ createSingleSlotDrawer() ___________________________________________________

	DrawerPanel_p InfoSection::_createSingleSlotDrawer(const CharSeq& label, StaticSlot* pSlot)
	{
		auto pSlotContent = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTypeInfo = &pSlot->typeInfo();

		while (pTypeInfo != nullptr)
		{
			auto pInfoSection = m_pContext->createSlotInfoSection(pTypeInfo, pSlot);
			if (pInfoSection)
				pSlotContent->slots << pInfoSection;
			pTypeInfo = pTypeInfo->pSuperClass;
		}

		return _createDrawer("Slot", nullptr, pSlotContent);
	}

	//____ refreshSingleSlotDrawer() ___________________________________________________

	void InfoSection::_refreshSingleSlotDrawer(DrawerPanel* pDrawer, StaticSlot* pSlot)
	{
		auto pContainer = static_cast<PackPanel*>(pDrawer->slots[1]._widget());
		for (auto& slot : pContainer->slots)
		{
			auto pInfoSection = dynamic_cast<InfoSection*>(slot._widget());
			if (pInfoSection)
				pInfoSection->refresh(pSlot);
		}
	}


} // namespace wg



