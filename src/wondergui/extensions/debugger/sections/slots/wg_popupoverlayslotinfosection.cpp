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
#include "wg_popupoverlayslotinfosection.h"

namespace wg
{

	const TypeInfo PopupOverlaySlotInfoSection::TYPEINFO = { "PopupOverlaySlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PopupOverlaySlotInfoSection::PopupOverlaySlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, PopupOverlay::Slot * pInspected)
		: TypedInfoSection<PopupOverlay::Slot>( theme, pContext, PopupOverlay::Slot::TYPEINFO.className, pInspected )
	{
		//TODO: Open/close state and its counter (protected, no getter)

		this->slot = _createRows({
			objectRow( "Opener: ",                [](PopupOverlay::Slot* s) -> Object* { return s->opener().rawPtr(); } ),
			textRow  ( "Attach point: ",          [](PopupOverlay::Slot* s) { return toString(s->attachPoint()); } ),
			boolRow  ( "Peek mode: ",             [](PopupOverlay::Slot* s) { return s->peekMode(); } ),
			boolRow  ( "Close on select: ",       [](PopupOverlay::Slot* s) { return s->closeOnSelect(); } ),
			spxRow   ( "Launcher X (spx): ",      [](PopupOverlay::Slot* s) { return s->launcherGeo().x; } ),
			spxRow   ( "Launcher Y (spx): ",      [](PopupOverlay::Slot* s) { return s->launcherGeo().y; } ),
			spxRow   ( "Launcher width (spx): ",  [](PopupOverlay::Slot* s) { return s->launcherGeo().w; } ),
			spxRow   ( "Launcher height (spx): ", [](PopupOverlay::Slot* s) { return s->launcherGeo().h; } ),
			spxRow   ( "Max width (spx): ",       [](PopupOverlay::Slot* s) { return s->maxSize().w; } ),
			spxRow   ( "Max height (spx): ",      [](PopupOverlay::Slot* s) { return s->maxSize().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PopupOverlaySlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
