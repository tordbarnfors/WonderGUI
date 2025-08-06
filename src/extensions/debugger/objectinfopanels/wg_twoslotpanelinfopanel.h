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

#ifndef	WG_TWOSLOTINFOPANEL_DOT_H
#define WG_TWOSLOTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_twoslotpanel.h>

namespace wg
{
	class TwoSlotPanelInfoPanel;
	typedef	StrongPtr<TwoSlotPanelInfoPanel>	TwoSlotPanelInfoPanel_p;
	typedef	WeakPtr<TwoSlotPanelInfoPanel>	TwoSlotPanelInfoPanel_wp;



	class TwoSlotPanelInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static TwoSlotPanelInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, TwoSlotPanel * pPanel) { return TwoSlotPanelInfoPanel_p(new TwoSlotPanelInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void 					refresh() override;


	protected:
		TwoSlotPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, TwoSlotPanel * pPanel );
		~TwoSlotPanelInfoPanel() {}

		TwoSlotPanel *	m_pInspected;
		TablePanel *	m_pTable;

		DrawerPanel_p	m_pSlotsDrawer;
		Object_p		m_displayedLayoutPointer;


	};

} // namespace wg
#endif //WG_TWOSLOTINFOPANEL_DOT_H

