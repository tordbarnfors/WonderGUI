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
#ifndef	WG_OVERLAYINFOSECTION_DOT_H
#define WG_OVERLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_overlay.h>

namespace wg
{
	class OverlayInfoSection;
	typedef	StrongPtr<OverlayInfoSection>	OverlayInfoSection_p;
	typedef	WeakPtr<OverlayInfoSection>	OverlayInfoSection_wp;



	class OverlayInfoSection : public TypedInfoSection<Overlay>
	{
	public:

		//.____ Creation __________________________________________

		static OverlayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Overlay * pInspected) { return OverlayInfoSection_p(new OverlayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		OverlayInfoSection(const DebugTheme& theme, IDebugContext* pContext, Overlay * pInspected );
		~OverlayInfoSection() {}

		DrawerPanel_p	m_pMainSlotDrawer;
	};

} // namespace wg
#endif //WG_OVERLAYINFOSECTION_DOT_H
