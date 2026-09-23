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
#include "wg_staticslotinfosection.h"
#include <wg_textdisplay.h>

namespace wg
{

	const TypeInfo StaticSlotInfoSection::TYPEINFO = { "StaticSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	StaticSlotInfoSection::StaticSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, StaticSlot * pInspected)
		: TypedInfoSection<StaticSlot>( theme, pContext, StaticSlot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			objectRow( "Widget: ",           [](StaticSlot* s) -> Object* { return s->widget(); } ),
			ptsRow   ( "X offset (pts): ",   [](StaticSlot* s) { return s->geo().x; } ),
			ptsRow   ( "Y offset (pts): ",   [](StaticSlot* s) { return s->geo().y; } ),
			ptsRow   ( "Width (pts): ",      [](StaticSlot* s) { return s->geo().w; } ),
			ptsRow   ( "Height (pts): ",     [](StaticSlot* s) { return s->geo().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
