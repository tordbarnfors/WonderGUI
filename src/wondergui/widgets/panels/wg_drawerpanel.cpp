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

#include <wg_drawerpanel.h>
#include <wg_patches.h>
#include <wg_msg.h>

//TODO: Set opaque if there is no skin (or skin has no borders) and both children and the handle are opaque.

namespace wg
{
	const TypeInfo DrawerPanel::TYPEINFO = { "DrawerPanel", &Container::TYPEINFO };

	using namespace Util;

	//____ constructor ____________________________________________________________

	DrawerPanel::DrawerPanel() : slots(this)
	{
		m_bSiblingsOverlap = false;
	}

	//____ Destructor _____________________________________________________________

	DrawerPanel::~DrawerPanel()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DrawerPanel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDirection() _______________________________________________________

	void DrawerPanel::setDirection(Direction direction)
	{
		if (direction != m_direction)
		{
			auto oldDirection = m_direction;

			m_direction = direction;

			if( m_foldState != FoldState::CLOSED )
				_requestResize();

			_refreshOverflow();
		}
	}

	//____ setTransition() _______________________________________________________

	void DrawerPanel::setTransition(ValueTransition * pTransition)
	{
		m_pTransition = pTransition;
	}

	//____ setOpen() _____________________________________________________________

	void DrawerPanel::setOpen(bool bOpen)
	{
		if( bOpen && (m_foldState == FoldState::OPEN || m_foldState == FoldState::OPENING ) )
			return;

		if( !bOpen && (m_foldState == FoldState::CLOSED || m_foldState == FoldState::CLOSING ) )
			return;

		if( m_pTransition )
		{
			if( m_transitionProgress < 0 )
				_startReceiveUpdates();

			m_foldState = bOpen ? FoldState::OPENING : FoldState::CLOSING;
			m_transitionProgress = 0;
		}
		else
		{
			m_foldState = bOpen ? FoldState::OPEN : FoldState::CLOSED;
			_updateGeo();
			_requestResize();
		}
	}

	//____ _matchingHeight() _____________________________________________________

	spx DrawerPanel::_matchingHeight(spx width, int scale) const
	{
		spx heightClosed = m_skin.contentBorderSize(scale).h;

		if( !slots[0].isEmpty() )
			heightClosed += slots[0]._widget()->_matchingHeight(width,scale);

		spx heightOpen = heightClosed;

		if( !slots[1].isEmpty() )
		{
			spx h = slots[1]._widget()->_matchingHeight(width,scale);

			if( _isVertical() )
				heightOpen += h;
			else
				heightOpen = std::max(heightOpen, h);
		}

		switch( m_foldState )
		{
			default:
			case FoldState::CLOSED:
				return heightClosed;
			case FoldState::OPEN:
				return heightOpen;
			case FoldState::OPENING:
			{
				if (_isVertical())
					return m_pTransition->snapshot(m_transitionProgress, heightClosed, heightOpen);
				else
					return heightOpen;
			}
			case FoldState::CLOSING:
			{
				if( _isVertical() )
					return m_pTransition->snapshot(m_transitionProgress, heightOpen, heightClosed);
				else
					return heightOpen;
			}
		}
	}

	//____ _matchingWidth() ______________________________________________________

	spx DrawerPanel::_matchingWidth(spx height, int scale) const
	{
		spx widthClosed = m_skin.contentBorderSize(scale).w;

		if( !slots[0].isEmpty() )
			widthClosed += slots[0]._widget()->_matchingWidth(height,scale);

		spx widthOpen = widthClosed;

		if( !slots[1].isEmpty() )
		{
			spx w = slots[1]._widget()->_matchingWidth(height,scale);

			if( _isVertical() )
				widthOpen = std::max(widthOpen, w);
			else
				widthOpen += w;
		}

		switch( m_foldState )
		{
			default:
			case FoldState::CLOSED:
				return widthClosed;
			case FoldState::OPEN:
				return widthOpen;
			case FoldState::OPENING:
			{
				if (_isVertical())
					return widthOpen;
				else
					return m_pTransition->snapshot(m_transitionProgress, widthClosed, widthOpen);
			}
			case FoldState::CLOSING:
			{
				if( _isVertical() )
					return widthOpen;
				else
					return m_pTransition->snapshot(m_transitionProgress, widthOpen, widthClosed);
			}
		}
	}

	//____ _minSize() ____________________________________________________________

