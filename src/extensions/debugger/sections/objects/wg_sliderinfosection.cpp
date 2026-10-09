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
#include "wg_sliderinfosection.h"


namespace wg
{

	const TypeInfo SliderInfoSection::TYPEINFO = { "SliderInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SliderInfoSection::SliderInfoSection(const DebugTheme& theme, IDebugContext* pContext, Slider * pInspected)
		: TypedInfoSection<Slider>( theme, pContext, Slider::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			decimalRow( "Value: ",                      [](Slider* s) { return s->value(); } ),
			intRow    ( "Steps: ",                      [](Slider* s) { return s->steps(); } ),
			textRow   ( "Axis: ",                       [](Slider* s) { return toString(s->axis()); } ),
			ptsRow    ( "Default slide length (pts): ", [](Slider* s) { return s->defaultSlideLength(); } ),
			objectRow ( "Handle skin: ",                [](Slider* s) -> Object* { return s->handleSkin().rawPtr(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SliderInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
