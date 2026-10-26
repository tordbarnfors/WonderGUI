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
#include "wg_panelslotinfosection.h"
#include <wg_textdisplay.h>
#include <wg_panel.h>

namespace wg
{

	const TypeInfo PanelSlotInfoSection::TYPEINFO = { "PanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelSlotInfoSection::PanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, PanelSlot * pInspected)
		: TypedInfoSection<PanelSlot>( theme, pContext, PanelSlot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			boolRow( "Visible: ", [](PanelSlot* s) { return s->isVisible(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