	SizeSPX DrawerPanel::_minSize(int scale) const
	{
		//TODO: Implement!!!

		return { 0, 0 };
	}

	//____ _maxSize() ____________________________________________________________

	SizeSPX DrawerPanel::_maxSize(int scale) const
	{
		//TODO: Implement!!!

		return { spx_max, spx_max };
	}

	//____ _defaultSize() _______________________________________________________

	SizeSPX  DrawerPanel::_defaultSize(int scale) const
	{
		SizeSPX 	defaultOpen, defaultClosed;

/*		if (scale == m_scale)
		{
			defaultOpen = m_defaultSizeOpen;
			defaultClosed = m_defaultSizeClosed;
		}
		else
*/
			std::tie(defaultClosed,defaultOpen) = _calcDefaultSize(scale);

		switch( m_foldState )
		{
			default:
			case FoldState::CLOSED:
				return defaultClosed;
			case FoldState::OPEN:
				return defaultOpen;
			case FoldState::OPENING:
			{
				if (_isVertical())
					defaultOpen.h = m_pTransition->snapshot(m_transitionProgress, defaultClosed.h, defaultOpen.h);
				else
					defaultOpen.w = m_pTransition->snapshot(m_transitionProgress, defaultClosed.w, defaultOpen.w);

				return defaultOpen;
			}
			case FoldState::CLOSING:
			{
				if( _isVertical() )
					defaultOpen.h = m_pTransition->snapshot(m_transitionProgress, defaultOpen.h, defaultClosed.h);
				else
					defaultOpen.w = m_pTransition->snapshot(m_transitionProgress, defaultOpen.w, defaultClosed.w);

				return defaultOpen;
			}
		}
	}

	//____ _receive() ___________________________________________________________

