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
#include "wg_widgettreepanel.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_drawerpanel.h>
#include <wg_selectcapsule.h>

#include <wg_boxskin.h>

#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo WidgetTreePanel::TYPEINFO = { "WidgetTreePanel", &DebugPanel::TYPEINFO };


	//____ constructor _____________________________________________________________

	WidgetTreePanel::WidgetTreePanel(const Blueprint& blueprint, Widget * pRoot, const std::function<void(Widget* pSelected)>& selectCallback ) : DebugPanel( blueprint )
	{
		m_selectCallback = selectCallback;

		m_pPaddingSkin = BoxSkin::create(WGBP(BoxSkin,
			_.markAlpha = 0,
			_.states = { {State::Default, Color::Transparent,Color::Transparent},
						 {State::Hovered, HiColor(Color::LightCyan).withAlpha(1024), HiColor(Color::DarkCyan).withAlpha(1024)},
						 {State::Selected, Color::LightCyan,Color::DarkCyan }
			}
		));

		m_pPackLayout = PackLayout::create(WGBP(PackLayout, ));

		m_pTransition = ValueTransition::create(250000);

		auto pSelectCapsule = SelectCapsule::create(WGBP(SelectCapsule, _.recursive = true ));

		pSelectCapsule->slot = _generateInfoTree(blueprint, pRoot );

		Base::msgRouter()->addRoute(MsgType::Selected, [this](Msg* pMsg) {
		
			Widget* pWidget = static_cast<SelectedMsg*>(pMsg)->selected()[0];

			auto pSelected = m_realWidgets[pWidget->id()];

			if (pSelected)
			{
				m_selectCallback(pSelected);
			}
		});

		this->slot = pSelectCapsule;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& WidgetTreePanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ collapseAll() _________________________________________________________

	void WidgetTreePanel::collapseAll()
	{
		_expandOrCollapseRecursively(slot.widget(), false);
	}

	//____ expandAll() __________________________________________________________

	void WidgetTreePanel::expandAll()
	{
		_expandOrCollapseRecursively(slot.widget(), true);
	}

	//____ _expandOrCollapseRecursively() _________________________________________________

	void WidgetTreePanel::_expandOrCollapseRecursively( Widget * pWidget, bool bOpen )
	{
		if (pWidget == nullptr)
			return;

		if (pWidget->typeInfo() == DrawerPanel::TYPEINFO)
		{
			auto pDrawer = static_cast<DrawerPanel*>(pWidget);

			auto pPackPanel = static_cast<PackPanel*>(pDrawer->slots[1]._widget());

			for (auto& slot : pPackPanel->slots)
				_expandOrCollapseRecursively(slot._widget(), bOpen);

			pDrawer->setOpen(bOpen);
		}
		else if (pWidget->isContainer())
		{
			auto pChild = static_cast<Container*>(pWidget)->firstChild();
			while (pChild)
			{
				_expandOrCollapseRecursively(pChild, bOpen);
				pChild = pChild->nextSibling();
			}
		}
	}


	//____ generateInfoTree() _________________________________________________

	Widget_p WidgetTreePanel::_generateInfoTree(const Blueprint& blueprint, Widget* pWidget, int indentation)
	{
		if (pWidget->isContainer())
		{
			auto pDrawer = DrawerPanel::create( WGBP(DrawerPanel, _.transition = m_pTransition ));

			auto pNameDisplay = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = pWidget->typeInfo().className));


			auto pPadder = PaddingCapsule::create(WGBP(PaddingCapsule,
				_.selectable = true,
				_.padding = Border{ 0.f,0.f,0.f,indentation * 16.f },
				_.skin = m_pPaddingSkin,
				_.child = pNameDisplay
			));

			auto pContent = PackPanel::create(WGBP(PackPanel,
				_.axis = Axis::Y,
				_.layout = m_pPackLayout
			));


			pDrawer->slots[0] = pPadder;
			pDrawer->slots[1] = pContent;

			pPadder->setId(m_realWidgets.size());
			m_realWidgets.push_back(pWidget);

			auto pChild = static_cast<Container*>(pWidget)->firstChild();
			while (pChild)
			{
				pContent->slots.pushBack(_generateInfoTree(blueprint, pChild, indentation + 1) );
				pChild = pChild->nextSibling();
			}

			pContent->setSlotWeight(pContent->slots.begin(), pContent->slots.end(), 0.f);

			return pDrawer;
		}
		else
		{
			auto pNameDisplay = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = pWidget->typeInfo().className));


			auto pEntry = PaddingCapsule::create(WGBP(PaddingCapsule,
				_.selectable = true,
				_.padding = { 0.f,0.f,0.f,indentation * 16.f },
				_.skin = m_pPaddingSkin,
				_.child = pNameDisplay
			));

			pEntry->setId(m_realWidgets.size());
			m_realWidgets.push_back(pWidget);


			return pEntry;
		}
	}


} // namespace wg


