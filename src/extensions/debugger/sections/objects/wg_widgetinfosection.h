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
#ifndef	WG_WIDGETINFOSECTION_DOT_H
#define WG_WIDGETINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>

namespace wg
{
	class WidgetInfoSection;
	typedef	StrongPtr<WidgetInfoSection>	WidgetInfoSection_p;
	typedef	WeakPtr<WidgetInfoSection>	WidgetInfoSection_wp;



	class WidgetInfoSection : public TypedInfoSection<Widget>
	{
	public:

		//.____ Creation __________________________________________

		static WidgetInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Widget * pWidget) { return WidgetInfoSection_p(new WidgetInfoSection(theme, pContext, pWidget) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		WidgetInfoSection(const DebugTheme& theme, IDebugContext* pContext, Widget * pWidget );
		~WidgetInfoSection() {}

		void			_refreshOverflow();
		void			_refreshSlotDrawer();

		TextDisplay_p	m_pOverflowHeaderValue;
		TablePanel_p	m_pOverflowTable;

		TextDisplay_p	m_pSlotHeaderValue;
		PackPanel_p		m_pSlotInfoSectionsContainer;
		StaticSlot *	m_pDisplayedSlot = nullptr;
	};

} // namespace wg
#endif //WG_WIDGETINFOSECTION_DOT_H
