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
#ifndef	WG_SPLITPANELSLOTINFOSECTION_DOT_H
#define WG_SPLITPANELSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_splitpanel.h>

namespace wg
{
	class SplitPanelSlotInfoSection;
	typedef	StrongPtr<SplitPanelSlotInfoSection>	SplitPanelSlotInfoSection_p;
	typedef	WeakPtr<SplitPanelSlotInfoSection>	SplitPanelSlotInfoSection_wp;



	class SplitPanelSlotInfoSection : public TypedInfoSection<SplitPanel::Slot>
	{
	public:

		//.____ Creation __________________________________________

		static SplitPanelSlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, SplitPanel::Slot * pInspected) { return SplitPanelSlotInfoSection_p(new SplitPanelSlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		SplitPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, SplitPanel::Slot * pInspected );
		~SplitPanelSlotInfoSection() {}

		DrawerPanel_p	m_pGeoDrawer;
		Rect			m_displayedGeo;
	};

} // namespace wg
#endif //WG_SPLITPANELSLOTINFOSECTION_DOT_H
