/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#include "wg_dummyinfopanel.h"
#include <wg_textdisplay.h>

namespace wg
{

	const TypeInfo DummyInfoPanel::TYPEINFO = { "DummyInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	DummyInfoPanel::DummyInfoPanel(const Blueprint& blueprint, DebugPanel::Holder * pHolder, void * pDummy) : DebugPanel( blueprint, pHolder )
	{

		auto pDisplay = TextDisplay::create( WGOVR( blueprint.infoDisplay, _.display.text = "No debug info for this class" ));
		this->slot = pDisplay;

	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DummyInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


