/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Bärnfors's WonderGUI Graphics Toolkit
  and copyright (c) Bärnfors Technology AB, Sweden [info@barnfors.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact us [info@barnfors.com] for details.

=========================================================================*/

#include <wg_debugfrontend.h>
#include <wg_debugcapsule.h>
#include <wg_msgrouter.h>

#include <wondergui.h>

#include <wg_basictextlayout.h>
#include <wg_basicnumberlayout.h>
#include <wg_blockskin.h>
#include <wg_colorskin.h>

namespace wg
{
	using namespace Util;

	const TypeInfo DebugFrontend::TYPEINFO = { "DebugFrontend", &Capsule::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugFrontend::DebugFrontend(const Blueprint& bp) : Capsule(bp)
	{
		m_pBackend = bp.backend;
		m_pTheme 	= bp.theme;
		m_pIcons	= bp.icons;
		m_pTransparencyGrid = bp.transparencyGrid;

		_createDebuggerBP();
		m_pBackend->setBlueprint(m_debugPanelBP);

		m_pBackend->setObjectSelectedCallback([this](Object* pSelected,Object* pSelectedFrom) {

			selectObject(pSelected, pSelectedFrom);
		});

		_createResources();
		_setupGUI();
	}

	//____ Destructor _____________________________________________________________

	DebugFrontend::~DebugFrontend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugFrontend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ activate() ____________________________________________________________

	void DebugFrontend::activate()
	{
		for (auto pCapsule : m_capsules)
		{
			pCapsule->_setSelectMode(m_bSelectMode);
			pCapsule->_widgetSelected(m_pSelectedWidget);
		}

		if (m_pListOfTreeViews->slots.isEmpty())
			_refreshWidgetTree();
	}

	//____ deactivate() __________________________________________________________

	void DebugFrontend::deactivate()
	{
		for (auto pCapsule : m_capsules)
		{
			pCapsule->_setSelectMode(m_bSelectMode);
			pCapsule->_widgetSelected(nullptr);
		}
	}


	//____ _addDebugCapsule() _____________________________________________________

	void DebugFrontend::_addDebugCapsule( DebugCapsule * pCapsule )
	{
		pCapsule->_setSelectMode(m_bSelectMode);
		m_capsules.push_back(pCapsule);

//		_refreshTreeSelector();
	}

	//____ _removeDebugCapsule() __________________________________________________

	void DebugFrontend::_removeDebugCapsule( DebugCapsule * pCapsule )
	{
		m_capsules.erase(std::remove(m_capsules.begin(), m_capsules.end(), pCapsule), m_capsules.end());

//		_refreshTreeSelector();
	}

	//____ selectObject() _______________________________________________________

	void DebugFrontend::selectObject(Object* pSelected, Object * pSelectedFrom)
	{
		auto pWidget = dynamic_cast<Widget*>(pSelected);
		if(pWidget || pSelected == nullptr)
		{
			for( auto pCapsule : m_capsules )
				pCapsule->_widgetSelected(pWidget);

			for (auto& slot : m_pListOfTreeViews->slots)
			{
				auto pTreeView = wg_dynamic_cast<WidgetTreeView_p>(slot.widget());
				if (pTreeView)
					pTreeView->select(pWidget);
			}

			m_pSelectedWidget = pWidget;
		}

		if (pSelected)
		{
			bool bReuseWindow = (Base::inputHandler()->modifierKeys() & ModKeys::Shift) == 0;
			_addWorkspaceWindow(pSelected, bReuseWindow);
		}
		else
			_focusWorkspaceWindow(nullptr);

	}

	//____ setSelectMode() _______________________________________________________

	void DebugFrontend::setSelectMode(bool selectMode)
	{
		if(selectMode != m_bSelectMode)
		{
			m_bSelectMode = selectMode;
			for( auto capsule : m_capsules )
				capsule->_setSelectMode(selectMode);
		}
	}

	//____ _addWorkspaceWindow() _________________________________________________

	void DebugFrontend::_addWorkspaceWindow( Object * pObject, bool bReuse )
	{

		auto pWidget = dynamic_cast<Widget*>(pObject);
		auto pSkin = dynamic_cast<Skin*>(pObject);

		DebugFrontendWindow_p pWindow = _findWorkspaceWindow(pObject);

		if (pWindow)
		{
			_focusWorkspaceWindow(pWindow);
			return;
		}

		if( bReuse )
		{
			const TypeInfo* pWantedTypeInfo = &ObjectInspector::TYPEINFO;

			if( pWidget)
				pWantedTypeInfo = &WidgetInspector::TYPEINFO;
			else if( pSkin )
				pWantedTypeInfo = &SkinInspector::TYPEINFO;

			// Try to find an existing window for this object.
			for( auto& slot : m_pWorkspace->slots )
			{
				auto pDebugWin = wg_dynamic_cast<DebugFrontendWindow_p>(slot.widget());

				if( pDebugWin && &pDebugWin->content()->typeInfo() == pWantedTypeInfo )
				{
					pWindow = pDebugWin;
					break;
				}
			}
		}

		if( !pWindow )
		{
			pWindow = WGCREATE(DebugFrontendWindow, _.theme = m_pTheme);
			m_pWorkspace->slots.pushBack(pWindow, WGBP(PackPanelSlot, _.weight = 0.f));
		}

		DebugWindow_p pContent;

		if( pWidget )
		{
			pContent = m_pBackend->createWidgetInspector(pWidget);
//			pWindow->setLabel("Widget");
		}
		else if (pSkin)
		{
			pContent = m_pBackend->createSkinInspector(pSkin);
//			pWindow->setLabel("Skin");
		}
		else
			pContent = m_pBackend->createObjectInspector(pObject);

		pWindow->setContent(pContent);
		pWindow->setInspected(pObject);
		pWindow->setLabel(pContent->title());

		_focusWorkspaceWindow(pWindow);
	}

	//____ _findWorkspaceWindow() _____________________________________________

	DebugFrontendWindow_p DebugFrontend::_findWorkspaceWindow(Object* pObjectInspected)
	{
		for (auto& slot : m_pWorkspace->slots)
		{
			auto pWindow = dynamic_cast<DebugFrontendWindow*>(slot._widget());
			if (pWindow->inspected() == pObjectInspected)
				return pWindow;
		}

		return nullptr;
	}



	//____ _focusWorkspaceWindow() _____________________________________________

	bool DebugFrontend::_focusWorkspaceWindow(DebugFrontendWindow* pWindow)
	{
		bool bFound = false;

		for (auto& slot : m_pWorkspace->slots)
		{
			auto p = dynamic_cast<DebugFrontendWindow*>(slot._widget());
			if (p == pWindow)
			{
				p->setFocused(true);
				p->bringIntoView();
				bFound = true;
			}
			else
				p->setFocused(false);
		}

		return bFound;
	}


	//____ _createResources() ____________________________________________________

	void DebugFrontend::_createResources()
	{
		m_pDummyPackLayout = PackLayout::create({});

		m_pRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(0, 0, 16, 16);
			));

		m_pUnselectIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(0, 64, 16, 16);
		));

		m_pSelectIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(16, 0, 16, 16);
		));

		m_pExpandIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(32, 0, 16, 16);
		));

		m_pCondenseIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(48, 0, 16, 16);
		));


	}

	//____ _setupGUI() ___________________________________________________________

	void DebugFrontend::_setupGUI()
	{
		auto pMainPanel = WGCREATE(PackPanel, _.axis = Axis::Y );
		auto pTopBar 	= WGCREATE(PackPanel, _.axis = Axis::X, _.skin = m_pTheme->plateSkin() );
		auto pLogSplit = WGCREATE(SplitPanel, _ = m_pTheme->splitPanelY() );
		auto pTreeSplit = WGCREATE(SplitPanel, _ = m_pTheme->splitPanelX() );

		pMainPanel->slots.pushBack({ {	pTopBar, WGBP(PackPanelSlot, _.weight = 0.f)},
										pLogSplit});

		auto pWorkspaceScroller = WGCREATE(ScrollPanel, _ = m_pTheme->scrollPanelX() );

		auto pWorkspaceReorder = WGCREATE(ReorderCapsule, _.dragOutside = false, _.usePickHandles = true );
		pWorkspaceScroller->slot = pWorkspaceReorder;

		auto pWorkspace = WGCREATE(PackPanel, _.axis = Axis::X, _.skin = ColorSkin::create( Color::Navy ));
		pWorkspaceReorder->slot = pWorkspace;

		//

		pTreeSplit->slots[0] = _createWidgetTreeView();
		pTreeSplit->slots[1] = pWorkspaceScroller;

		pLogSplit->slots[0] = pTreeSplit;
		pLogSplit->slots[1] = m_pBackend->createMsgLogViewer();


		pTopBar->slots.pushBack( _createToolbox(), WGBP(PackPanelSlot, _.weight = 0.f));

//		pMainPanel->slots.pushBack(pWorkspaceReorder);


		m_pWorkspace = pWorkspace;
		slot = pMainPanel;
	}

	//____ _createToolbox() ______________________________________________________

	Widget_p DebugFrontend::_createToolbox()
	{
		auto pToolbox = PackPanel::create(WGBP(PackPanel, _.axis = Axis::X));

		auto pPickButton = ToggleButton::create(WGOVR(m_pTheme->toggleButton(), _.icon.skin = m_pSelectIcon, _.icon.placement = Placement::Center));

//		m_pPickWidgetButton = pPickButton;

		Base::msgRouter()->addRoute(pPickButton, MsgType::Toggle, [this](Msg* pMsg) {

			auto pButton = wg_static_cast<ToggleButton_p>(pMsg->source());
			setSelectMode(pButton->isChecked());
		});

		auto pUnselectButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pUnselectIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pUnselectButton, MsgType::Select, [this](Msg* pMsg) {

			selectObject(nullptr, nullptr);
		});

		pToolbox->slots.pushBack({ pPickButton, pUnselectButton });

		return pToolbox;
	}

	//____ _refreshWidgetTree() __________________________________________________

	void DebugFrontend::_refreshWidgetTree()
	{
		m_pListOfTreeViews->slots.clear();

		for (auto pCapsule : m_capsules)
		{
			if (pCapsule->slot.isEmpty())
				continue;

			auto pTreeView = m_pBackend->createWidgetTreeView(pCapsule);
			this->m_pListOfTreeViews->slots << pTreeView;
			pTreeView->select(m_pSelectedWidget);

		}
	}

	//____ _createWidgetTreeView() ____________________________________________

	Widget_p DebugFrontend::_createWidgetTreeView()
	{
		auto pTreePanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		m_pListOfTreeViews = WGCREATE(PackPanel, _.axis = Axis::Y, _.skin = m_pTheme->canvasSkin(), _.layout = m_pDummyPackLayout );

		auto pTreeScrollPanel = WGCREATE(ScrollPanel, _ = m_pTheme->scrollPanelXY());
		pTreeScrollPanel->slot = m_pListOfTreeViews;

		// Create button row with refresh, collapse and expand buttons

		auto pButtonRow = PackPanel::create(WGBP(PackPanel, _.axis = Axis::X, _.skin = m_pTheme->plateSkin() ));

		auto pRefreshButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pRefreshIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pRefreshButton, MsgType::Select, [this](Msg* pMsg) {
			_refreshWidgetTree();
		});

		auto pCollapseAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pCondenseIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pCollapseAllButton, MsgType::Select, [this](Msg* pMsg) {

			for( auto& slot : m_pListOfTreeViews->slots )
			{
				auto pTreeView = wg_dynamic_cast<WidgetTreeView_p>(slot.widget());
				if (pTreeView)
					pTreeView->collapseAll();
			}
		});

		auto pExpandAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pExpandIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pExpandAllButton, MsgType::Select, [this](Msg* pMsg) {

			for (auto& slot : m_pListOfTreeViews->slots)
			{
				auto pTreeView = wg_dynamic_cast<WidgetTreeView_p>(slot.widget());
				if (pTreeView)
					pTreeView->expandAll();
			}
			});


		pButtonRow->slots.pushBack({ pRefreshButton, pCollapseAllButton, pExpandAllButton });
		pButtonRow->setSlotWeight(pButtonRow->slots.begin(), pButtonRow->slots.end(), 0.f); // Set all buttons to not expand


		//


		pTreePanel->slots.pushBack(pButtonRow, WGBP(PackPanelSlot, _.weight = 0.f));
		pTreePanel->slots.pushBack(pTreeScrollPanel);

		return pTreePanel;
	}

	//____ _createDebuggerBP() ___________________________________________________

	void DebugFrontend::_createDebuggerBP()
	{
		m_debugPanelBP.theme = m_pTheme;
		m_debugPanelBP.icons = m_pIcons;
		m_debugPanelBP.transparencyGrid = m_pTransparencyGrid;

		auto pListTextLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.placement = Placement::East ));

		auto pInfoLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.wrap = true,
														  _.placement = Placement::Center ));

		auto pWrapTextLayout = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.wrap = true,
			_.placement = Placement::NorthWest));

		auto pValueLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 2
		));

		auto pIntegerLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 0
		));


		CharBuffer chrBuff;
		chrBuff.pushBack("0x");
		chrBuff.setStyle(m_pTheme->defaultStyle());

		auto pPointerLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.base = 16,
			_.integerGrouping = 0,
			_.prefix = String(&chrBuff)
			));

		auto pPtsLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 2
		));



		m_debugPanelBP.classCapsule = WGBP(LabelCapsule,
			_.skin = ColorSkin::create(HiColor::Transparent, { 10,0,0,8 }),
			_.label.style = m_pTheme->finePrintStyle()
		);


		m_debugPanelBP.listEntryLabel = WGBP(TextDisplay,
											 _.display.style = m_pTheme->strongStyle() );

		m_debugPanelBP.listEntryText = WGBP(TextDisplay,
											 _.display.style = m_pTheme->defaultStyle(),
											 _.display.layout = pListTextLayout );

		m_debugPanelBP.listEntryInteger = WGBP(NumberDisplay,
											 _.display.layout = pIntegerLayout );

		m_debugPanelBP.listEntryBool = WGBP(NumberDisplay,
											_.display.layout = pValueLayout);

		m_debugPanelBP.listEntrySPX = WGBP(NumberDisplay,
											 _.display.layout = pIntegerLayout );

		m_debugPanelBP.listEntryPts = WGBP(NumberDisplay,
											 _.display.layout = pPtsLayout );

		m_debugPanelBP.listEntryDecimal = WGBP(NumberDisplay,
											 _.display.layout = pValueLayout );

//		m_debugPanelBP.listEntryPointer = WGBP(NumberDisplay,
//											 _.display.layout = pPointerLayout );

		m_debugPanelBP.listEntryDrawer = m_pTheme->treeListDrawer();
		m_debugPanelBP.selectableListEntryCapsule = WGOVR( m_pTheme->treeListEntry(), _.selectable = true );

		m_debugPanelBP.textField = WGBP(TextDisplay,
			_.display.style = m_pTheme->defaultStyle(),
			_.display.layout = pWrapTextLayout,
			_.skin = m_pTheme->canvasSkin() );

		m_debugPanelBP.infoDisplay = WGBP(TextDisplay,
											 _.display.style = m_pTheme->emphasisStyle(),
											 _.display.layout = pInfoLayout );

		m_debugPanelBP.table = WGBP(TablePanel,
									_.columnLayout = Base::defaultPackLayout());

	}


} // namespace wg

