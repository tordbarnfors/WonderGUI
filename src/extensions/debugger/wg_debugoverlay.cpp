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

#include <wg_debugoverlay.h>
#include <wg_util.h>
#include <wg_patches.h>
#include <wg_msgrouter.h>
#include <wg_base.h>
#include <wg_inputhandler.h>
#include <wg_colorskin.h>
#include <wg_boxskin.h>
#include <wg_blockskin.h>
#include <wg_packpanel.h>
#include <wg_tablepanel.h>
#include <wg_numberdisplay.h>
#include <wg_textdisplay.h>
#include <wg_gfxdevice.h>
#include <wg_togglebutton.h>
#include <wg_togglegroup.h>
#include <wg_scrollpanel.h>
#include <wg_basictextlayout.h>
#include <wg_basicnumberlayout.h>

#include <wg_staticslotvector.impl.h>

#include <windows/wg_widgettreeview.h>

#include <string>

namespace wg
{
	using namespace Util;

	template class StaticSlotVector<DebugOverlay::WindowSlot>;

	const TypeInfo DebugOverlay::TYPEINFO = { "DebugOverlay", &Overlay::TYPEINFO };
	const TypeInfo DebugOverlay::WindowSlot::TYPEINFO = { "DebugOverlay::WindowSlot", &Overlay::Slot::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugOverlay::DebugOverlay(const Blueprint& bp) : Overlay(bp), windows(this)
	{
		m_pDebugger = bp.debugger;
		m_pTheme 	= bp.theme;
		m_pIcons	= bp.icons;
		m_pTransparencyGrid = bp.transparencyGrid;

		_createResources();

		// Create default skins

		m_pSelectionSkin = BoxSkin::create(1, Color::Transparent, Color::Red, 1);

		_createToolboxWindow();
		_createWidgetTreeWindow();
		_createWidgetInfoWindow();
		_createSkinInfoWindow();
		_createObjectInfoWindow();
		_createMsgLogWindow();

		m_pDebugger->setObjectSelectedCallback([this](Object* pSelected,Object* pSelectedFrom) {

			auto pWidget = dynamic_cast<Widget*>(pSelected);
			if (pWidget)
			{
				_selectWidget(pWidget); 
				return;
			}

			auto pSkin = dynamic_cast<Skin*>(pSelected);
			if( pSkin )
			{
				_selectSkin(pSkin);
				return;
			}

			if( pSelected )
				_selectObject(pSelected, pSelectedFrom);
		});
	}

	//____ Destructor _____________________________________________________________

	DebugOverlay::~DebugOverlay()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugOverlay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setWindowSkin() ___________________________________________________

	void DebugOverlay::setWindowSkin(Skin * pSkin)
	{
		m_pWindowSkin = pSkin;
		_requestRender();
	}

	//____ setSelectionSkin() _________________________________________________

	void DebugOverlay::setSelectionSkin(Skin * pSkin)
	{
		m_pSelectionSkin = pSkin;
		_requestRender();
	}

	//____ pointerStyle() ________________________________________________________

	PointerStyle DebugOverlay::pointerStyle() const
	{
		if( m_bActivated && m_generatedPointerStyle != PointerStyle::Undefined )
			return m_generatedPointerStyle;
		else
			return Overlay::pointerStyle();
	}

	//____ setActivated() ______________________________________________________

	void DebugOverlay::setActivated(bool bActivated)
	{
		if (bActivated != m_bActivated)
		{
			m_bActivated = bActivated;
			_requestRender();
		}
	}

	//____ _matchingHeight() _______________________________________________________

	spx DebugOverlay::_matchingHeight(spx width, int scale) const
	{
		if (mainSlot._widget())
			return mainSlot._widget()->_matchingHeight(width, scale);
		else
			return Widget::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() _______________________________________________________

	spx DebugOverlay::_matchingWidth(spx height, int scale) const
	{
		if (mainSlot._widget())
			return mainSlot._widget()->_matchingWidth(height,scale);
		else
			return Widget::_matchingWidth(height,scale);
	}

	//____ defaultSize() _____________________________________________________________

	SizeSPX DebugOverlay::_defaultSize(int scale) const
	{
		if (mainSlot._widget())
			return mainSlot._widget()->_defaultSize(scale);
		else
			return SizeSPX(1, 1);
	}

	//____ _findWidget() ____________________________________________________________

	Widget *  DebugOverlay::_findWidget(const CoordSPX& ofs, SearchMode mode)
	{
		if (m_bActivated)
		{
			for (auto& slot : windows)
				if (slot.m_bVisible && slot.m_geo.contains(ofs) )
				{
					return static_cast<Container*>(slot._widget())->_findWidget(ofs - slot.m_geo.pos(), mode);
				}

			if( m_bSelectMode)
				return this;
		}

		return Container::_findWidget(ofs, mode);
	}

	//____ _render() __________________________________________________________

	void DebugOverlay::_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		// Render skin

		m_skin.render(pDevice, _canvas, m_scale, m_state);

		RectSPX contentRect = m_skin.contentRect(_canvas, m_scale, m_state);

		if (!mainSlot.isEmpty())
		{
			mainSlot._widget()->_render(pDevice, contentRect, contentRect);
		}

		if (m_bActivated)
		{
			if (m_pSelectedWidget)
				m_pSelectionSkin->_render(pDevice, _selectionGeo(), m_scale, State::Default);

			for ( int i = windows.size()-1 ; i >= 0 ; i-- )
			{
				auto& window = windows[i];
				if (window.m_bVisible)
				{
					RectSPX geo = window.m_geo + _canvas.pos();

					RectSPX pxWindowGeo = window.m_geo;
					if (pDevice->clipBounds().isOverlapping(pxWindowGeo))
					{
						ClipPopData popData = limitClipList(pDevice, window.m_geo);
						window._widget()->_render(pDevice, geo, geo);

						popClipList(pDevice, popData);
					}
				}
			}

		}

	}

	//____ _selectionGeo() ____________________________________________________

	RectSPX DebugOverlay::_selectionGeo() const
	{
		if (!m_pSelectedWidget)
			return RectSPX();

		RectSPX selectedGeo = _toLocal(m_pSelectedWidget->_globalGeo());

		if (m_pSelectionSkin)
			selectedGeo += m_pSelectionSkin->_contentBorder(m_scale, State::Default);

		return selectedGeo;
	}


	//____ _refreshRealGeo() __________________________________________________

	void DebugOverlay::_refreshRealGeo(WindowSlot * pSlot, bool bForceResize)
	{
		SizeSPX windowSize = pSlot->m_chosenSize.isEmpty() ? pSlot->_widget()->_defaultSize(pSlot->_widget()->_scale()) : pSlot->m_chosenSize;

		windowSize.limit({ 100*64,50*64 }, _size());

		RectSPX childGeo = Util::placementToRect(pSlot->m_placement, _size(), windowSize);

		childGeo += pSlot->m_placementPos;

		if (childGeo.x + childGeo.w > m_size.w)
			childGeo.x = m_size.w - childGeo.w;

		if (childGeo.y + childGeo.h > m_size.h)
			childGeo.y = m_size.h - childGeo.h;


		if (childGeo != pSlot->m_geo)
		{
			_requestRender(childGeo);
			_requestRender(pSlot->m_geo);

			pSlot->m_geo = childGeo;
		}

		if (pSlot->_widget()->_size() != childGeo.size() || bForceResize )
			pSlot->_setSize(childGeo, m_scale);
	}

	//____ _selectWidget() ____________________________________________________

	void DebugOverlay::_selectWidget(Widget * pWidget)
	{
		if (pWidget == m_pSelectedWidget)
			return;

		// Update selectionSkin

		if (m_pSelectionSkin)
		{
			if (m_pSelectedWidget)
			{
				RectSPX geo = _toLocal(m_pSelectedWidget->_globalGeo());
				geo += m_pSelectionSkin->_contentBorder(m_scale, State::Default);
				_requestRender(geo);
			}


			if (pWidget)
			{
				RectSPX geo = _toLocal(pWidget->_globalGeo());
				geo += m_pSelectionSkin->_contentBorder(m_scale, State::Default);
				_requestRender(geo);
			}
		}

		m_pSelectedWidget = pWidget;

		// Update windows

		if (m_pSelectedWidget)
		{
			m_pWidgetTools->slots.clear();
			m_pWidgetTools->slots << m_pDebugger->createObjectInspector(m_debugPanelBP,m_pSelectedWidget);

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreeView*>(m_pWidgetTreeContainer->slot._widget())->select(m_pSelectedWidget);
/*
			for (auto& window : windows)
			{
				window.m_bVisible = true;
				_refreshRealGeo(&window);
			}
 */
		}
		else
		{
			for (auto& window : windows)
			{
				if (window.m_bVisible)
				{
					_requestRender(window.m_geo);
					window.m_bVisible = false;
				}
			}
		}

		// 


	}

	//____ _selectSkin() ____________________________________________________

	void DebugOverlay::_selectSkin(Skin* pSkin)
	{
		m_pSkinContainer->slots.clear();

		m_pSkinContainer->slots << m_pDebugger->createSkinInspector(m_debugPanelBP, pSkin);
	}

	//____ _selectObject() ____________________________________________________

	void DebugOverlay::_selectObject(Object* pSelected, Object* pSelectedFrom)
	{
		m_pAnyObjectContainer->slots.clear();

		m_pAnyObjectContainer->slots << m_pDebugger->createObjectInspector(m_debugPanelBP, pSelected);
	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	DebugOverlay::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return WindowSlot::TYPEINFO;
	}

	//____ _childRequestResize() ______________________________________________

	void DebugOverlay::_childRequestResize( StaticSlot * pSlot )
	{
		if( pSlot == &mainSlot )
			_requestResize();
		else
		{
			auto p = static_cast<WindowSlot*>(pSlot);
			_refreshRealGeo( p, true );
		}
	}

	//____ _releaseChild() ____________________________________________________

	void DebugOverlay::_releaseChild(StaticSlot * pSlot)
	{
		if (pSlot == &mainSlot )
			Overlay::_releaseChild(pSlot);
		else
		{
			auto p = static_cast<WindowSlot*>(pSlot);

			if (p->m_bVisible)
				_requestRender(p->m_geo);
			windows._erase(p);
		}
	}

	//____ _beginOverlaySlots() __________________________________________________

	const Overlay::Slot * DebugOverlay::_beginOverlaySlots() const
	{
		return windows._begin();
	}

	//____ _endOverlaySlots() ____________________________________________________

	const Overlay::Slot *  DebugOverlay::_endOverlaySlots() const
	{
		if( m_bActivated )
			return windows._end();
		else
			return windows._begin();
	}

	//____ _sizeOfOverlaySlot() __________________________________________________

	int DebugOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(WindowSlot);
	}


