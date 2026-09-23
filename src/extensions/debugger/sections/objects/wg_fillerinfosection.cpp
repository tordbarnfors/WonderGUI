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
#include "wg_fillerinfosection.h"


namespace wg
{

	const TypeInfo FillerInfoSection::TYPEINFO = { "FillerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	FillerInfoSection::FillerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Filler * pInspected)
		: TypedInfoSection<Filler>( theme, pContext, Filler::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			ptsRow( "Default width (pts): ",  [](Filler* f) { return f->defaultSize().w; } ),
			ptsRow( "Default height (pts): ", [](Filler* f) { return f->defaultSize().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& FillerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
