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
#ifndef	WG_PACKPANELSLOTINFOPANEL_DOT_H
#define WG_PACKPANELSLOTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class PackPanelSlotInfoPanel;
	typedef	StrongPtr<PackPanelSlotInfoPanel>	PackPanelSlotInfoPanel_p;
	typedef	WeakPtr<PackPanelSlotInfoPanel>	PackPanelSlotInfoPanel_wp;



	class PackPanelSlotInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static PackPanelSlotInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot) { return PackPanelSlotInfoPanel_p(new PackPanelSlotInfoPanel(blueprint, pHolder, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh(StaticSlot * pStaticSlot) override;


	protected:
		PackPanelSlotInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, StaticSlot * pStaticSlot );
		~PackPanelSlotInfoPanel() {}

		TablePanel_p	m_pTable;
	};

} // namespace wg
#endif //WG_PACKPANELSLOTINFOPANEL_DOT_H


