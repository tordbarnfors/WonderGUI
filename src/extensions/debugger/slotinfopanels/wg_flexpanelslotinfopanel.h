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
#ifndef	WG_FLEXPANELSLOTINFOPANEL_DOT_H
#define WG_FLEXPANELSLOTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_flexpanel.h>

namespace wg
{
	class FlexPanelSlotInfoPanel;
	typedef	StrongPtr<FlexPanelSlotInfoPanel>	FlexPanelSlotInfoPanel_p;
	typedef	WeakPtr<FlexPanelSlotInfoPanel>	FlexPanelSlotInfoPanel_wp;



	class FlexPanelSlotInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static FlexPanelSlotInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) { return FlexPanelSlotInfoPanel_p(new FlexPanelSlotInfoPanel(blueprint, pHolder, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh(StaticSlot * pStaticSlot) override;


	protected:
		FlexPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot );
		~FlexPanelSlotInfoPanel() {}

		void	flexPosToString(FlexPos pos, char * pString);

		TablePanel_p	m_pTable;
	};

} // namespace wg
#endif //WG_FLEXPANELSLOTINFOPANEL_DOT_H


