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
#include "wg_nodepanelslotinfosection.h"

namespace wg
{

	const TypeInfo NodePanelSlotInfoSection::TYPEINFO = { "NodePanelSlotInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	NodePanelSlotInfoSection::NodePanelSlotInfoSection(const DebugTheme& theme, IDebugContext* pContext, NodePanelSlot * pInspected)
		: TypedInfoSection<NodePanelSlot>( theme, pContext, NodePanelSlot::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			intRow( "Node id: ",          [](NodePanelSlot* s) { return s->nodeId(); } ),
			ptsRow( "Center X (pts): ",   [](NodePanelSlot* s) { return s->center().x; } ),
			ptsRow( "Center Y (pts): ",   [](NodePanelSlot* s) { return s->center().y; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& NodePanelSlotInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
