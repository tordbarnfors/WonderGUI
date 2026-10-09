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
#ifndef	WG_MODALOVERLAYSLOTINFOSECTION_DOT_H
#define WG_MODALOVERLAYSLOTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_typedinfosection.h>
#include <wg_modaloverlay.h>

namespace wg
{
	class ModalOverlaySlotInfoSection;
	typedef	StrongPtr<ModalOverlaySlotInfoSection>	ModalOverlaySlotInfoSection_p;
	typedef	WeakPtr<ModalOverlaySlotInfoSection>	ModalOverlaySlotInfoSection_wp;



	class ModalOverlaySlotInfoSection : public TypedInfoSection<ModalOverlay::Slot>
	{
	public:

		//.____ Creation __________________________________________

		static ModalOverlaySlotInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, ModalOverlay::Slot * pInspected) { return ModalOverlaySlotInfoSection_p(new ModalOverlaySlotInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ModalOverlaySlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, ModalOverlay::Slot * pInspected );
		~ModalOverlaySlotInfoSection() {}

	};

} // namespace wg
#endif //WG_MODALOVERLAYSLOTINFOSECTION_DOT_H
