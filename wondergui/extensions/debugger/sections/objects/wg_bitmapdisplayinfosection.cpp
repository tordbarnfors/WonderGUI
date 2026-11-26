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
#include "wg_bitmapdisplayinfosection.h"


namespace wg
{

	const TypeInfo BitmapDisplayInfoSection::TYPEINFO = { "BitmapDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	BitmapDisplayInfoSection::BitmapDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, BitmapDisplay * pInspected)
		: TypedInfoSection<BitmapDisplay>( theme, pContext, BitmapDisplay::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			objectRow ( "Bitmap: ",           [](BitmapDisplay* s) -> Object* { return s->bitmap().rawPtr(); } ),
			textRow   ( "Bitmap placement: ", [](BitmapDisplay* s) { return toString(s->bitmapPlacement()); } ),
			boolRow   ( "Zoom to fit: ",       [](BitmapDisplay* s) { return s->zoomToFit(); } ),
			decimalRow( "Zoom: ",              [](BitmapDisplay* s) { return s->zoom(); } ),
			decimalRow( "Min user zoom: ",     [](BitmapDisplay* s) { return s->minUserZoom(); } ),
			decimalRow( "Max user zoom: ",     [](BitmapDisplay* s) { return s->maxUserZoom(); } ),
			ptsRow    ( "Offset X (pts): ",    [](BitmapDisplay* s) { return s->offset().x; } ),
			ptsRow    ( "Offset Y (pts): ",    [](BitmapDisplay* s) { return s->offset().y; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BitmapDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
