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
#include "wg_rangesliderinfosection.h"


namespace wg
{

	const TypeInfo RangeSliderInfoSection::TYPEINFO = { "RangeSliderInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	RangeSliderInfoSection::RangeSliderInfoSection(const DebugTheme& theme, IDebugContext* pContext, RangeSlider * pInspected)
		: TypedInfoSection<RangeSlider>( theme, pContext, RangeSlider::TYPEINFO.className, pInspected )
	{
		//TODO: Minimum range (m_minRange has no public getter)

		this->slot = _createRows({
			decimalRow( "Range begin: ",                [](RangeSlider* s) { return s->rangeBegin(); } ),
			decimalRow( "Range end: ",                  [](RangeSlider* s) { return s->rangeEnd(); } ),
			boolRow   ( "Range dragable: ",             [](RangeSlider* s) { return s->isRangeDragable(); } ),
			intRow    ( "Steps: ",                      [](RangeSlider* s) { return s->steps(); } ),
			textRow   ( "Axis: ",                       [](RangeSlider* s) { return toString(s->axis()); } ),
			ptsRow    ( "Default slide length (pts): ", [](RangeSlider* s) { return s->defaultSlideLength(); } ),
			objectRow ( "Begin handle skin: ",          [](RangeSlider* s) -> Object* { return s->beginHandleSkin().rawPtr(); } ),
			objectRow ( "End handle skin: ",            [](RangeSlider* s) -> Object* { return s->endHandleSkin().rawPtr(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& RangeSliderInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
