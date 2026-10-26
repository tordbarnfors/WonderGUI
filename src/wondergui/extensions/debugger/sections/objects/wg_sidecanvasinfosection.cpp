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
#include "wg_sidecanvasinfosection.h"


namespace wg
{

	const TypeInfo SideCanvasInfoSection::TYPEINFO = { "SideCanvasInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SideCanvasInfoSection::SideCanvasInfoSection(const DebugTheme& theme, IDebugContext* pContext, SideCanvas * pInspected)
		: TypedInfoSection<SideCanvas>( theme, pContext, SideCanvas::TYPEINFO.className, pInspected )
	{
		//TODO: Holder (protected member, not an Object)

		this->slot = _createRows({});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SideCanvasInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
