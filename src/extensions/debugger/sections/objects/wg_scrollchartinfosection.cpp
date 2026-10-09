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
#include "wg_scrollchartinfosection.h"


namespace wg
{

	const TypeInfo ScrollChartInfoSection::TYPEINFO = { "ScrollChartInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollChartInfoSection::ScrollChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, ScrollChart * pInspected)
		: TypedInfoSection<ScrollChart>( theme, pContext, ScrollChart::TYPEINFO.className, pInspected )
	{
		//TODO: Max display time, latency, latest timestamp (protected)
		//TODO: Surface factory, scroll surface, pixel format, scrolling state (private, no getters)

		this->slot = _createRows({
			textRow( "Flip: ",                   [](ScrollChart* c) { return toString(c->flip()); } ),
			intRow ( "Display time (microsec): ", [](ScrollChart* c) { return c->displayTime(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollChartInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
