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
#ifndef	WG_TWOSLOTPANELSLOTINFOSECTION_DOT_H
#define WG_TWOSLOTPANELSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>
#include <wg_twoslotpanel.h>

namespace wg
{
	class TwoSlotPanelSlotInfoSection;
	typedef	StrongPtr<TwoSlotPanelSlotInfoSection>	TwoSlotPanelSlotInfoSection_p;
	typedef	WeakPtr<TwoSlotPanelSlotInfoSection>	TwoSlotPanelSlotInfoSection_wp;



	class TwoSlotPanelSlotInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static TwoSlotPanelSlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, StaticSlot * pStaticSlot) { return TwoSlotPanelSlotInfoSection_p(new TwoSlotPanelSlotInfoSection(theme, pContext, pStaticSlot) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh(StaticSlot * pStaticSlot) override;


	protected:
		TwoSlotPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, StaticSlot * pStaticSlot );
		~TwoSlotPanelSlotInfoSection() {}

		TablePanel_p			m_pTable;
	};

} // namespace wg
#endif //WG_TWOSLOTPANELSLOTINFOSECTION_DOT_H


