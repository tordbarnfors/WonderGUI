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
#ifndef	WG_TWOSLOTINFOSECTION_DOT_H
#define WG_TWOSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_twoslotpanel.h>

namespace wg
{
	class TwoSlotPanelInfoSection;
	typedef	StrongPtr<TwoSlotPanelInfoSection>	TwoSlotPanelInfoSection_p;
	typedef	WeakPtr<TwoSlotPanelInfoSection>	TwoSlotPanelInfoSection_wp;



	class TwoSlotPanelInfoSection : public TypedInfoSection<TwoSlotPanel>
	{
	public:

		//.____ Creation __________________________________________

		static TwoSlotPanelInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, TwoSlotPanel * pInspected) { return TwoSlotPanelInfoSection_p(new TwoSlotPanelInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		TwoSlotPanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, TwoSlotPanel * pInspected );
		~TwoSlotPanelInfoSection() {}

		DrawerPanel_p	m_pSlotsDrawer;
	};

} // namespace wg
#endif //WG_TWOSLOTINFOSECTION_DOT_H
