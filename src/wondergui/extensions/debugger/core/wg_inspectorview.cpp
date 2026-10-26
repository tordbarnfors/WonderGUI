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
#include "wg_inspectorview.h"
#include "wg_infosection.h"

#include <wg_colorskin.h>
#include <wg_msgrouter.h>
#include <wg_msg.h>
#include <wg_packpanel.h>
#include <wg_textdisplay.h>
#include <wg_blockskin.h>

#include <cinttypes>


namespace wg
{

	const TypeInfo InspectorView::TYPEINFO = { "InspectorView", &Capsule::TYPEINFO };


	//____ constructor _____________________________________________________________

	InspectorView::InspectorView(const DebugTheme& theme, IDebugContext * pContext )
		: Capsule( theme.mainCapsule )
		, m_pContext(pContext)
	{
		setSkin(nullptr);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& InspectorView::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAutoRefresh() ______________________________________________________

	void InspectorView::setAutoRefresh(bool bAutoRefresh)
	{
		if( bAutoRefresh != m_bAutoRefresh )
		{
			if( bAutoRefresh )
				_startReceiveUpdates();
			else
				_stopReceiveUpdates();

			m_bAutoRefresh = bAutoRefresh;
		}
	}

	//____ refresh() _____________________________________________________________

	void InspectorView::refresh()
	{
		_refreshRecursively(slot._widget());
	}

	//____ _refreshRecursively() _________________________________________________

	void InspectorView::_refreshRecursively( Widget * pWidget )
	{
		auto pInfoSection = dynamic_cast<InfoSection*>(pWidget);
		if( pInfoSection )
		{
			pInfoSection->refresh();
			return;
		}

		auto pContainer = dynamic_cast<Container*>(pWidget);
		if( pContainer )
		{
			auto pChild = pContainer->firstChild();
			while( pChild )
			{
				_refreshRecursively(pChild);
				pChild = pChild->nextSibling();
			}
		}
	}

	//____ _update() _____________________________________________________________

	void InspectorView::_update(int microPassed, int64_t microsecTimestamp)
	{
		refresh();
	}

	//____ _createButtonRow() ____________________________________________________

	PackPanel_p InspectorView::_createButtonRow( bool bAutoRefresh, bool bRefresh )
	{
		auto pPackLayout = PackLayout::create({});

		auto& bp = m_pContext->theme();

		auto pButtonRow = PackPanel::create(WGBP(PackPanel,
		  _.axis = Axis::X,
		  _.layout = pPackLayout,
		  _.skin = dbgkit::Skins::Plate));

		auto pAutoRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			  _.surface = bp.icons,
			  _.firstBlock = { 48,48,16,16 } ));

		auto pRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			  _.surface = bp.icons,
			  _.firstBlock = { 0,0,16,16 } ));

		auto pAutoRefreshButton = WGCREATE(dbgkit::ToggleButton, _.icon.skin = pAutoRefreshIcon);
		auto pRefreshButton = WGCREATE(dbgkit::Button, _.icon.skin = pRefreshIcon);

		Base::msgRouter()->addRoute(pAutoRefreshButton, MsgType::Toggle, [this](Msg* _pMsg) {

			auto pMsg = static_cast<ToggleMsg*>(_pMsg);
			setAutoRefresh(pMsg->isChecked());
		});

		Base::msgRouter()->addRoute(pRefreshButton, MsgType::Select, [this](Msg* _pMsg) {
			refresh();
		});


		pButtonRow->slots.pushBack( { pAutoRefreshButton, pRefreshButton } );

		return pButtonRow;
	}

	//____ _createObjectHeader() ______________________________________________

	Widget_p InspectorView::_createObjectHeader(Object* pObject) const
	{
		auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
			_.display.text = _createObjectTitle(pObject),
			_.display.style = dbgkit::TextStyles::Heading5
		));

		return pDisplay;
	}

	//____ _createObjectTitle() ______________________________________________

	String InspectorView::_createObjectTitle(Object* pObject) const
	{
		char temp[64];
		std::snprintf(temp, sizeof(temp), " 0x%" PRIxPTR, reinterpret_cast<std::uintptr_t>(pObject));

		CharBuffer buf(64);
		buf.pushBack(pObject->typeInfo().className);
		int ofs = buf.length();
		buf.pushBack(temp);
		buf.setStyle(dbgkit::TextStyles::Default, ofs, 10000);

		return &buf;
	}

	//___ _createClassInfoSections() ______________________________________________

	Widget_p InspectorView::_createClassInfoSections(const DebugTheme& bp, Object* pObject) const
	{
		auto pScrollPanel = WGCREATE(dbgkit::ScrollCapsuleY);

		auto pInnerPanel = WGCREATE(PackPanel, 
			_.axis = Axis::Y, 
			_.layout = PackLayout::create({}), 
			_.skin = WGCREATE(ColorSkin, _.color = HiColor::Transparent, _.padding = 3 ));

		pInnerPanel->slots << _createObjectHeader(pObject);

		m_pContext->addObjectInfoSections(pInnerPanel, pObject);

		pScrollPanel->slot = pInnerPanel;

		return pScrollPanel;
	}

} // namespace wg
