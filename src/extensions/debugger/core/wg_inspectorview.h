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
#ifndef	WG_INSPECTORVIEW_DOT_H
#define WG_INSPECTORVIEW_DOT_H
#pragma once

#include <wg_idebugcontext.h>
#include <wg_labelcapsule.h>
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_packpanel.h>


namespace wg
{
	class InspectorView;
	typedef	StrongPtr<InspectorView>	InspectorView_p;
	typedef	WeakPtr<InspectorView>		InspectorView_wp;



	class InspectorView : public Capsule
	{
	public:


		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _____________________________________________________

		void					setAutoRefresh(bool bAutoRefresh);

		virtual void			refresh();

		//.____ Content _______________________________________________________

		String					title() const { return m_title; };


	protected:
		InspectorView(const DebugTheme& theme, IDebugContext * pContext );
		~InspectorView() {}

		void				_update(int microPassed, int64_t microsecTimestamp) override;
		void				_refreshRecursively( Widget * pWidget );

		PackPanel_p			_createButtonRow( bool bAutoRefresh = false, bool bRefresh = false );

		Widget_p			_createObjectHeader(Object* pObject) const;
		String				_createObjectTitle(Object* pObject) const;

		Widget_p			_createClassInfoSections(const DebugTheme& bp, Object* pObject) const;


		IDebugContext*	m_pContext = nullptr;

		bool		m_bAutoRefresh = false;

		String		m_title;
	};



} // namespace wg
#endif //WG_INSPECTORVIEW_DOT_H

