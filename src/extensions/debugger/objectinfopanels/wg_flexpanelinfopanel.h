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

#ifndef	WG_FLEXPANELINFOPANEL_DOT_H
#define WG_FLEXPANELINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_flexpanel.h>

namespace wg
{
	class FlexPanelInfoPanel;
	typedef	StrongPtr<FlexPanelInfoPanel>	FlexPanelInfoPanel_p;
	typedef	WeakPtr<FlexPanelInfoPanel>	FlexPanelInfoPanel_wp;



	class FlexPanelInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static FlexPanelInfoPanel_p		create( const Blueprint& blueprint, IDebugger* pHolder, FlexPanel * pPanel) { return FlexPanelInfoPanel_p(new FlexPanelInfoPanel(blueprint, pHolder, pPanel) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		FlexPanelInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, FlexPanel * pPanel );
		~FlexPanelInfoPanel() {}

		FlexPanel *		m_pInspected;
		TablePanel_p	m_pTable;
		DrawerPanel_p	m_pSlotsDrawer;

	};

} // namespace wg
#endif //WG_FLEXPANELINFOPANEL_DOT_H

