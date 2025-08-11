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

#ifndef	WG_SCROLLERINFOPANEL_DOT_H
#define WG_SCROLLERINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_scroller.h>

namespace wg
{
	class ScrollerInfoPanel;
	typedef	StrongPtr<ScrollerInfoPanel>	ScrollerInfoPanel_p;
	typedef	WeakPtr<ScrollerInfoPanel>	ScrollerInfoPanel_wp;



	class ScrollerInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static ScrollerInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, Scroller* pScroller) { return ScrollerInfoPanel_p(new ScrollerInfoPanel(blueprint, pHolder, pScroller)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ScrollerInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, Scroller* pScroller);
		~ScrollerInfoPanel() {}

		TablePanel_p	m_pTable;
		Scroller *		m_pInspected;

		Object_p		m_displayedBackgroundSkinPtr;
		Object_p		m_displayedBarSkinPtr;
		Object_p		m_displayedForwardButtonSkinPtr;
		Object_p		m_displayedBackwardButtonSkinPtr;
	};

} // namespace wg
#endif //WG_SCROLLERINFOPANEL_DOT_H

