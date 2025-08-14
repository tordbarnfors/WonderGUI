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
#include "wg_widgettreeview.h"
#include <wg_textdisplay.h>
#include <wg_packpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_drawerpanel.h>
#include <wg_selectcapsule.h>
#include <wg_filler.h>

#include <wg_boxskin.h>
#include <wg_colorskin.h>

#include <wg_msgrouter.h>

namespace wg
{

	const TypeInfo WidgetTreeView::TYPEINFO = { "WidgetTreeView", &DebugWindow::TYPEINFO };


	//____ constructor _____________________________________________________________

	WidgetTreeView::WidgetTreeView(const Blueprint& blueprint, IDebugger * pHolder, Widget * pRoot ) : DebugWindow( blueprint, pHolder )
	{
		m_pPackLayout = PackLayout::create(WGBP(PackLayout, ));
		 
		m_pSelectCapsule = SelectCapsule::create(WGBP(SelectCapsule, _.recursive = true ));

		if( pRoot )
			m_pSelectCapsule->slot = _generateInfoTree(blueprint, pRoot );
		else
			m_pSelectCapsule->slot = Filler::create( WGBP(Filler, _.skin = blueprint.theme->canvasSkin() ));

		m_routeIdForSelect = Base::msgRouter()->addRoute(m_pSelectCapsule, MsgType::Selected, [this](Msg* pMsg) {
		
			Widget* pWidget = static_cast<SelectedMsg*>(pMsg)->selected()[0];

			auto pSelected = m_realWidgets[pWidget->id()];

			if (pSelected)
			{
				m_pHolder->objectSelected(pSelected,this);
				m_pSelectedWidget = pSelected;
			}
		});

		this->slot = m_pSelectCapsule;

		this->setSkin(nullptr);
	}

	//____ Destructor ______________________________________________________________

	WidgetTreeView::~WidgetTreeView()
	{
		Base::msgRouter()->deleteRoute(m_routeIdForSelect);
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& WidgetTreeView::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ collapseAll() _________________________________________________________

	void WidgetTreeView::collapseAll()
	{
		_expandOrCollapseRecursively(slot.widget(), false);
	}

	//____ expandAll() __________________________________________________________

	void WidgetTreeView::expandAll()
	{
		_expandOrCollapseRecursively(slot.widget(), true);
	}

	//____ select() __________________________________________________________

	void WidgetTreeView::select(Widget* pWidget)
	{
		if( pWidget == m_pSelectedWidget )
			return;

		if (pWidget != nullptr)
		{
			auto it = std::find(m_realWidgets.begin(), m_realWidgets.end(), pWidget);
			if (it != m_realWidgets.end())
			{
				int id = std::distance(m_realWidgets.begin(), it);

				auto pFound = _findWidgetRecursively(id, slot.widget());

				m_pSelectCapsule->select(pFound);
				if (pFound)
				{
					// The widget we want to bring into view is inside a PaddingCapsule and
					// has the full width given. We only want to bring the visible text into view,
					// so we need to calculate the area of that.

					auto pContent = static_cast<Container*>(pFound.rawPtr())->firstChild();
					Size contentSize = pContent->size();

					contentSize.w = pContent->matchingWidth(contentSize.h);
					pContent->bringIntoView({ 0,0,contentSize }, Border(0,0,0,16) );			// With left margin for any drawer expander button.

				}

				m_pSelectedWidget = pWidget;
				return;
			}

		}

		m_pSelectedWidget = nullptr;
		m_pSelectCapsule->unselectAll();
		return;

	}

	//____ _findWidgetRecursively() ___________________________________________

	Widget_p WidgetTreeView::_findWidgetRecursively(int idToFind, Widget * pPos)
	{
		if( pPos == nullptr )
			return nullptr;

		if (pPos->id() == idToFind)
			return pPos;
		else if (pPos->isContainer())
		{
			auto pContainer = static_cast<Container*>(pPos);
			auto pChild = pContainer->firstChild();
			while (pChild)
			{
				auto pFound = _findWidgetRecursively(idToFind, pChild);
				if (pFound)
					return pFound;
				pChild = pChild->nextSibling();
			}
		}

		return nullptr;
	}

	//____ _expandOrCollapseRecursively() _________________________________________________

	void WidgetTreeView::_expandOrCollapseRecursively( Widget * pWidget, bool bOpen )
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

	Widget_p WidgetTreeView::_generateInfoTree(const Blueprint& blueprint, Widget* pWidget, int indentation)
	{
		if (pWidget->isContainer())
		{
			auto pDrawer = DrawerPanel::create( WGOVR(blueprint.listEntryDrawer, 
				_.buttonOfs.x += pts(indentation * 16)
			));

			auto pNameDisplay = TextDisplay::create(WGOVR(blueprint.listEntryLabel, _.display.text = pWidget->typeInfo().className));

			auto pEntry = PaddingCapsule::create(WGOVR(blueprint.selectableListEntryCapsule,
				_.padding.left += pts((indentation+1) *16),
				_.child = pNameDisplay
			));

			auto pContent = PackPanel::create(WGBP(PackPanel,
				_.axis = Axis::Y,
				_.layout = m_pPackLayout
			));


			pDrawer->slots[0] = pEntry;
			pDrawer->slots[1] = pContent;

			pEntry->setId(m_realWidgets.size());
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

			auto pEntry = PaddingCapsule::create(WGOVR(blueprint.selectableListEntryCapsule,
				_.padding.left += pts((indentation + 1) * 16),
				_.child = pNameDisplay
			));

			pEntry->setId(m_realWidgets.size());
			m_realWidgets.push_back(pWidget);


			return pEntry;
		}
	}


} // namespace wg


