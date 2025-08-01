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
#include "wg_skininfopanel.h"
#include <wg_textdisplay.h>
#include <wg_numberdisplay.h>
#include <wg_basicnumberlayout.h>
#include <wg_packpanel.h>
#include <wg_skindisplay.h>
#include <wg_tileskin.h>
#include <wg_boxskin.h>


namespace wg
{

	const TypeInfo SkinInfoPanel::TYPEINFO = { "SkinInfoPanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinInfoPanel::SkinInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Skin * pSkin) : DebugPanel( blueprint, pHolder )
	{
		auto pBasePanel = WGCREATE( PackPanel, _.axis = Axis::Y );


		auto pDisplayBackground = WGCREATE(BoxSkin, _.color = Color::Wheat, _.padding = 8, _.spacing = 8);

		auto pDisplay = WGCREATE(SkinDisplay, _.skin = pDisplayBackground, _.displaySkin = pSkin);
		pBasePanel->slots << pDisplay;

		pBasePanel->slots << _createBorderDrawer("Margin", pSkin->margin());
		pBasePanel->slots << _createBorderDrawer("Padding", pSkin->padding());
		pBasePanel->slots << _createBorderDrawer("Overflow", pSkin->overflow());
		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinInfoPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


