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
#ifndef	WG_TOOLTIPOVERLAYINFOSECTION_DOT_H
#define WG_TOOLTIPOVERLAYINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_tooltipoverlay.h>

namespace wg
{
	class TooltipOverlayInfoSection;
	typedef	StrongPtr<TooltipOverlayInfoSection>	TooltipOverlayInfoSection_p;
	typedef	WeakPtr<TooltipOverlayInfoSection>	TooltipOverlayInfoSection_wp;



	class TooltipOverlayInfoSection : public TypedInfoSection<TooltipOverlay>
	{
	public:

		//.____ Creation __________________________________________

		static TooltipOverlayInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, TooltipOverlay * pInspected) { return TooltipOverlayInfoSection_p(new TooltipOverlayInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		TooltipOverlayInfoSection(const DebugTheme& theme, IDebugContext* pContext, TooltipOverlay * pInspected );
		~TooltipOverlayInfoSection() {}
	};

} // namespace wg
#endif //WG_TOOLTIPOVERLAYINFOSECTION_DOT_H
