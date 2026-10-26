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
#include "wg_areascrollchartinfosection.h"


namespace wg
{

	const TypeInfo AreaScrollChartInfoSection::TYPEINFO = { "AreaScrollChartInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	AreaScrollChartInfoSection::AreaScrollChartInfoSection(const DebugTheme& theme, IDebugContext* pContext, AreaScrollChart * pInspected)
		: TypedInfoSection<AreaScrollChart>( theme, pContext, AreaScrollChart::TYPEINFO.className, pInspected )
	{
		//TODO: Edgemap factory (private, no getter)

		this->slot = _createRows({
			boolRow( "Pad with last sample: ", [](AreaScrollChart* c) { return c->padWithLastSample(); } ),
			intRow ( "Entries: ",              [](AreaScrollChart* c) { return c->entries.size(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AreaScrollChartInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
