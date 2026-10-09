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
#include "wg_pianokeyboardinfosection.h"


namespace wg
{

	const TypeInfo PianoKeyboardInfoSection::TYPEINFO = { "PianoKeyboardInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	PianoKeyboardInfoSection::PianoKeyboardInfoSection(const DebugTheme& theme, IDebugContext* pContext, PianoKeyboard * pInspected)
		: TypedInfoSection<PianoKeyboard>( theme, pContext, PianoKeyboard::TYPEINFO.className, pInspected )
	{
		//TODO: Layout (number of white keys, black key positions) and key surfaces have no public getters
		//TODO: Pressed/selected keys (number of keys has no public getter)

		this->slot = _createRows({
			boolRow( "Flip key on press: ", [](PianoKeyboard* p) { return p->flipKeyOnPress(); } ),
			intRow ( "User pressed key: ",  [](PianoKeyboard* p) { return p->userPressedKey(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PianoKeyboardInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
