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
#ifndef	WG_POPUPOVERLAYSLOTINFOSECTION_DOT_H
#define WG_POPUPOVERLAYSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_popupoverlay.h>

namespace wg
{
	class PopupOverlaySlotInfoSection;
	typedef	StrongPtr<PopupOverlaySlotInfoSection>	PopupOverlaySlotInfoSection_p;
	typedef	WeakPtr<PopupOverlaySlotInfoSection>	PopupOverlaySlotInfoSection_wp;



	class PopupOverlaySlotInfoSection : public TypedInfoSection<PopupOverlay::Slot>
	{
	public:

		//.____ Creation __________________________________________

		static PopupOverlaySlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, PopupOverlay::Slot * pInspected) { return PopupOverlaySlotInfoSection_p(new PopupOverlaySlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		PopupOverlaySlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, PopupOverlay::Slot * pInspected );
		~PopupOverlaySlotInfoSection() {}

	};

} // namespace wg
#endif //WG_POPUPOVERLAYSLOTINFOSECTION_DOT_H
