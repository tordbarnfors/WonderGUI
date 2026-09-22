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
#ifndef	WG_SCROLLERINFOSECTION_DOT_H
#define WG_SCROLLERINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_scroller.h>

namespace wg
{
	class ScrollerInfoSection;
	typedef	StrongPtr<ScrollerInfoSection>	ScrollerInfoSection_p;
	typedef	WeakPtr<ScrollerInfoSection>	ScrollerInfoSection_wp;



	class ScrollerInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static ScrollerInfoSection_p		create(const DebugTheme& theme, IDebugContext* pContext, Scroller* pScroller) { return ScrollerInfoSection_p(new ScrollerInfoSection(theme, pContext, pScroller)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ScrollerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Scroller* pScroller);
		~ScrollerInfoSection() {}

		TablePanel_p	m_pTable;
		Scroller *		m_pInspected;

		Object_p		m_displayedBackgroundSkinPtr;
		Object_p		m_displayedBarSkinPtr;
		Object_p		m_displayedForwardButtonSkinPtr;
		Object_p		m_displayedBackwardButtonSkinPtr;
	};

} // namespace wg
#endif //WG_SCROLLERINFOSECTION_DOT_H

