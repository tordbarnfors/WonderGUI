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
#include "wg_containerinfosection.h"


namespace wg
{

	const TypeInfo ContainerInfoSection::TYPEINFO = { "ContainerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ContainerInfoSection::ContainerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Container * pInspected)
		: TypedInfoSection<Container>( theme, pContext, Container::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			boolRow( "Use pick handles: ", [](Container* c) { return c->usePickHandles(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ContainerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
