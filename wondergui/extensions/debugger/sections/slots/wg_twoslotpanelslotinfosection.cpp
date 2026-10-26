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
#include "wg_twoslotpanelslotinfosection.h"
#include <wg_textdisplay.h>
#include <wg_twoslotpanel.h>

namespace wg
{

	const TypeInfo TwoSlotPanelSlotInfoSection::TYPEINFO = { "TwoSlotPanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	TwoSlotPanelSlotInfoSection::TwoSlotPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, TwoSlotPanel::Slot * pInspected)
		: TypedInfoSection<TwoSlotPanel::Slot>( theme, pContext, TwoSlotPanel::Slot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			decimalRow( "Weight: ", [](TwoSlotPanel::Slot* s) { return s->weight(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TwoSlotPanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
