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
#ifndef	WG_INFOSECTION_DOT_H
#define WG_INFOSECTION_DOT_H
#pragma once

#include <algorithm>

#include <wg_idebugcontext.h>
#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_packpanel.h>
#include <wg_msgrouter.h>


namespace wg
{
	class InfoSection;
	typedef	StrongPtr<InfoSection>	InfoSection_p;
	typedef	WeakPtr<InfoSection>		InfoSection_wp;



	class InfoSection : public LabelCapsule
	{
	public:


		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _____________________________________________________


		virtual void			refresh();

		// Slot sections are shown in a drawer that is reused when the slot list
		// changes, so the owner points them at the current slot before each
		// refresh(). Ignored by all other sections.

		virtual void			setInspectedSlot(StaticSlot* pSlot) {}


	protected:
		InfoSection(const DebugTheme& theme, IDebugContext * pContext, const char * pLabel );
		~InfoSection() {}


		TablePanel_p		_createTable(int rows, int columns);
		DrawerPanel_p		_createDrawer(const CharSeq& label, Widget * pHeaderValue, Widget * pContent );
		void				_setDrawerHeaderValue(DrawerPanel* pDrawer, Widget* pHeaderValue);

		DrawerPanel_p		_createRectDrawer(const CharSeq& label, const Rect& rect);
		void				_refreshRectDrawer(DrawerPanel* pRectDrawer, const Rect& rect, Rect& displayedRect);

		DrawerPanel_p		_createColorDrawer(const CharSeq& label, const HiColor& color);
		void				_refreshColorDrawer(DrawerPanel* pColorDrawer, const HiColor& color, HiColor& displayedColor );

		DrawerPanel_p		_createBorderDrawer(const CharSeq& label, const Border& border);
		void				_refreshBorderDrawer(DrawerPanel* pBorderDrawer, const Border& border, Border& displayedBorder );

		DrawerPanel_p		_createComponentDrawer(const CharSeq& label, Component* pComponent);
		void				_refreshComponentDrawer(DrawerPanel * pComponentDrawer );

		template<typename Iterator>
		DrawerPanel_p		_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd);
		template<typename Iterator>
		void				_refreshSlotsDrawer(DrawerPanel * pDrawer, Iterator slotsBegin, Iterator slotsEnd);
		template<typename Iterator>
		void				_addSlotInfoSections(PackPanel * pPanel, int numberingStart, Iterator slotsBegin, Iterator slotsEnd);

		DrawerPanel_p		_createSingleSlotDrawer(const CharSeq& label, StaticSlot * pSlot);
		void				_refreshSingleSlotDrawer(DrawerPanel* pDrawer, StaticSlot * pSlot);


		// Table rows are made in two steps: _init*Entry() puts the label and an
		// empty value widget in the row, _refresh*Entry() sets the value.
		// TypedInfoSection's row helpers are built on these.

		void _initEntry(TablePanel* pTable, int row, const char* pLabel, Widget* pValue);

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
		void _refreshObjectPointerEntry(TablePanel* pTable, int row, Object * pPointer, Object_p& pDisplayedPointer, RouteId& linkRoute);

		IDebugContext*	m_pContext = nullptr;

		Skin_p		m_pIndentationSkin;
	};

	//____ createSlotsDrawer() ___________________________________________________

	template<typename Iterator>
	DrawerPanel_p InfoSection::_createSlotsDrawer(const CharSeq& label, Iterator slotsBegin, Iterator slotsEnd)
	{
		auto pSlotList = WGCREATE(PackPanel, _.axis = Axis::Y);

		_addSlotInfoSections(pSlotList, 0, slotsBegin, slotsEnd);

		auto pNumberSlots = WGCREATE(NumberDisplay,
									 _ = m_pContext->theme().listEntryInteger,
									 _.display.value = std::distance(slotsBegin,slotsEnd ));

		auto pDrawer = _createDrawer(label, pNumberSlots, pSlotList);
		return pDrawer;
	}

//____ refreshSlotsDrawer() ___________________________________________________

template<typename Iterator>
void InfoSection::_refreshSlotsDrawer(DrawerPanel * pDrawer, Iterator slotsBegin, Iterator slotsEnd)
{
	auto pContainer = static_cast<PackPanel*>(pDrawer->slots[1]._widget());

	int nSlotsNow 		= (int) std::distance(slotsBegin,slotsEnd);
	int nSlotsBefore 	= pContainer->slots.size();

	int slotsToRefresh = nSlotsNow < nSlotsBefore ? nSlotsNow : nSlotsBefore;


	// Refresh existing SlotInfo panels.

	Iterator it = slotsBegin;
	for( int i = 0 ; i < slotsToRefresh ; i++ )
	{
		auto pSlotDrawer = dynamic_cast<DrawerPanel*>(pContainer->slots[i]._widget() );
		if( pSlotDrawer )
		{
			auto pInfoSectionContainer = dynamic_cast<PackPanel*>(pSlotDrawer->slots[1]._widget());

			for( auto& slot : pInfoSectionContainer->slots )
			{
				auto pInfoSection = dynamic_cast<InfoSection*>(slot._widget());
				if( pInfoSection )
				{
					pInfoSection->setInspectedSlot(it);
					pInfoSection->refresh();
				}
			}
		}

		it++;
	}

	// Add or remove panels if number of slots have changed

	if( nSlotsNow < nSlotsBefore )
		pContainer->slots.erase(nSlotsNow, nSlotsBefore - nSlotsNow );
	else if( nSlotsNow > nSlotsBefore )
		_addSlotInfoSections(pContainer, nSlotsBefore, it, slotsEnd);

	if( nSlotsNow != nSlotsBefore )
	{
		auto pNumberSlots = WGCREATE(NumberDisplay,
									 _ = m_pContext->theme().listEntryInteger,
									 _.display.value = nSlotsNow);
		_setDrawerHeaderValue(pDrawer, pNumberSlots);
	}

}

//____ addSlotInfoSections() ____________________________________________________

template<typename Iterator>
void InfoSection::_addSlotInfoSections(PackPanel * pPanel, int numberingStart, Iterator slotsBegin, Iterator slotsEnd)
{

	int slotNb = numberingStart;
	for (Iterator it = slotsBegin ; it != slotsEnd ; it++ )
	{
		char buf[16];
		snprintf(buf,16, "%d", slotNb++);

		auto pSlot = it;

		auto pSlotContent = WGCREATE(PackPanel, _.axis = Axis::Y);
		m_pContext->addSlotInfoSections(pSlotContent, pSlot);

		auto pSlotDrawer = _createDrawer(buf, nullptr, pSlotContent);

		pPanel->slots << pSlotDrawer;
	}
}




} // namespace wg
#endif //WG_INFOSECTION_DOT_H