	//____ _resize() ___________________________________________________________

	void DebugOverlay::_resize( const SizeSPX& sz, int scale )
	{
		Overlay::_resize(sz, scale);

		// Refresh modal widgets geometry, their positions might have changed.

		for( auto pSlot = windows._begin() ; pSlot != windows._end() ; pSlot++ )
			_refreshRealGeo( pSlot, true );
	}

	//____ _receive() ______________________________________________________________

	void DebugOverlay::_receive( Msg * _pMsg )
	{
		// Check if it is our enable/disable toggle

		if( _pMsg->isInstanceOf( KeyMsg::TYPEINFO ) && static_cast<KeyMsg*>(_pMsg)->translatedKeyCode() == Key::Escape && (static_cast<KeyMsg*>(_pMsg)->modKeys() == ModKeys::StdCtrl || static_cast<KeyMsg*>(_pMsg)->modKeys() == ModKeys::MacCtrl) )
		{
			if( _pMsg->typeInfo() == KeyPressMsg::TYPEINFO )
				setActivated(!m_bActivated);

			_pMsg->swallow();
			Overlay::_receive(_pMsg);
			return;
		}

		//

		if( !m_bActivated )
		{
			Overlay::_receive(_pMsg);
			return;
		}

		//

		switch( _pMsg->type() )
		{

			case MsgType::MouseMove:
			{
				auto pMsg = static_cast<MouseMoveMsg*>(_pMsg);

				auto pWidget = static_cast<Widget*>(_pMsg->originalSourceRawPtr());

				if( !mainSlot.isEmpty() && pWidget->isDescendantOf(mainSlot._widget() ))
				{
					m_generatedPointerStyle = PointerStyle::Undefined;
					break;																	// Not from our windows.
				}

				if( pWidget == this )
				{
					m_generatedPointerStyle = m_bSelectMode ? PointerStyle::Crosshair : PointerStyle::Undefined;
					break;
				}

				int id = pWidget->id();

				if( id < 10000 )
				{
					m_generatedPointerStyle = PointerStyle::Undefined;
					break;
				}

				int	windowIndex = _windowIndex(pWidget);
				int meaning = id % 1000;

				if( meaning == 1 )
				{
					CoordSPX mousePos = _toLocal(pMsg->pointerSpxPos());

					auto section = _windowFrameSection( mousePos, windowIndex );
					switch( section )
					{
						case Placement::North:
						case Placement::South:
							m_generatedPointerStyle = PointerStyle::ResizeNS;
							break;

						case Placement::East:
						case Placement::West:
							m_generatedPointerStyle = PointerStyle::ResizeWE;
							break;

						case Placement::SouthEast:
						case Placement::NorthWest:
							m_generatedPointerStyle = PointerStyle::ResizeNwSe;
							break;

						case Placement::NorthEast:
						case Placement::SouthWest:
							m_generatedPointerStyle = PointerStyle::ResizeNeSw;
							break;

						default:
							m_generatedPointerStyle = PointerStyle::Undefined;
							break;
					}
				}
				else if( meaning == 2 )
				{
					m_generatedPointerStyle = m_movingWindow >= 0 ? PointerStyle::ClosedHand : PointerStyle::OpenHand;
				}

				break;
			}


			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);


				auto pWidget = static_cast<Widget*>(_pMsg->originalSourceRawPtr());

				if( !mainSlot.isEmpty() && pWidget->isDescendantOf(mainSlot._widget() ))
					break;																	// Not from our windows.

				if (pMsg->button() == MouseButton::Left)
				{
					CoordSPX mousePos = _toLocal(pMsg->pointerSpxPos());

					if( pWidget == this )
					{
						auto pContainer = wg_dynamic_cast<Container_p>(mainSlot._widget());
						if (pContainer)
						{
							Widget * pWidget = pContainer->_findWidget(mousePos, SearchMode::MarkPolicy);
							if (pWidget)
								_selectWidget(pWidget);
						}
						break;									// Press on us is ignored
					}
					else
					{
						// Check for press on Window

						int id = pWidget->id();

						if( id < 10000 )
							break;								// Not any press to bother about.

						int windowIndex = _windowIndex(pWidget);
						int meaning = id % 1000;



						switch( meaning )
						{
							case 1:					// Resize
							{
								m_resizingWindow = windowIndex;
								m_resizingWindowDirection = _windowFrameSection( mousePos, windowIndex );
								m_resizingWindowStartGeo = windows[windowIndex].m_geo;
								break;
							}
							case 2:					// Raise and move

								windows._move(windowIndex, 0);
								m_movingWindow = 0; // windowIndex;
								m_movingWindowStartOfs = windows[0].m_placementPos;

								_childRequestRender(windows.begin(), windows[0].m_geo.size());
								break;

						}
					}

//					pMsg->swallow();
				}
				break;
			}

