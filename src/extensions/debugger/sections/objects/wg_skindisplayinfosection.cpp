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
#include "wg_skindisplayinfosection.h"


namespace wg
{

	const TypeInfo SkinDisplayInfoSection::TYPEINFO = { "SkinDisplayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinDisplayInfoSection::SkinDisplayInfoSection(const DebugTheme& theme, IDebugContext* pContext, SkinDisplay * pInspected)
		: TypedInfoSection<SkinDisplay>( theme, pContext, SkinDisplay::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			objectRow( "Display skin: ",    [](SkinDisplay* s) -> Object* { return s->displaySkin().rawPtr(); } ),
			textRow  ( "Display state: ",   [](SkinDisplay* s) { return toString(s->displayState().value()); } ),
			intRow   ( "Display value: ",   [](SkinDisplay* s) { return s->displayValue(); } ),
			intRow   ( "Display value 2: ", [](SkinDisplay* s) { return s->displayValue2(); } ),
			intRow   ( "Display scale: ",   [](SkinDisplay* s) { return s->displayScale(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinDisplayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
