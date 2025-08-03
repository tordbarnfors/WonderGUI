
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

#ifndef	WG_DEBUGPANEL_DOT_H
#define WG_DEBUGPANEL_DOT_H
#pragma once

#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_theme.h>


namespace wg
{
	class DebugPanel;
	typedef	StrongPtr<DebugPanel>	DebugPanel_p;
	typedef	WeakPtr<DebugPanel>		DebugPanel_wp;



	class DebugPanel : public LabelCapsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Theme_p						theme;
			Surface_p					icons;

			LabelCapsule::Blueprint		mainCapsule;
			LabelCapsule::Blueprint		classCapsule;
			TextDisplay::Blueprint		listEntryLabel;
			TextDisplay::Blueprint		listEntryText;
			NumberDisplay::Blueprint	listEntryInteger;
			NumberDisplay::Blueprint	listEntryBool;
			NumberDisplay::Blueprint	listEntrySPX;
			NumberDisplay::Blueprint	listEntryPts;
			NumberDisplay::Blueprint	listEntryDecimal;
//			NumberDisplay::Blueprint	listEntryPointer;
			TextDisplay::Blueprint		textField;
			TextDisplay::Blueprint		infoDisplay;
			TablePanel::Blueprint		table;
			DrawerPanel::Blueprint		listEntryDrawer;
			PaddingCapsule::Blueprint	selectableListEntryCapsule;
		};


		class Holder
		{
		public:
			virtual void	objectSelected(Object * pSelected, Object * pCaller) = 0;

			virtual Widget_p createObjectInfoPanel(const DebugPanel::Blueprint& blueprint, const TypeInfo* pType, Object* pObject) = 0;
			virtual Widget_p createSlotInfoPanel(const DebugPanel::Blueprint& blueprint, const TypeInfo* pType, StaticSlot* pSlot) = 0;
			virtual Widget_p createComponentInfoPanel(const DebugPanel::Blueprint& blueprint, const TypeInfo* pType, Component* pComponent) = 0;
		};


		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		DebugPanel(const Blueprint& blueprint, DebugPanel::Holder * pHolder );
		~DebugPanel() {}

		TablePanel_p		_createTable(int rows, int columns);
		DrawerPanel_p		_createDrawer(const CharSeq& label, Widget * pHeaderValue, Widget * pContent );

		DrawerPanel_p		_createBorderDrawer(const CharSeq& label, const Border& border);

		template<typename Iterator>
		DrawerPanel_p		_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd);

		DrawerPanel_p		_createComponentDrawer(const CharSeq& label, Component* pComponent);


		void _setTextEntry(TablePanel* pTable, int row, const char* pLabel, const CharSeq& string);
		void _setIntegerEntry(TablePanel * pTable, int row, const char * pLabel, int value);
		void _setDecimalEntry(TablePanel* pTable, int row, const char* pLabel, float value);
		void _setPtsEntry(TablePanel* pTable, int row, const char* pLabel, pts value);
		void _setSpxEntry(TablePanel* pTable, int row, const char* pLabel, spx value);
		void _setBoolEntry(TablePanel* pTable, int row, const char* pLabel, bool value);
		void _setPointerEntry(TablePanel* pTable, int row, const char* pLabel, void* pPointer);
		void _setObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel, Object * pPointer, Object * pSource);

		Blueprint	m_blueprint;
		Holder*		m_pHolder = nullptr;

		Skin_p		m_pIndentationSkin;
	};



	//____ createSlotsDrawer() ___________________________________________________

	template<typename Iterator>
	DrawerPanel_p DebugPanel::_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd)
	{
		auto pSlotList = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto bp = m_blueprint;

		int nbSlots = 0;
		for (Iterator it = slotsBegin ; it != slotsEnd ; it++ )
		{
			char buf[32];
			sprintf(buf, "%d", nbSlots);

			auto pSlot = it;

			auto pSlotContent = WGCREATE(PackPanel, _.axis = Axis::Y);

			auto pTypeInfo = &it->typeInfo();

			while (pTypeInfo != nullptr)
			{
				bp.classCapsule.label.text = pTypeInfo->className;
				pSlotContent->slots << m_pHolder->createSlotInfoPanel(bp, pTypeInfo, pSlot);
				pTypeInfo = pTypeInfo->pSuperClass;
			}

			auto pSlotDrawer = _createDrawer(buf, nullptr, pSlotContent);

			pSlotList->slots << pSlotDrawer;
			nbSlots++;
		}

		auto pNumberSlots = WGCREATE(NumberDisplay, _ = m_blueprint.listEntryInteger);
		pNumberSlots->display.set(nbSlots);

		auto pDrawer = _createDrawer("Slots", pNumberSlots, pSlotList);
		return pDrawer;
	}


} // namespace wg
#endif //WG_OBJECTINFOPANEL_DOT_H