			case MsgType::MouseDrag:
			{
				auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

				if (m_movingWindow >= 0)
				{
					CoordSPX pos = m_movingWindowStartOfs + pMsg->_draggedTotal();
					SizeSPX size = windows[m_movingWindow]._size();

					if( pos.x < 0 )
						pos.x = 0;

					if( pos.y < 0 )
						pos.y = 0;

					if( pos.x + size.w > m_size.w )
						pos.x = m_size.w - size.w;

					if( pos.y + size.h > m_size.h )
						pos.y = m_size.h - size.h;

					windows[m_movingWindow].m_placementPos = pos;
					_refreshRealGeo(&windows[m_movingWindow]);
				}
				else if( m_resizingWindow >= 0)
				{

					const static CoordI	mulMtx[Placement_size] = { {0,0}, {-1,-1}, {0,-1}, {1,-1},
						{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {0,0} };

					CoordI mul = mulMtx[int(m_resizingWindowDirection)];

					auto pWindow = &windows[m_resizingWindow];

					CoordSPX dragged = pMsg->_draggedTotal();

					SizeSPX	sizeModif = { dragged.x * mul.x, dragged.y * mul.y };

					SizeSPX newSize = m_resizingWindowStartGeo.size() + sizeModif;
					newSize.limit( pWindow->_widget()->_minSize(m_scale), newSize );

					SizeSPX diff = newSize - m_resizingWindowStartGeo.size();

					RectSPX geo = m_resizingWindowStartGeo + diff;

					if( mul.x < 0 )
						geo.x -= diff.w;

					if( mul.y < 0 )
						geo.y -= diff.h;

					if( geo.x < 0 )
					{
						geo.w += geo.x;
						geo.x = 0;
					}

					if( geo.y < 0 )
					{
						geo.h += geo.y;
						geo.y = 0;
					}

					if( geo.x + geo.w > m_size.w )
						geo.w = m_size.w - geo.x;

					if( geo.y + geo.h > m_size.h )
						geo.h = m_size.h - geo.y;

					pWindow->m_placementPos = geo.pos();
					pWindow->m_chosenSize = geo.size();

					_refreshRealGeo(pWindow);
				}


				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() == MouseButton::Left)
				{
					m_movingWindow = -1;
					m_resizingWindow = -1;
				}

				break;
			}

