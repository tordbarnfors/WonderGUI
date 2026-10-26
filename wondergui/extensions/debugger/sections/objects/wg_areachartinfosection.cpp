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
#include "wg_areachartinfosection.h"


namespace wg
{

	const TypeInfo AreaChartInfoSection::TYPEINFO = { "AreaChartInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	AreaChartInfoSection::AreaChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, AreaChart * pInspected)
		: TypedInfoSection<AreaChart>( theme, pContext, AreaChart::TYPEINFO.className, pInspected )
	{
		//TODO: Edgemap factory, dirty section width, preserve peaks, resampler (private, no getters)

		this->slot = _createRows({
			textRow( "Flip: ",    [](AreaChart* c) { return toString(c->flip()); } ),
			intRow ( "Entries: ", [](AreaChart* c) { return c->entries.size(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AreaChartInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
