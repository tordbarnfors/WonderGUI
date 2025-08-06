
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

#include <wg_idebugger.h>
#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_packpanel.h>
#include <wg_theme.h>


namespace wg
{
	class DebugPanel;
	typedef	StrongPtr<DebugPanel>	DebugPanel_p;
	typedef	WeakPtr<DebugPanel>		DebugPanel_wp;



	class DebugPanel : public LabelCapsule
	{
	public:

		using Blueprint = IDebugger::Blueprint;

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _____________________________________________________

		void					setAutoRefresh(bool bAutoRefresh);

		virtual void			refresh();					// For all InfoPanels except slots.
		virtual void			refresh(StaticSlot* pSlot);	// Specifically for slot-InfoPanel.


	protected:
		DebugPanel(const IDebugger::Blueprint& blueprint, IDebugger * pHolder, const char * pLabel );
		~DebugPanel() {}

		void				_update(int microPassed, int64_t microsecTimestamp) override;


		TablePanel_p		_createTable(int rows, int columns);
		DrawerPanel_p		_createDrawer(const CharSeq& label, Widget * pHeaderValue, Widget * pContent );

		DrawerPanel_p		_createBorderDrawer(const CharSeq& label, const Border& border);

		template<typename Iterator>
		DrawerPanel_p		_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd);

		template<typename Iterator>
		DrawerPanel_p		_refreshSlotsDrawer(DrawerPanel * pDrawer, Iterator slotsBegin, Iterator slotsEnd);

		DrawerPanel_p		_createComponentDrawer(const CharSeq& label, Component* pComponent);

		Widget_p			_createObjectHeader(Object* pObject);


		void _setTextEntry(TablePanel* pTable, int row, const char* pLabel, const CharSeq& string);
		void _setIntegerEntry(TablePanel * pTable, int row, const char * pLabel, int value);
		void _setDecimalEntry(TablePanel* pTable, int row, const char* pLabel, float value);
		void _setPtsEntry(TablePanel* pTable, int row, const char* pLabel, pts value);
		void _setSpxEntry(TablePanel* pTable, int row, const char* pLabel, spx value);
		void _setBoolEntry(TablePanel* pTable, int row, const char* pLabel, bool value);
		void _setPointerEntry(TablePanel* pTable, int row, const char* pLabel, void* pPointer);
		void _setObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel, Object * pPointer, Object * pSource);

		void _initTextEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initIntegerEntry(TablePanel * pTable, int row, const char * pLabel);
		void _initDecimalEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initPtsEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initSpxEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initBoolEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initPointerEntry(TablePanel* pTable, int row, const char* pLabel);
		void _initObjectPointerEntry(TablePanel* pTable, int row, const char* pLabel);

		void _refreshTextEntry(TablePanel* pTable, int row, const CharSeq& string);
		void _refreshIntegerEntry(TablePanel * pTable, int row, int value);
		void _refreshDecimalEntry(TablePanel* pTable, int row, float value);
		void _refreshPtsEntry(TablePanel* pTable, int row, pts value);
		void _refreshSpxEntry(TablePanel* pTable, int row, spx value);
		void _refreshBoolEntry(TablePanel* pTable, int row, bool value);
		void _refreshPointerEntry(TablePanel* pTable, int row, void* pPointer, void*& pDisplayedPointer );
		void _refreshObjectPointerEntry(TablePanel* pTable, int row, Object * pPointer, Object_p& pDisplayedPointer);

		IDebugger*	m_pHolder = nullptr;

		Skin_p		m_pIndentationSkin;

		bool		m_bAutoRefresh = false;
	};



	//____ createSlotsDrawer() ___________________________________________________

	template<typename Iterator>
	DrawerPanel_p DebugPanel::_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd)
	{
		auto pSlotList = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto bp = m_pHolder->blueprint();

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
				pSlotContent->slots << m_pHolder->createSlotInfoPanel(pTypeInfo, pSlot);
				pTypeInfo = pTypeInfo->pSuperClass;
			}

			auto pSlotDrawer = _createDrawer(buf, nullptr, pSlotContent);

			pSlotList->slots << pSlotDrawer;
			nbSlots++;
		}

		auto pNumberSlots = WGCREATE(NumberDisplay, _ = m_pHolder->blueprint().listEntryInteger);
		pNumberSlots->display.set(nbSlots);

		auto pDrawer = _createDrawer("Slots", pNumberSlots, pSlotList);
		return pDrawer;
	}

//____ refreshSlotsDrawer() ___________________________________________________

template<typename Iterator>
DrawerPanel_p DebugPanel::_refreshSlotsDrawer(DrawerPanel * pDrawer, Iterator slotsBegin, Iterator slotsEnd)
{
	auto pContainer = static_cast<PackPanel*>(pDrawer->slots[1]._widget());


	int nSlotsNow 		= std::distance(slotsBegin,slotsEnd);
	int nSlotsBefore 	= pContainer->slots.size();

	int slotsToRefresh = std::min(nSlotsNow,nSlotsBefore);

	Iterator it = slotsBegin;
	for( int i = 0 ; i < slotsToRefresh ; i++ )
	{
		
	}

}


} // namespace wg
#endif //WG_OBJECTINFOPANEL_DOT_H

