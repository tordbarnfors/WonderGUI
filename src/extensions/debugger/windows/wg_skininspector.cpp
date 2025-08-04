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
#include "wg_skininspector.h"
#include <wg_textdisplay.h>
#include <wg_skindisplay.h>
#include <wg_boxskin.h>
#include <wg_packpanel.h>
#include <wg_tileskin.h>



namespace wg
{

	const TypeInfo SkinInspector::TYPEINFO = { "SkinInspector", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinInspector::SkinInspector(const Blueprint& blueprint, DebugPanel::Holder * pHolder, Skin * pSkin) : DebugPanel( blueprint, pHolder )
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		pBasePanel->slots << _createObjectHeader(pSkin);

		auto pDisplayBackground = WGCREATE(TileSkin, _.surface = blueprint.transparencyGrid, _.spacing = 8, _.padding = 8 );

		auto pDisplay = WGCREATE(SkinDisplay, _.skin = pDisplayBackground, _.displaySkin = pSkin);
		pBasePanel->slots << pDisplay;

		auto bp = m_blueprint;

		auto pTypeInfo = &pSkin->typeInfo();

		while (pTypeInfo != nullptr)
		{
			bp.classCapsule.label.text = pTypeInfo->className;
			pBasePanel->slots << m_pHolder->createObjectInfoPanel(bp, pTypeInfo, pSkin);
			pTypeInfo = pTypeInfo->pSuperClass;
		}


		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


