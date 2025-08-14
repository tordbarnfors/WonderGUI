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
#include "wg_textstyleinspector.h"
#include <wg_textdisplay.h>
#include <wg_skindisplay.h>
#include <wg_boxskin.h>
#include <wg_packpanel.h>



namespace wg
{

	const TypeInfo TextStyleInspector::TYPEINFO = { "TextStyleInspector", &DebugWindow::TYPEINFO };


	//____ constructor _____________________________________________________________

	TextStyleInspector::TextStyleInspector(const Blueprint& blueprint, IDebugger* pHolder, TextStyle* pStyle) : DebugWindow(blueprint, pHolder)
	{
		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y, _.layout = PackLayout::create({}));

		pBasePanel->slots << _createObjectHeader(pStyle);

		auto pTypeInfo = &pStyle->typeInfo();

		while (pTypeInfo != nullptr)
		{
			pBasePanel->slots << m_pHolder->createObjectInfoPanel(pTypeInfo, pStyle);
			pTypeInfo = pTypeInfo->pSuperClass;
		}

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& TextStyleInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg


