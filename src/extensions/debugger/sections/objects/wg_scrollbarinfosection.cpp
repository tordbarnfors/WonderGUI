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
#include "wg_scrollbarinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ScrollbarInfoSection::TYPEINFO = { "ScrollbarInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollbarInfoSection::ScrollbarInfoSection(const DebugTheme& theme, IDebugContext* pContext, Scrollbar * pInspected)
		: TypedInfoSection<Scrollbar>( theme, pContext, Scrollbar::TYPEINFO.className, pInspected )
	{
		//TODO: Movement amounts (single step, wheel roll, page overlap) are only reachable through protected members

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			textRow   ( "Axis: ",                     [](Scrollbar* s) { return toString(s->axis()); } ),
			ptsRow    ( "View position (pts): ",      [](Scrollbar* s) { return s->viewPos(); } ),
			ptsRow    ( "View length (pts): ",        [](Scrollbar* s) { return s->viewLength(); } ),
			ptsRow    ( "Content length (pts): ",     [](Scrollbar* s) { return s->contentLength(); } ),
			decimalRow( "Fractional view position: ", [](Scrollbar* s) { return s->contentLength() > s->viewLength() ? s->fracViewPos() : 0.f; } ),
			decimalRow( "Fractional view length: ",   [](Scrollbar* s) { return s->contentLength() > 0 ? s->fracViewLength() : 1.f; } )
		});

		m_pScrollbarDrawer = _createComponentDrawer("Scrollbar", &pInspected->scrollbar);

		pPanel->slots.pushBack({ pTable, m_pScrollbarDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollbarInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ScrollbarInfoSection::refresh()
	{
		TypedInfoSection<Scrollbar>::refresh();

		_refreshComponentDrawer(m_pScrollbarDrawer);
	}

} // namespace wg