			case MsgType::KeyPress:
			{
				auto pMsg = static_cast<KeyPressMsg*>(_pMsg);

				Key key = pMsg->translatedKeyCode();
				switch( key )
				{
					case Key::Escape:

						if( m_bSelectMode )
						{
							m_bSelectMode = false;
							m_pPickWidgetButton->setChecked(false);
							if( m_generatedPointerStyle == PointerStyle::Crosshair )
								m_generatedPointerStyle = PointerStyle::Undefined;
						}
						break;

					case Key::F1:
					case Key::F2:
					case Key::F3:
					case Key::F4:
					case Key::F5:
					case Key::F6:
					case Key::F7:
					case Key::F8:
					case Key::F9:
					case Key::F10:
					{
						int WindowIdx = int(key) - int(Key::F1);

						for( auto& Window : windows )
						{

							if( (Window.widget()->id() - 10000) / 1000 == WindowIdx )
							{
								Window.m_bVisible = !Window.m_bVisible;
								_requestRender(Window.m_geo);

								windows._move(&Window, windows.begin());

								// Make sure we don't keep dragging around an invisible window.

								m_movingWindow = -1;
								m_resizingWindow = -1;
								break;
							}
						}
					}


					default:
						break;
				}

				break;
			}

			default:
				break;
		};

		Overlay::_receive(_pMsg);
	}

	//____ _windowIndex() ___________________________________________________________

	int DebugOverlay::_windowIndex(Widget* pWidget)
	{
		auto pChild = pWidget;
		while (pChild->parent() != this)
			pChild = pChild->parent();

		auto it = windows._find(pChild);
		return windows._index(it);
	}

	//____ _windowFrameSection() _________________________________________________________

	Placement DebugOverlay::_windowFrameSection( CoordSPX pos, int windowIndex )
	{
		CoordSPX relPos = pos - windows[windowIndex].m_geo.pos();
		SizeSPX windowSize = windows[windowIndex].m_geo.size();

		int margin = m_scale * 8;

		Placement	section = Placement::Undefined;

		if( relPos.y < margin )
		{
			if( relPos.x < margin )
				section = Placement::NorthWest;
			else if( relPos.x >= windowSize.w - margin )
				section = Placement::NorthEast;
			else
				section = Placement::North;
		}
		else if( relPos.y >= windowSize.h - margin )
		{
			if( relPos.x < margin )
				section = Placement::SouthWest;
			else if( relPos.x >= windowSize.w - margin )
				section = Placement::SouthEast;
			else
				section = Placement::South;
		}
		else
		{
			if( relPos.x < margin )
				section = Placement::West;
			else if( relPos.x >= windowSize.w - margin )
				section = Placement::East;
			else
				section = Placement::Center;
		}

		return section;
	}


	//____ _createWindow() ________________________________________

	std::tuple<Widget_p, PackPanel_p> DebugOverlay::_createWindow( const char * pTitle )
	{
		auto pPanelSkin = m_pTheme->windowSkin();


		int idOfs = 10000 + 1000 * windows.size();

		auto pMain = PackPanel::create( WGBP(PackPanel, _.id = idOfs + 1, _.axis = Axis::Y, _.skin = pPanelSkin) );

		auto pTitleDisplay = TextDisplay::create( WGOVR( m_pTheme->windowTitleBar(), _.id = idOfs + 2, _.display.text = pTitle ));
/*
		auto pTitleDisplay = TextDisplay::create( WGBP(TextDisplay,
													   _.id = idOfs + 2,
													   _.display.text = pTitle,
													   _.markPolicy = MarkPolicy::Geometry,
													   _.skin = pTitleBarSkin ));
*/
		auto pContent = PackPanel::create( WGBP(PackPanel, _.axis = Axis::Y ));


		pMain->slots.pushBack( pTitleDisplay, WGBP(PackPanelSlot, _.weight = 0.f ));
		pMain->slots.pushBack( pContent, WGBP(PackPanelSlot, _.weight = 1.f ));


		auto it = windows._pushBackEmpty(1);



		it->_setWidget(pMain);
		it->m_bVisible = true;
		it->m_placement = Placement::NorthWest;

		return std::make_tuple(pMain,pContent);
	}

	//____ _createToolboxWindow() ____________________________________________

	void DebugOverlay::_createToolboxWindow()
	{
		// Create our window

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F1 - Toolbox");

		// Create our button palette

		auto pButtonPalette = PackPanel::create(WGBP(PackPanel, _.axis = Axis::X));

		auto pPickButton = ToggleButton::create(WGOVR(m_pTheme->toggleButton(), _.icon.skin = m_pSelectIcon, _.icon.placement = Placement::Center));

		m_pPickWidgetButton = pPickButton;

		Base::msgRouter()->addRoute(pPickButton, MsgType::Toggle, [this](Msg* pMsg) {

			auto pButton = wg_static_cast<ToggleButton_p>(pMsg->source());

			this->m_bSelectMode = pButton->isChecked();
			});

		pButtonPalette->slots << pPickButton;

		pContent->slots.pushBack(pButtonPalette, WGBP(PackPanelSlot, _.weight = 0.f));

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}

	//____ _createWidgetTreeWindow() ____________________________________________

	void DebugOverlay::_createWidgetTreeWindow()
	{
		// Add our windows

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F2 - Widget Tree");

		// Create our button palette

		auto pButtonPalette = PackPanel::create(WGBP(PackPanel, _.axis = Axis::X));

		auto pRefreshButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pRefreshIcon, _.icon.placement = Placement::Center ));

		Base::msgRouter()->addRoute(pRefreshButton, MsgType::Select, [this](Msg* pMsg) {

			if (mainSlot.widget())
				this->m_pWidgetTreeContainer->slot = m_pDebugger->createWidgetTreeView(m_debugPanelBP, mainSlot.widget());
			else
				this->m_pWidgetTreeContainer->slot = nullptr;
		});

		auto pCollapseAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pCondenseIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pCollapseAllButton, MsgType::Select, [this](Msg* pMsg) {

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreeView*>(m_pWidgetTreeContainer->slot._widget())->collapseAll();
		});

		auto pExpandAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pExpandIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pExpandAllButton, MsgType::Select, [this](Msg* pMsg) {

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreeView*>(m_pWidgetTreeContainer->slot._widget())->expandAll();
			});


		pButtonPalette->slots.pushBack({ pRefreshButton, pCollapseAllButton, pExpandAllButton });

		pContent->slots.pushBack(pButtonPalette, WGBP(PackPanelSlot, _.weight = 0.f));
		pContent->setLayout(nullptr);

		// Create scrollable content

		auto pContentSkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::White,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 2));

		auto pContentWindow = ScrollPanel::create(m_pTheme->scrollPanelXY());

		m_pWidgetTreeContainer = pContentWindow;

		if( mainSlot.widget() )
			pContentWindow->slot = m_pDebugger->createWidgetTreeView(m_debugPanelBP, mainSlot.widget());
			
		pContent->slots << pContentWindow;

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}

	//____ _createWidgetInfoWindow() ____________________________________________

	void DebugOverlay::_createWidgetInfoWindow()
	{
		// Create our window

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F3 - Widget Inspector");

		// Create scrollable content

		auto pContentSkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::White,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 4));

		auto pContentWindow = ScrollPanel::create(m_pTheme->scrollPanelY());

		auto pScrollableContent = PackPanel::create(WGBP(PackPanel,
			_.skin = pContentSkin,
			_.axis = Axis::Y,
			_.layout = m_pPackLayoutForScrollingContent

		));

		m_pWidgetTools = PackPanel::create();
		m_pWidgetTools->setAxis(Axis::Y);

		pScrollableContent->slots.pushBack(m_pWidgetTools);

		pContentWindow->slot = pScrollableContent;

		pContent->slots << pContentWindow;

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}

	//____ _createSkinInfoWindow() _____________________________________________

	void DebugOverlay::_createSkinInfoWindow()
	{
		// Add our windows

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F4 - Skin Inspector");

		// Create scrollable content

		auto pContentSkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::White,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 4));

		auto pContentWindow = ScrollPanel::create(m_pTheme->scrollPanelY());


		auto pScrollableContent = PackPanel::create(WGBP(PackPanel,
			_.skin = pContentSkin,
			_.axis = Axis::Y,
			_.layout = m_pPackLayoutForScrollingContent

		));

		m_pSkinContainer = PackPanel::create();
		m_pSkinContainer->setAxis(Axis::Y);

		pScrollableContent->slots.pushBack(m_pSkinContainer);

		pContentWindow->slot = pScrollableContent;

		pContent->slots << pContentWindow;

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}

	//____ _createObjectInfoWindow() _____________________________________________

	void DebugOverlay::_createObjectInfoWindow()
	{
		// Add our windows

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F5 - Object Inspector");

		// Create scrollable content

		auto pContentSkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::White,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 4));

		auto pContentWindow = ScrollPanel::create(m_pTheme->scrollPanelY());


		auto pScrollableContent = PackPanel::create(WGBP(PackPanel,
			_.skin = pContentSkin,
			_.axis = Axis::Y,
			_.layout = m_pPackLayoutForScrollingContent

		));

		m_pAnyObjectContainer = PackPanel::create();
		m_pAnyObjectContainer->setAxis(Axis::Y);

		pScrollableContent->slots.pushBack(m_pAnyObjectContainer);

		pContentWindow->slot = pScrollableContent;

		pContent->slots << pContentWindow;

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}


	//____ _createMsgLogWindow() _____________________________________________

	void DebugOverlay::_createMsgLogWindow()
	{
		// Add our windows

		Widget_p	pWindow;
		PackPanel_p	pContent;

		std::tie(pWindow, pContent) = _createWindow("F6 - Message Log");

		pContent->setLayout(nullptr);


		pContent->slots << m_pDebugger->createMsgLogViewer(m_debugPanelBP);

		_refreshRealGeo(windows._first() + windows.size() - 1);
	}




	//____ _createResources() ____________________________________________________

	void DebugOverlay::_createResources()
	{
		m_pPackLayoutForScrollingContent = PackLayout::create({});

		m_pRefreshIcon = BlockSkin::create(WGBP(BlockSkin,
			_.surface = m_pIcons,
			_.firstBlock = Rect(0, 0, 16, 16);
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

		auto& bp = m_pTheme->labeledSection();

		auto pListTextLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.placement = Placement::East ));

		auto pInfoLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.wrap = true,
														  _.placement = Placement::Center ));

		auto pWrapTextLayout = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.wrap = true,
			_.placement = Placement::NorthWest));

		m_pHeaderLayout = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.placement = Placement::Center));

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

		m_debugPanelBP.theme = m_pTheme;
		m_debugPanelBP.icons = m_pIcons;
		m_debugPanelBP.transparencyGrid = m_pTransparencyGrid;

		m_debugPanelBP.transparencyGrid = m_pTransparencyGrid;

		m_debugPanelBP.mainCapsule = m_pTheme->labeledSection();

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
