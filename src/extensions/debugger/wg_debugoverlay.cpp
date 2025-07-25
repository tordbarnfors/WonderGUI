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

#include <wg_widgettreepanel.h>

#include <string>

namespace wg
{
	using namespace Util;

	template class StaticSlotVector<DebugOverlay::ToolboxSlot>;

	const TypeInfo DebugOverlay::TYPEINFO = { "DebugOverlay", &Overlay::TYPEINFO };
	const TypeInfo DebugOverlay::ToolboxSlot::TYPEINFO = { "DebugOverlay::ToolboxSlot", &Overlay::Slot::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugOverlay::DebugOverlay(const Blueprint& bp) : Overlay(bp), toolboxes(this)
	{
		m_pDebugger = bp.debugger;
		m_pTheme 	= bp.theme;
		m_pIcons	= bp.icons;

		_createResources();

		// Create default skins

		m_pSelectionSkin = BoxSkin::create(1, Color::Transparent, Color::Red, 1);

		_createSlotWidgetToolbox();
		_createWidgetTreeToolbox();
		_createMsgLogToolbox();

		m_pDebugger->setWidgetSelectedCallback([this](Widget* pWidget) {_selectWidget(pWidget); });
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

	//____ setToolboxSkin() ___________________________________________________

	void DebugOverlay::setToolboxSkin(Skin * pSkin)
	{
		m_pToolboxSkin = pSkin;
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
			for (auto& slot : toolboxes)
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

			for ( int i = toolboxes.size()-1 ; i >= 0 ; i-- )
			{
				auto& palette = toolboxes[i];
				if (palette.m_bVisible)
				{
					RectSPX geo = palette.m_geo + _canvas.pos();

					RectSPX pxPaletteGeo = palette.m_geo;
					if (pDevice->clipBounds().isOverlapping(pxPaletteGeo))
					{
						ClipPopData popData = limitClipList(pDevice, palette.m_geo);
						palette._widget()->_render(pDevice, geo, geo);

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

	void DebugOverlay::_refreshRealGeo(ToolboxSlot * pSlot, bool bForceResize)
	{
		SizeSPX paletteSize = pSlot->m_chosenSize.isEmpty() ? pSlot->_widget()->_defaultSize(pSlot->_widget()->_scale()) : pSlot->m_chosenSize;

		paletteSize.limit({ 100*64,50*64 }, _size());

		RectSPX childGeo = Util::placementToRect(pSlot->m_placement, _size(), paletteSize);

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

		// Update toolboxes

		if (m_pSelectedWidget)
		{
			m_pSlotTools->slots.clear();
			m_pSlotTools->slots << _createSlotInfoPanel(m_pSelectedWidget->_slot());

			m_pWidgetTools->slots.clear();
			m_pWidgetTools->slots << _createWidgetInfoPanel(m_pSelectedWidget);

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreePanel*>(m_pWidgetTreeContainer->slot._widget())->select(m_pSelectedWidget);
/*
			for (auto& palette : toolboxes)
			{
				palette.m_bVisible = true;
				_refreshRealGeo(&palette);
			}
 */
		}
		else
		{
			for (auto& palette : toolboxes)
			{
				if (palette.m_bVisible)
				{
					_requestRender(palette.m_geo);
					palette.m_bVisible = false;
				}
			}
		}

		// 


	}

	//____ _slotTypeInfo() ________________________________________________________

	const TypeInfo&	DebugOverlay::_slotTypeInfo(const StaticSlot * pSlot) const
	{
		if (pSlot == &mainSlot)
			return DynamicSlot::TYPEINFO;

		return ToolboxSlot::TYPEINFO;
	}

	//____ _childRequestResize() ______________________________________________

	void DebugOverlay::_childRequestResize( StaticSlot * pSlot )
	{
		if( pSlot == &mainSlot )
			_requestResize();
		else
		{
			auto p = static_cast<ToolboxSlot*>(pSlot);
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
			auto p = static_cast<ToolboxSlot*>(pSlot);

			if (p->m_bVisible)
				_requestRender(p->m_geo);
			toolboxes._erase(p);
		}
	}

	//____ _beginOverlaySlots() __________________________________________________

	const Overlay::Slot * DebugOverlay::_beginOverlaySlots() const
	{
		return toolboxes._begin();
	}

	//____ _endOverlaySlots() ____________________________________________________

	const Overlay::Slot *  DebugOverlay::_endOverlaySlots() const
	{
		if( m_bActivated )
			return toolboxes._end();
		else
			return toolboxes._begin();
	}

	//____ _sizeOfOverlaySlot() __________________________________________________

	int DebugOverlay::_sizeOfOverlaySlot() const
	{
		return sizeof(ToolboxSlot);
	}


	//____ _resize() ___________________________________________________________

	void DebugOverlay::_resize( const SizeSPX& sz, int scale )
	{
		Overlay::_resize(sz, scale);

		// Refresh modal widgets geometry, their positions might have changed.

		for( auto pSlot = toolboxes._begin() ; pSlot != toolboxes._end() ; pSlot++ )
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
					break;																	// Not from our palettes.
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

				int	boxIndex = _boxIndex(pWidget);
				int meaning = id % 1000;

				if( meaning == 1 )
				{
					CoordSPX mousePos = _toLocal(pMsg->pointerSpxPos());

					auto section = _boxSection( mousePos, boxIndex );
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
					m_generatedPointerStyle = m_movingToolbox >= 0 ? PointerStyle::ClosedHand : PointerStyle::OpenHand;
				}

				break;
			}


			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);


				auto pWidget = static_cast<Widget*>(_pMsg->originalSourceRawPtr());

				if( !mainSlot.isEmpty() && pWidget->isDescendantOf(mainSlot._widget() ))
					break;																	// Not from our palettes.

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
						// Check for press on toolbox

						int id = pWidget->id();

						if( id < 10000 )
							break;								// Not any press to bother about.

						//int	boxIndex = (id - 10000) / 1000;

						int boxIndex = _boxIndex(pWidget);
						int meaning = id % 1000;



						switch( meaning )
						{
							case 1:					// Resize
							{
								m_resizingToolbox = boxIndex;
								m_resizingToolboxDirection = _boxSection( mousePos, boxIndex );
								m_resizingToolboxStartGeo = toolboxes[boxIndex].m_geo;
								break;
							}
							case 2:					// Raise and move

								toolboxes._move(boxIndex, 0);
								m_movingToolbox = 0; // boxIndex;
								m_movingToolboxStartOfs = toolboxes[0].m_placementPos;

								_childRequestRender(toolboxes.begin(), toolboxes[0].m_geo.size());
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

				if (m_movingToolbox >= 0)
				{
					CoordSPX pos = m_movingToolboxStartOfs + pMsg->_draggedTotal();
					SizeSPX size = toolboxes[m_movingToolbox]._size();

					if( pos.x < 0 )
						pos.x = 0;

					if( pos.y < 0 )
						pos.y = 0;

					if( pos.x + size.w > m_size.w )
						pos.x = m_size.w - size.w;

					if( pos.y + size.h > m_size.h )
						pos.y = m_size.h - size.h;

					toolboxes[m_movingToolbox].m_placementPos = pos;
					_refreshRealGeo(&toolboxes[m_movingToolbox]);
				}
				else if( m_resizingToolbox >= 0)
				{

					const static CoordI	mulMtx[Placement_size] = { {0,0}, {-1,-1}, {0,-1}, {1,-1},
						{1,0}, {1,1}, {0,1}, {-1,1}, {-1,0}, {0,0} };

					CoordI mul = mulMtx[int(m_resizingToolboxDirection)];

					auto pBox = &toolboxes[m_resizingToolbox];

					CoordSPX dragged = pMsg->_draggedTotal();

					SizeSPX	sizeModif = { dragged.x * mul.x, dragged.y * mul.y };

					SizeSPX newSize = m_resizingToolboxStartGeo.size() + sizeModif;
					newSize.limit( pBox->_widget()->_minSize(m_scale), newSize );

					SizeSPX diff = newSize - m_resizingToolboxStartGeo.size();

					RectSPX geo = m_resizingToolboxStartGeo + diff;

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

					pBox->m_placementPos = geo.pos();
					pBox->m_chosenSize = geo.size();

					_refreshRealGeo(pBox);
				}


				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() == MouseButton::Left)
				{
					m_movingToolbox = -1;
					m_resizingToolbox = -1;
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
					{
						int toolboxIdx = int(key) - int(Key::F1);

						for( auto& toolbox : toolboxes )
						{

							if( (toolbox.widget()->id() - 10000) / 1000 == toolboxIdx )
							{
								toolbox.m_bVisible = !toolbox.m_bVisible;
								_requestRender(toolbox.m_geo);

								toolboxes._move(&toolbox, toolboxes.begin());

								// Make sure we don't keep dragging around an invisible box.

								m_movingToolbox = -1;
								m_resizingToolbox = -1;
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

	//____ _boxIndex() ___________________________________________________________

	int DebugOverlay::_boxIndex(Widget* pWidget)
	{
		auto pChild = pWidget;
		while (pChild->parent() != this)
			pChild = pChild->parent();

		auto it = toolboxes._find(pChild);
		return toolboxes._index(it);
	}

	//____ _boxSection() _________________________________________________________

	Placement DebugOverlay::_boxSection( CoordSPX pos, int boxIndex )
	{
		CoordSPX relPos = pos - toolboxes[boxIndex].m_geo.pos();
		SizeSPX boxSize = toolboxes[boxIndex].m_geo.size();

		int margin = m_scale * 8;

		Placement	section = Placement::Undefined;

		if( relPos.y < margin )
		{
			if( relPos.x < margin )
				section = Placement::NorthWest;
			else if( relPos.x >= boxSize.w - margin )
				section = Placement::NorthEast;
			else
				section = Placement::North;
		}
		else if( relPos.y >= boxSize.h - margin )
		{
			if( relPos.x < margin )
				section = Placement::SouthWest;
			else if( relPos.x >= boxSize.w - margin )
				section = Placement::SouthEast;
			else
				section = Placement::South;
		}
		else
		{
			if( relPos.x < margin )
				section = Placement::West;
			else if( relPos.x >= boxSize.w - margin )
				section = Placement::East;
			else
				section = Placement::Center;
		}

		return section;
	}


	//____ _createToolbox() ________________________________________

	std::tuple<Widget_p, PackPanel_p> DebugOverlay::_createToolbox( const char * pTitle )
	{
		auto pPanelSkin = m_pTheme->windowSkin();


		int idOfs = 10000 + 1000 * toolboxes.size();

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


		auto it = toolboxes._pushBackEmpty(1);



		it->_setWidget(pMain);
		it->m_bVisible = true;
		it->m_placement = Placement::NorthWest;

		return std::make_tuple(pMain,pContent);
	}

	//____ _createWidgetTreeToolbox() ____________________________________________

	void DebugOverlay::_createWidgetTreeToolbox()
	{
		// Add our toolboxes

		Widget_p	pToolbox;
		PackPanel_p	pContent;

		std::tie(pToolbox, pContent) = _createToolbox("F2 - Widget Tree View");

		// Create our button palette

		auto pButtonPalette = PackPanel::create(WGBP(PackPanel, _.axis = Axis::X));

		auto pRefreshButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pRefreshIcon, _.icon.placement = Placement::Center ));

		Base::msgRouter()->addRoute(pRefreshButton, MsgType::Select, [this](Msg* pMsg) {

			if (mainSlot.widget())
				this->m_pWidgetTreeContainer->slot = m_pDebugger->createWidgetTreePanel(m_debugPanelBP, mainSlot.widget());
			else
				this->m_pWidgetTreeContainer->slot = nullptr;
		});

		auto pCollapseAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pCondenseIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pCollapseAllButton, MsgType::Select, [this](Msg* pMsg) {

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreePanel*>(m_pWidgetTreeContainer->slot._widget())->collapseAll();
		});

		auto pExpandAllButton = Button::create(WGOVR(m_pTheme->pushButton(), _.icon.skin = m_pExpandIcon, _.icon.placement = Placement::Center));

		Base::msgRouter()->addRoute(pExpandAllButton, MsgType::Select, [this](Msg* pMsg) {

			if (!m_pWidgetTreeContainer->slot.isEmpty())
				static_cast<WidgetTreePanel*>(m_pWidgetTreeContainer->slot._widget())->expandAll();
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

		auto pContentWindow = ScrollPanel::create(m_pTheme->scrollPanelY());

		m_pWidgetTreeContainer = pContentWindow;

		if( mainSlot.widget() )
			pContentWindow->slot = m_pDebugger->createWidgetTreePanel(m_debugPanelBP, mainSlot.widget());
			
		pContent->slots << pContentWindow;

		_refreshRealGeo(toolboxes._first() + toolboxes.size() - 1);
	}

	//____ _createMsgLogToolbox() _____________________________________________

	void DebugOverlay::_createMsgLogToolbox()
	{
		// Add our toolboxes

		Widget_p	pToolbox;
		PackPanel_p	pContent;

		std::tie(pToolbox, pContent) = _createToolbox("F3 - Message Log");

		pContent->setLayout(nullptr);


		pContent->slots << m_pDebugger->createMsgLogPanel(m_debugPanelBP);

		_refreshRealGeo(toolboxes._first() + toolboxes.size() - 1);
	}


	//____ _createSlotWidgetToolbox() ____________________________________________

	void DebugOverlay::_createSlotWidgetToolbox()
	{
		// Add our toolboxes

		Widget_p	pToolbox;
		PackPanel_p	pContent;

		std::tie(pToolbox, pContent) = _createToolbox("F1 - Slot/Widget View");

		// Create our button palette

		auto pButtonPalette = PackPanel::create( WGBP(PackPanel, _.axis = Axis::X) );

		auto pPickButton = ToggleButton::create( WGOVR(m_pTheme->toggleButton(), _.icon.skin = m_pSelectIcon, _.icon.placement = Placement::Center ));

		m_pPickWidgetButton = pPickButton;

		Base::msgRouter()->addRoute(pPickButton, MsgType::Toggle, [this](Msg * pMsg) {

			auto pButton = wg_static_cast<ToggleButton_p>(pMsg->source());

			this->m_bSelectMode = pButton->isChecked();
		});


		pButtonPalette->slots << pPickButton;

		pContent->slots.pushBack(pButtonPalette, WGBP(PackPanelSlot, _.weight = 0.f) );

		// Create scrollable content

		auto pContentSkin = BoxSkin::create( WGBP(BoxSkin,
												  _.color = Color::White,
												  _.outlineColor = Color::Black,
												  _.outlineThickness = 1,
												  _.padding = 2));

		auto pContentWindow = ScrollPanel::create( m_pTheme->scrollPanelY() );

		auto pScrollableContent = PackPanel::create( WGBP(PackPanel, 
			_.skin = pContentSkin, 
			_.axis = Axis::Y,
			_.layout = nullptr

		));


		m_pSlotTools = PackPanel::create();
		m_pSlotTools->setAxis(Axis::Y);

		m_pWidgetTools = PackPanel::create();
		m_pWidgetTools->setAxis(Axis::Y);

		pScrollableContent->slots.pushBack( {m_pSlotTools, m_pWidgetTools} );

		pContentWindow->slot = pScrollableContent;

		pContent->slots << pContentWindow;

		_refreshRealGeo( toolboxes._first() + toolboxes.size() -1 );
	}

	//____ _createSlotInfoPanel() ________________________________________

	Widget_p DebugOverlay::_createSlotInfoPanel(StaticSlot * pSlot)
	{
		auto pPanel = PackPanel::create( WGBP(PackPanel,
											  _.axis = Axis::Y ));

		auto pTypeInfo = &pSlot->typeInfo();

		DebugPanel::Blueprint bp = m_debugPanelBP;

		while( pTypeInfo != nullptr )
		{
			bp.mainCapsule.label.text = pTypeInfo->className;
			bp.mainCapsule.label.style = m_pTheme->strongStyle();

			auto pInfoPanel = m_pDebugger->createDebugPanel(bp, pTypeInfo, pSlot);

			if( pInfoPanel )
				pPanel->slots << pInfoPanel;

			pTypeInfo = pTypeInfo->pSuperClass;
		}

		return pPanel;
	}

	//____ _createWidgetInfoPanel() ______________________________________

	Widget_p DebugOverlay::_createWidgetInfoPanel( Widget * pWidget )
	{

		auto pPanel = PackPanel::create( WGBP(PackPanel,
											  _.axis = Axis::Y ));

		auto pTypeInfo = &pWidget->typeInfo();

		DebugPanel::Blueprint bp = m_debugPanelBP;

		while( pTypeInfo != nullptr )
		{
			bp.mainCapsule.label.text = pTypeInfo->className;
			bp.mainCapsule.label.style = m_pTheme->strongStyle();

			auto pInfoPanel = m_pDebugger->createDebugPanel(bp, pTypeInfo, pWidget);

			if( pInfoPanel )
				pPanel->slots << pInfoPanel;

			pTypeInfo = pTypeInfo->pSuperClass;
		}

		return pPanel;

	}

	//____ _createResources() ____________________________________________________

	void DebugOverlay::_createResources()
	{
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

		auto pValueLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle() ));


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

		m_debugPanelBP.mainCapsule = m_pTheme->labeledSection();

		m_debugPanelBP.listEntryLabel = WGBP(TextDisplay,
											 _.display.style = m_pTheme->strongStyle() );

		m_debugPanelBP.listEntryText = WGBP(TextDisplay,
											 _.display.style = m_pTheme->defaultStyle(),
											 _.display.layout = pListTextLayout );

		m_debugPanelBP.listEntryInteger = WGBP(NumberDisplay,
											 _.display.layout = pValueLayout );

		m_debugPanelBP.listEntrySPX = WGBP(NumberDisplay,
											 _.display.layout = pValueLayout );

		m_debugPanelBP.listEntryPts = WGBP(NumberDisplay,
											 _.display.layout = pPtsLayout );

		m_debugPanelBP.listEntryDecimal = WGBP(NumberDisplay,
											 _.display.layout = pValueLayout );

		m_debugPanelBP.listEntryPointer = WGBP(NumberDisplay,
											 _.display.layout = pPointerLayout );

		m_debugPanelBP.listEntryDrawer = m_pTheme->treeListDrawer();
		m_debugPanelBP.selectableListEntryCapsule = WGOVR( m_pTheme->treeListEntry(), _.selectable = true );

		m_debugPanelBP.infoDisplay = WGBP(TextDisplay,
											 _.display.style = m_pTheme->emphasisStyle(),
											 _.display.layout = pInfoLayout );

		m_debugPanelBP.table = WGBP(TablePanel,
									_.columnLayout = Base::defaultPackLayout());
	}


} // namespace wg
