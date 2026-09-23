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
#include "wg_panelinfosection.h"


namespace wg
{

	const TypeInfo PanelInfoSection::TYPEINFO = { "PanelInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PanelInfoSection::PanelInfoSection(const DebugTheme& theme, IDebugContext* pContext, Panel * pInspected)
		: TypedInfoSection<Panel>( theme, pContext, Panel::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow( "Mask op: ", [](Panel* p) { return toString(p->maskOp()); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PanelInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
