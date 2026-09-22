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
#ifndef	WG_SCROLLPANELINFOSECTION_DOT_H
#define WG_SCROLLPANELINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_scrollpanel.h>

namespace wg
{
	class ScrollPanelInfoSection;
	typedef	StrongPtr<ScrollPanelInfoSection>	ScrollPanelInfoSection_p;
	typedef	WeakPtr<ScrollPanelInfoSection>	ScrollPanelInfoSection_wp;



	class ScrollPanelInfoSection : public TypedInfoSection<ScrollPanel>
	{
	public:

		//.____ Creation __________________________________________

		static ScrollPanelInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ScrollPanel * pInspected) { return ScrollPanelInfoSection_p(new ScrollPanelInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ScrollPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScrollPanel * pInspected );
		~ScrollPanelInfoSection() {}

		DrawerPanel_p	m_pScrollbarXDrawer;
		DrawerPanel_p	m_pScrollbarYDrawer;
		DrawerPanel_p	m_pSlotDrawer;
	};

} // namespace wg
#endif //WG_SCROLLPANELINFOSECTION_DOT_H
