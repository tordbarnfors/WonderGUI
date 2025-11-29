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
#ifndef	WG_TWOSLOTPANELSLOTINFOPANEL_DOT_H
#define WG_TWOSLOTPANELSLOTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_twoslotpanel.h>

namespace wg
{
	class TwoSlotPanelSlotInfoPanel;
	typedef	StrongPtr<TwoSlotPanelSlotInfoPanel>	TwoSlotPanelSlotInfoPanel_p;
	typedef	WeakPtr<TwoSlotPanelSlotInfoPanel>	TwoSlotPanelSlotInfoPanel_wp;



	class TwoSlotPanelSlotInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static TwoSlotPanelSlotInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) { return TwoSlotPanelSlotInfoPanel_p(new TwoSlotPanelSlotInfoPanel(blueprint, pHolder, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh(StaticSlot * pStaticSlot) override;


	protected:
		TwoSlotPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot );
		~TwoSlotPanelSlotInfoPanel() {}

		TablePanel_p			m_pTable;
	};

} // namespace wg
#endif //WG_TWOSLOTPANELSLOTINFOPANEL_DOT_H