	void DrawerPanel::_receive(Msg * _pMsg)
	{
		switch (_pMsg->type())
		{
			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() == MouseButton::Left)
				{
					CoordSPX pos = _toLocal(pMsg->pointerSpxPos());

					if( m_frontGeo.contains(pos) )
					{
						if (m_foldState == FoldState::OPEN || m_foldState == FoldState::OPENING)
							setOpen(false);
						else
							setOpen(true);

						//TODO: Swallow!

					}
				}

				break;
			}
		}

	}

	//____ _update() ____________________________________________________________

	void DrawerPanel::_update(int microPassed, int64_t microsecTimestamp)
	{
		if (m_transitionProgress < 0)
		{
			assert(false);
			return;
		}

		m_transitionProgress += microPassed;
		if (m_transitionProgress >= m_pTransition->duration())
		{
			m_foldState = (m_foldState == FoldState::OPENING) ? FoldState::OPEN : FoldState::CLOSED;
			m_transitionProgress = -1;
			_stopReceiveUpdates();
		}

		_requestResize();
	}


	//____ _render() _____________________________________________________________

	void DrawerPanel::_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		m_skin.render(pDevice, canvas, m_scale, m_state);

		if( !slots[0].isEmpty() )
			slots[0]._widget()->_render(pDevice, m_frontGeo + canvas.pos(), m_frontGeo + canvas.pos() );

		if( !slots[1].isEmpty() )
		{
			Util::ClipPopData	clipPop;

			if( m_backWindow != m_backCanvas)
			{
				auto popData = Util::limitClipList(pDevice, m_backWindow + canvas.pos() );
				slots[1]._widget()->_render(pDevice, m_backCanvas + canvas.pos(), m_backWindow + canvas.pos() );
				Util::popClipList(pDevice, popData);
			}
			else
				slots[1]._widget()->_render(pDevice, m_backCanvas + canvas.pos(), m_backWindow + canvas.pos() );
		}
	}

	//____ _resize() _________________________________________________________

	void DrawerPanel::_resize(const SizeSPX& size, int scale)
	{
		Container::_resize(size,scale);
		_updateGeo();
		_refreshOverflow();
	}

	//____ _calcDefaultSize() ___________________________________________________

	std::tuple<SizeSPX,SizeSPX> DrawerPanel::_calcDefaultSize(int scale) const
	{
		SizeSPX defaultClosed = m_skin.contentBorderSize(scale);

		if( !slots[0].isEmpty() )
			defaultClosed += slots[0]._widget()->_defaultSize(scale);

		SizeSPX defaultOpen = defaultClosed;

		if( !slots[1].isEmpty() )
		{
			SizeSPX def = slots[1]._widget()->_defaultSize(scale);

			if( _isVertical() )
			{
				defaultOpen.w = std::max(defaultClosed.w, def.w);
				defaultOpen.h += def.h;
			}
			else
			{
				defaultOpen.h = std::max(defaultClosed.h, def.h);
				defaultOpen.w += def.w;
			}
		}

		return std::make_tuple(defaultClosed,defaultOpen);
	}

	//____ _updateGeo() _________________________________________________________

	void DrawerPanel::_updateGeo()
	{
		RectSPX content = _contentRect();


		SizeSPX frontSize, backSize;

		// Calculate size for front and back children.

		if( _isVertical() )
		{
			frontSize.w = content.w;
			frontSize.h = slots[0].isEmpty() ? 0 : std::min(slots[0]._widget()->_matchingHeight(content.w, m_scale ), content.h);

			backSize.w = content.w;
			backSize.h = slots[1].isEmpty() ? 0 : slots[1]._widget()->_matchingHeight(content.w, m_scale );
		}
		else
		{
			frontSize.w = slots[0].isEmpty() ? 0 : std::min( slots[0]._widget()->_matchingWidth(content.h, m_scale ), content.w);
			frontSize.h = content.h;

			backSize.w = slots[0].isEmpty() ? 0 : slots[0]._widget()->_matchingWidth(content.h, m_scale );
			backSize.h = content.h;
		}

		// Update size of children if changed

		auto pFrontWidget = slots[0]._widget();
		if( pFrontWidget && (frontSize != pFrontWidget->_size() || m_scale != pFrontWidget->_scale()) )
			pFrontWidget->_resize(frontSize,m_scale);

		auto pBackWidget = slots[1]._widget();
		if( pBackWidget && (backSize != pBackWidget->_size() || m_scale != pBackWidget->_scale()) )
			pBackWidget->_resize(backSize,m_scale);

		// Calculate maximum size of window (in case we are bigger than we want).

		SizeSPX maxWindowSize;

		switch (m_foldState)
		{
			case FoldState::OPEN:
				maxWindowSize = backSize;
				break;
			case FoldState::CLOSED:
				maxWindowSize = { 0,0 };
				break;
			case FoldState::OPENING:
			{
				maxWindowSize.h = m_pTransition->snapshot(m_transitionProgress, 0, backSize.h);
				maxWindowSize.w = m_pTransition->snapshot(m_transitionProgress, 0, backSize.w);
				break;
			}
			case FoldState::CLOSING:
			{
				maxWindowSize.h = m_pTransition->snapshot(m_transitionProgress, backSize.h, 0);
				maxWindowSize.w = m_pTransition->snapshot(m_transitionProgress, backSize.w, 0);
				break;
			}
		}


		// Calc frontGeo, backWindow & backCanvas.

		switch( m_direction )
		{
			case Direction::Up:

				m_frontGeo = { content.x, content.y + content.h - frontSize.h, frontSize.w, frontSize.h };
				m_backWindow = { content.x, content.y, content.w, std::min(content.h - frontSize.h, maxWindowSize.h) };

				m_backCanvas = { m_backWindow.x, m_backWindow.y, backSize };

				break;

			case Direction::Down:
				m_frontGeo = { content.x, content.y, frontSize.w, frontSize.h };
				m_backWindow = { content.x, content.y + frontSize.h, content.w, std::min(content.h - frontSize.h, maxWindowSize.h) };

				m_backCanvas = { m_backWindow.x, m_backWindow.y + m_backWindow.h - backSize.h, backSize };

				break;

			case Direction::Left:
				m_frontGeo = { content.x + content.w - frontSize.w, content.y, frontSize.w, frontSize.h };
				m_backWindow = { content.x, content.y, std::min(content.w - frontSize.w, maxWindowSize.w), content.h };

				m_backCanvas = { m_backWindow.x, m_backWindow.y, backSize };
				break;

			case Direction::Right:
				m_frontGeo = { content.x, content.y, frontSize.w, frontSize.h };
				m_backWindow = { content.x + frontSize.w, content.y, std::min(content.w - frontSize.w, maxWindowSize.w), content.h };

				m_backCanvas = { m_backWindow.x + m_backWindow.w - backSize.w, m_backWindow.y, backSize };
				break;
		}
	}

	//____ _slotTypeInfo() ____________________________________________________

	const TypeInfo& DrawerPanel::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		return DynamicSlot::TYPEINFO;
	}


	//_____ _firstChild() _____________________________________________________

	Widget * DrawerPanel::_firstChild() const
	{
		if (slots[0]._widget())
			return slots[0]._widget();
		else
			return slots[1]._widget();
	}

	//_____ _lastChild() ______________________________________________________

	Widget * DrawerPanel::_lastChild() const
	{
		if (slots[1]._widget())
			return slots[1]._widget();
		else
			return slots[0]._widget();
	}

	//____ _calcOverflow() ___________________________________________________
					  
	BorderSPX DrawerPanel::_calcOverflow()
	{
		//TODO: Implement!!!

		return BorderSPX();

	/*
		// We need our own _refreshOverflow because of our handleSkin that needs to
		// be included.
			
		RectSPX overflowArea = RectSPX{ 0,0, m_size } + m_skin.overflow(m_scale);
			
		if( slots[0]._widget() && slots[0]._widget()->_hasOverflow() )
			overflowArea.growToContain(slots[0].m_geo + slots[0]._widget()->_overflow() );

		if( slots[1]._widget() && slots[1]._widget()->_hasOverflow() )
			overflowArea.growToContain(slots[1].m_geo + slots[1]._widget()->_overflow() );

		if( !m_handleSkin.isEmpty() )
			overflowArea.growToContain(m_handleGeo + m_handleSkin.overflow(m_scale));
			
		BorderSPX overflow = BorderSPX::diff( RectSPX{0,0,m_size}, overflowArea );
		
		return overflow;
	 */
	}
					  
	//_____ _firstSlotWithGeo() _______________________________________________

	void DrawerPanel::_firstSlotWithGeo(SlotWithGeo& package) const
	{
		if (slots[0]._widget())
		{
			package.geo = m_frontGeo;
			package.pSlot = &slots[0];
		}
		else if (slots[1]._widget())
		{
			package.geo = m_backCanvas;
			package.pSlot = &slots[1];
		}
		else
		{
			package.pSlot = nullptr;
		}
	}

	//____ _nextSlotWithGeo() _________________________________________________

	void DrawerPanel::_nextSlotWithGeo(SlotWithGeo& package) const
	{
		if (package.pSlot == &slots[0] && slots[1]._widget())
		{
			package.geo = m_backCanvas;
			package.pSlot = &slots[1];
		}
		else
			package.pSlot = nullptr;
	}

	//_____ _slotGeo() _______________________________________________________

	RectSPX DrawerPanel::_slotGeo(const StaticSlot * pSlot) const
	{
		if( pSlot == &slots[0] )
			return m_frontGeo;
		else
			return m_backCanvas;
	}

	//____ _childRequestRender() ______________________________________________

	void DrawerPanel::_childRequestRender(StaticSlot * pSlot, const RectSPX& rect)
	{
		if( pSlot == &slots[0] )
			_requestRender( rect + m_frontGeo.pos() );
		else
		{
			RectSPX box = RectSPX::overlap( rect + m_backCanvas.pos(), m_backWindow );
			if( !box.isEmpty() )
				_requestRender( box );
		}
	}

	//____ _childRequestResize() ______________________________________________

	void DrawerPanel::_childRequestResize(StaticSlot * pSlot)
	{
		_requestResize();
	}

	//____ _prevChild() _______________________________________________________

	Widget * DrawerPanel::_prevChild(const StaticSlot * pSlot) const
	{
		if (pSlot == &slots[1])
			return slots[0]._widget();
		else
			return nullptr;
	}

	//____ _nextChild() _______________________________________________________

	Widget * DrawerPanel::_nextChild(const StaticSlot * pSlot) const
	{
		if (pSlot == &slots[0])
			return slots[1]._widget();
		else
			return nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void DrawerPanel::_releaseChild(StaticSlot * pSlot)
	{
		_replaceChild(pSlot, nullptr);
	}

	//____ _replaceChild() _______________________________________________________

	void DrawerPanel::_replaceChild(StaticSlot * _pSlot, Widget * pWidget)
	{
		//TODO: Shouldn't need to resize at start, should wait for result from _requestResize().

		_pSlot->_setWidget(pWidget );

		if (pWidget)
			pWidget->_resize(m_backCanvas.size(), m_scale);

		_updateGeo();
		_refreshOverflow();

		if( _pSlot == &slots[0] )
			_requestRender(m_frontGeo);
		else
			_requestRender(m_backWindow);
	}




}
