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
#include "wg_debugpanel.h"

#include <wg_colorskin.h>
#include <wg_twoslotpanel.h>
#include <wg_msgrouter.h>
#include <wg_msg.h>



namespace wg
{

	const TypeInfo DebugPanel::TYPEINFO = { "DebugPanel", &LabelCapsule::TYPEINFO };


	//____ constructor _____________________________________________________________

	DebugPanel::DebugPanel(const Blueprint& blueprint, DebugPanel::Holder * pHolder ) 
		: LabelCapsule( blueprint.classCapsule )
		, m_pHolder(pHolder)
	{
		m_blueprint = blueprint;

		m_pIndentationSkin = ColorSkin::create(Color::Transparent, { 0,0,0,16 });
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _createTable() ________________________________________________________

	TablePanel_p DebugPanel::_createTable(int rows, int columns)
	{
		return WGCREATE(TablePanel, _ = m_blueprint.table, _.columns = columns, _.rows = rows, _.skin = m_pIndentationSkin);
	}

	//____ _createDrawer() ________________________________________________________

	DrawerPanel_p DebugPanel::_createDrawer(const CharSeq& label, Widget* pHeaderValue, Widget* pContent)
	{
		auto pDrawer = WGCREATE(DrawerPanel, _ = m_blueprint.theme->treeListDrawer(), _.skin = m_pIndentationSkin, _.buttonOfs.x -= 16);

		auto pHeaderPanel = WGCREATE(TwoSlotPanel, _.axis = Axis::X);
		pHeaderPanel->slots[0] = WGCREATE(TextDisplay, _ = m_blueprint.listEntryLabel, _.display.text = label);
		pHeaderPanel->slots[1] = pHeaderValue;

		pDrawer->slots[0] = pHeaderPanel;
		pDrawer->slots[1] = pContent;

		return pDrawer;
	}

	//____ _createBorderDrawer() ________________________________________________________

	DrawerPanel_p DebugPanel::_createBorderDrawer(const CharSeq& label, const Border& border)
	{
		bool bEmpty = border.isEmpty();

		auto pHeaderValue = WGCREATE(TextDisplay, _ = m_blueprint.listEntryText, _.display.text = bEmpty ? "none" : "");

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



	//___ _setTextEntry() _________________________________________________

	void DebugPanel::_setTextEntry(TablePanel* pTable, int row, const char* pLabel, const CharSeq& string)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_blueprint.listEntryText, _.display.text = string));
	}

	//___ _setIntegerEntry() _________________________________________________

	void DebugPanel::_setIntegerEntry(TablePanel* pTable, int row, const char* pLabel, int value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_blueprint.listEntryInteger, _.display.value = value));
	}

	//___ _setDecimalEntry() _________________________________________________

	void DebugPanel::_setDecimalEntry(TablePanel* pTable, int row, const char* pLabel, float value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_blueprint.listEntryDecimal, _.display.value = value));
	}

	//___ _setPtsEntry() _________________________________________________

	void DebugPanel::_setPtsEntry(TablePanel* pTable, int row, const char* pLabel, pts value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_blueprint.listEntryPts, _.display.value = value));
	}

	//___ _setSpxEntry() _________________________________________________

	void DebugPanel::_setSpxEntry(TablePanel* pTable, int row, const char* pLabel, spx value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = NumberDisplay::create(WGOVR(m_blueprint.listEntrySPX, _.display.value = value));
	}

	//___ _setBoolEntry() _________________________________________________

	void DebugPanel::_setBoolEntry(TablePanel* pTable, int row, const char* pLabel, bool value)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_blueprint.listEntryText, _.display.text = value ? "true" : "false"));
	}

	//___ _setPointerEntry() _________________________________________________

	void DebugPanel::_setPointerEntry(TablePanel* pTable, int row, const char* pLabel, void* pPointer)
	{
		if (row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;

		char temp[64];
		sprintf(temp, "0x%p", pPointer);

		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_blueprint.listEntryText, _.display.text = temp));
	}

	//___ _setObjectPointerEntry() _________________________________________________

	void DebugPanel::_setObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel, Object* pPointer, Object * pSource)
	{
		if( row < 0 || row >= pTable->rows.size())
			return;

		if (pTable->columns.size() < 2)
			return;


		CharBuffer	buff(128);

		if (pPointer)
		{
			buff.pushBack(pPointer->typeInfo().className);
			buff.setStyle(m_blueprint.theme->finePrintStyle(), 0, 1000);

			int ofs = buff.nbChars();

			char temp[64];
			sprintf(temp, " 0x%p", pPointer);
			buff.pushBack(temp);

			TextLink_p pLink = TextLink::create();

			Object_p	safePointer = pPointer;
			Object_p	safeSource = pSource;
			Holder*		pHolder = m_pHolder;

			Base::msgRouter()->addRoute(pLink, MsgType::MouseClick, [safePointer, safeSource, pHolder](Msg* pMsg) {
				
				pHolder->objectSelected(safePointer, safeSource);
			});


			TextStyle_p pStyle = WGCREATE(TextStyle, _.link = pLink, _.color = Color::DarkRed, _.decoration = TextDecoration::Underline);

			buff.setStyle(pStyle, ofs, 1000);

		}
		else
			buff.pushBack("null");




		pTable->slots[row][0] = TextDisplay::create(WGOVR(m_blueprint.listEntryLabel, _.display.text = pLabel ));
		pTable->slots[row][1] = TextDisplay::create(WGOVR(m_blueprint.listEntryText, _.display.text = &buff, _.markPolicy = MarkPolicy::Geometry));
	}




} // namespace wg



