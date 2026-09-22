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
#ifndef	WG_FLEXPANELSLOTINFOSECTION_DOT_H
#define WG_FLEXPANELSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_flexpanel.h>

namespace wg
{
	class FlexPanelSlotInfoSection;
	typedef	StrongPtr<FlexPanelSlotInfoSection>	FlexPanelSlotInfoSection_p;
	typedef	WeakPtr<FlexPanelSlotInfoSection>	FlexPanelSlotInfoSection_wp;



	class FlexPanelSlotInfoSection : public TypedInfoSection<FlexPanelSlot>
	{
	public:

		//.____ Creation __________________________________________

		static FlexPanelSlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, FlexPanelSlot * pInspected) { return FlexPanelSlotInfoSection_p(new FlexPanelSlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		FlexPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, FlexPanelSlot * pInspected );
		~FlexPanelSlotInfoSection() {}

		void			_updateRowVisibility();
	};

} // namespace wg
#endif //WG_FLEXPANELSLOTINFOSECTION_DOT_H
