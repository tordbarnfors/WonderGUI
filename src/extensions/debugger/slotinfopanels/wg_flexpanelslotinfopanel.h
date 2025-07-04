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

		static FlexPanelSlotInfoPanel_p		create( const Blueprint& blueprint, StaticSlot * pStaticSlot) { return FlexPanelSlotInfoPanel_p(new FlexPanelSlotInfoPanel(blueprint, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		FlexPanelSlotInfoPanel(const Blueprint& blueprint, StaticSlot * pStaticSlot );
		~FlexPanelSlotInfoPanel() {}

		void	flexPosToString(FlexPos pos, char * pString);

	};

} // namespace wg
#endif //WG_FLEXPANELSLOTINFOPANEL_DOT_H


