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

#ifndef	WG_SCROLLPANELINFOPANEL_DOT_H
#define WG_SCROLLPANELINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_scrollpanel.h>

namespace wg
{
	class ScrollPanelInfoPanel;
	typedef	StrongPtr<ScrollPanelInfoPanel>	ScrollPanelInfoPanel_p;
	typedef	WeakPtr<ScrollPanelInfoPanel>	ScrollPanelInfoPanel_wp;



	class ScrollPanelInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static ScrollPanelInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, ScrollPanel * pPanel) { return ScrollPanelInfoPanel_p(new ScrollPanelInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ScrollPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, ScrollPanel * pPanel );
		~ScrollPanelInfoPanel() {}

		ScrollPanel *	m_pInspected;
		TablePanel_p	m_pTable;
		DrawerPanel_p	m_pScrollbarXDrawer;
		DrawerPanel_p	m_pScrollbarYDrawer;
		DrawerPanel_p	m_pSlotDrawer;
		Object_p		m_displayedTransitionPtr;

	};

} // namespace wg
#endif //WG_SCROLLPANELINFOPANEL_DOT_H

