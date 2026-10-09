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
#include "wg_surfacedisplayinfosection.h"


namespace wg
{

	const TypeInfo SurfaceDisplayInfoSection::TYPEINFO = { "SurfaceDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SurfaceDisplayInfoSection::SurfaceDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, SurfaceDisplay * pInspected)
		: TypedInfoSection<SurfaceDisplay>( theme, pContext, SurfaceDisplay::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			objectRow ( "Surface: ",           [](SurfaceDisplay* s) -> Object* { return s->surface().rawPtr(); } ),
			textRow   ( "Surface placement: ", [](SurfaceDisplay* s) { return toString(s->surfacePlacement()); } ),
			boolRow   ( "Zoom to fit: ",       [](SurfaceDisplay* s) { return s->zoomToFit(); } ),
			decimalRow( "Zoom: ",              [](SurfaceDisplay* s) { return s->zoom(); } ),
			decimalRow( "Min user zoom: ",     [](SurfaceDisplay* s) { return s->minUserZoom(); } ),
			decimalRow( "Max user zoom: ",     [](SurfaceDisplay* s) { return s->maxUserZoom(); } ),
			ptsRow    ( "Offset X (pts): ",    [](SurfaceDisplay* s) { return s->offset().x; } ),
			ptsRow    ( "Offset Y (pts): ",    [](SurfaceDisplay* s) { return s->offset().y; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SurfaceDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
