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
#include <wg_sizecapsule.h>
#include <wg_msgrouter.h>



namespace wg
{

	const TypeInfo SkinInspector::TYPEINFO = { "SkinInspector", &DebugWindow::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinInspector::SkinInspector(const Blueprint& blueprint, IDebugger * pHolder, Skin * pSkin) : DebugWindow( blueprint, pHolder )
	{
		m_title = _createObjectTitle(pSkin);
		m_pSkin = pSkin;

		auto pBasePanel = WGCREATE(PackPanel, _.axis = Axis::Y );


		//----

		pBasePanel->slots.pushBack( _createButtonRow(true,true), WGBP(PackPanelSlot, _.weight = 0.f) );

		auto pDisplayBackground = WGCREATE(TileSkin, _.surface = blueprint.transparencyGrid, _.spacing = 8, _.padding = 8);

		m_pSkinDisplay = WGCREATE(SkinDisplay, _.skin = pDisplayBackground, _.displaySkin = pSkin);

		auto pSizeCapsule = WGCREATE(SizeCapsule, _.minSize = { 100, 100 }, _.child = m_pSkinDisplay);

		pBasePanel->slots.pushBack(pSizeCapsule, WGBP(PackPanelSlot, _.weight = 0));

		//----

		m_pStateSelector = WGCREATE(SelectBox, _ = blueprint.theme->selectBox());

		for (int i = 0; i < StateEnum_size; i++)
		{
			m_pStateSelector->entries.pushBack(WGBP(SelectBoxEntry, _.id = i, _.text = toString((StateEnum(i) ))));
		}

		Base::msgRouter()->addRoute(m_pStateSelector, MsgType::Select, [this](Msg* pMsg) { this->_refreshState(); });;

		pBasePanel->slots.pushBack(m_pStateSelector, WGBP(PackPanelSlot, _.weight = 0) );

		//---

		pBasePanel->slots.pushBack(_createClassInfoPanels(blueprint, pSkin));

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinInspector::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _refreshState() ____________________________________________________

	void SkinInspector::_refreshState()
	{
		State selectedState = (StateEnum) m_pStateSelector->selectedEntryId();

		m_pSkinDisplay->setDisplayState(selectedState);
	}


} // namespace wg


