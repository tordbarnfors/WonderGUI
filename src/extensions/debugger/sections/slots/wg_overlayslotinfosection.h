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
#ifndef	WG_OVERLAYSLOTINFOSECTION_DOT_H
#define WG_OVERLAYSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_overlay.h>

namespace wg
{
	class OverlaySlotInfoSection;
	typedef	StrongPtr<OverlaySlotInfoSection>	OverlaySlotInfoSection_p;
	typedef	WeakPtr<OverlaySlotInfoSection>	OverlaySlotInfoSection_wp;



	class OverlaySlotInfoSection : public TypedInfoSection<Overlay::Slot>
	{
	public:

		//.____ Creation __________________________________________

		static OverlaySlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Overlay::Slot * pInspected) { return OverlaySlotInfoSection_p(new OverlaySlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		OverlaySlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, Overlay::Slot * pInspected );
		~OverlaySlotInfoSection() {}

		DrawerPanel_p	m_pGeoDrawer;
		Rect			m_displayedGeo;
	};

} // namespace wg
#endif //WG_OVERLAYSLOTINFOSECTION_DOT_H
