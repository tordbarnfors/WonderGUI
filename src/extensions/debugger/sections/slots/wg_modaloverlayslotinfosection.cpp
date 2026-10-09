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
#include "wg_modaloverlayslotinfosection.h"

namespace wg
{

	const TypeInfo ModalOverlaySlotInfoSection::TYPEINFO = { "ModalOverlaySlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ModalOverlaySlotInfoSection::ModalOverlaySlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, ModalOverlay::Slot * pInspected)
		: TypedInfoSection<ModalOverlay::Slot>( theme, pContext, ModalOverlay::Slot::TYPEINFO.className, pInspected )
	{
		// Origo, offset and size are what the slot was placed with. Resulting
		// geo is shown by the Overlay::Slot section.

		this->slot = _createRows({
			textRow( "Origo: ",                    [](ModalOverlay::Slot* s) { return toString(s->origo()); } ),
			ptsRow ( "Placement offset X (pts): ", [](ModalOverlay::Slot* s) { return s->pos().x; } ),
			ptsRow ( "Placement offset Y (pts): ", [](ModalOverlay::Slot* s) { return s->pos().y; } ),
			ptsRow ( "Placement width (pts): ",    [](ModalOverlay::Slot* s) { return s->size().w; } ),
			ptsRow ( "Placement height (pts): ",   [](ModalOverlay::Slot* s) { return s->size().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ModalOverlaySlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
