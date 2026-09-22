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
#include "wg_packpanelslotinfosection.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>

namespace wg
{

	const TypeInfo PackPanelSlotInfoSection::TYPEINFO = { "PackPanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PackPanelSlotInfoSection::PackPanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, PackPanelSlot * pInspected)
		: TypedInfoSection<PackPanelSlot>( theme, pContext, PackPanelSlot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			decimalRow( "Weight: ",   [](PackPanelSlot* s) { return s->weight(); } ),
			decimalRow( "Baseline: ", [](PackPanelSlot* s) { return s->baseline(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackPanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
