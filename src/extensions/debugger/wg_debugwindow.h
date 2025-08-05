
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

#ifndef	WG_DEBUGWINDOW_DOT_H
#define WG_DEBUGWINDOW_DOT_H
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
	class DebugWindow;
	typedef	StrongPtr<DebugWindow>	DebugWindow_p;
	typedef	WeakPtr<DebugWindow>		DebugWindow_wp;



	class DebugWindow : public Capsule
	{
	public:

		using Blueprint = IDebugger::Blueprint;

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _____________________________________________________

		void					setAutoRefresh(bool bAutoRefresh);

		virtual void			refresh();


	protected:
		DebugWindow(const Blueprint& blueprint, IDebugger * pHolder );
		~DebugWindow() {}

		void				_update(int microPassed, int64_t microsecTimestamp) override;
		void				_refreshRecursively( Widget * pWidget );

		PackPanel_p			_createButtonRow( bool bAutoRefresh = false, bool bRefresh = false );

		TablePanel_p		_createTable(int rows, int columns);
		DrawerPanel_p		_createDrawer(const CharSeq& label, Widget * pHeaderValue, Widget * pContent );

		DrawerPanel_p		_createBorderDrawer(const CharSeq& label, const Border& border);

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



} // namespace wg
#endif //WG_DEBUGWINDOW_DOT_H

