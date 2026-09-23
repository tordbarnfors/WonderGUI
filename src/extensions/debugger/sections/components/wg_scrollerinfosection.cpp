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
#include "wg_scrollerinfosection.h"
#include <wg_enumextras.h>

namespace wg
{

	const TypeInfo ScrollerInfoSection::TYPEINFO = { "ScrollerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ScrollerInfoSection::ScrollerInfoSection(const DebugTheme& theme, IDebugContext* pContext, Scroller * pInspected)
		: TypedInfoSection<Scroller>( theme, pContext, Scroller::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow  ( "Axis: ",                    [](Scroller* s) { return toString(s->_axis()); } ),
			boolRow  ( "Jump to press: ",           [](Scroller* s) { return s->jumpToPress(); } ),
			boolRow  ( "Visible: ",                 [](Scroller* s) { return s->isVisible(); } ),
			objectRow( "Back skin: ",               [](Scroller* s) -> Object* { return s->backSkin(); } ),
			objectRow( "Bar skin: ",                [](Scroller* s) -> Object* { return s->barSkin(); } ),
			objectRow( "Backward button skin: ",    [](Scroller* s) -> Object* { return s->backwardButtonSkin(); } ),
			objectRow( "Forward button skin: ",     [](Scroller* s) -> Object* { return s->forwardButtonSkin(); } ),
			textRow  ( "Back state: ",              [](Scroller* s) { return toString(s->_backState().value()); } ),
			textRow  ( "Bar state: ",               [](Scroller* s) { return toString(s->_barState().value()); } ),
			textRow  ( "Backward button state: ",   [](Scroller* s) { return toString(s->_backwardButtonState().value()); } ),
			textRow  ( "Forward button state: ",    [](Scroller* s) { return toString(s->_forwardButtonState().value()); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
