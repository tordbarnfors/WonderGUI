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
#include "wg_canvasdisplayinfosection.h"


namespace wg
{

	const TypeInfo CanvasDisplayInfoSection::TYPEINFO = { "CanvasDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	CanvasDisplayInfoSection::CanvasDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, CanvasDisplay * pInspected)
		: TypedInfoSection<CanvasDisplay>( theme, pContext, CanvasDisplay::TYPEINFO.className, pInspected )
	{
		//TODO: Tint color, tintmap, blend mode, default size and skin around canvas (no public getters)

		this->slot = _createRows({
			objectRow( "Canvas: ",    [](CanvasDisplay* c) -> Object* { return c->canvas().rawPtr(); } ),
			textRow  ( "Placement: ", [](CanvasDisplay* c) { return toString(c->placement()); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CanvasDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
