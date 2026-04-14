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

#include <wg_scrollcapsule.h>
#include <wg_base.h>
#include <wg_inputhandler.h>

namespace wg
{
	using namespace Util;

	const TypeInfo	ScrollCapsule::TYPEINFO = { "ScrollCapsule", &Capsule::TYPEINFO };

	//____ create() ___________________________________________________________

	ScrollCapsule_p ScrollCapsule::create()
	{
		return ScrollCapsule_p(new ScrollCapsule());
	}

	ScrollCapsule_p ScrollCapsule::create(const Blueprint& blueprint)
	{
		return ScrollCapsule_p(new ScrollCapsule(blueprint));
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScrollCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ constructor ______________________________________________________

	ScrollCapsule::ScrollCapsule() : Capsule(), scrollbarX(this, this, Axis::X), scrollbarY(this, this, Axis::Y), m_cornerSkin(this)
	{
	}

	//____ destructor ______________________________________________________

	ScrollCapsule::~ScrollCapsule()
	{
	}

	//____ _receive() ____________________________________________________________

	void ScrollCapsule::_receive(Msg * pMsg)
	{
		bool bX = false, bY = false;

		// Give our scrollbars the opportunity to process

		if (pMsg->type() != MsgType::WheelRoll || m_bWheelFollowsScrollbar)
		{
			if (scrollbarX.isDisplayable())
				bX = scrollbarX._receive(pMsg);

			if (scrollbarY.isDisplayable())
				bY = scrollbarY._receive(pMsg);
		}

		// Leave if processed by our scrollbars.

		if (bX || bY)
			return;

		switch (pMsg->type())
		{
		case MsgType::WheelRoll:
		{
			auto p = static_cast<WheelRollMsg*>(pMsg);

			if( m_wheelAxis == Axis::Undefined || p->wheel() < 1 || p->wheel() > 2 )
				break;		// No scroll axis defined, ignore.


			Axis direction = m_wheelAxis;
			if (p->wheel() == 2 || p->modKeys() & m_wheelAxisModifier )
				direction = m_wheelAxis == Axis::X ? Axis::Y : Axis::X;

			int factor = p->distance();

			if( p->invertScroll() )
				factor = -factor;

			if (p->modKeys() & m_wheelAccelerator)
				factor *= m_wheelAccelFactor;

			if (direction == Axis::X)
			{
				spx movement = factor * ptsToSpx(m_wheelStepSizeX, m_scale);
				_setViewOffset({ (m_viewRegion.x - m_childCanvas.x) - movement, (m_viewRegion.y - m_childCanvas.y) });
			}
			else if (direction == Axis::Y)
			{
				spx movement = factor * ptsToSpx(m_wheelStepSizeY, m_scale);
				_setViewOffset({ (m_viewRegion.x - m_childCanvas.x), (m_viewRegion.y - m_childCanvas.y) - movement });
			}

			p->swallow();
			break;
		}

		default:
			break;
		}



	}

	//____ _maskPatches() ________________________________________________________

	void ScrollCapsule::_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip)
	{
		//TODO: Implement!!!
	}

	//____ _findWidget() _________________________________________________________

	Widget * ScrollCapsule::_findWidget(const CoordSPX& pos, SearchMode mode)
	{
		Widget* pFound = nullptr;

		if (!slot.isEmpty() && m_viewRegion.contains(pos))
		{
			auto pChild = slot._widget();
			if (pChild->isContainer())
				pFound = static_cast<Container*>(pChild)->_findWidget(pos - m_childCanvas.pos(), mode);
			else if (mode == SearchMode::Geometry || pChild->_markTest(pos - m_childCanvas.pos()))
				pFound = pChild;
		}

		if (!pFound)
		{
			// Test against ourself

			if (mode == SearchMode::Geometry || _alphaTest(pos) )
				pFound = this;
		}

		return pFound;
	}

	//____ _firstSlotWithGeo() ___________________________________________________

	void ScrollCapsule::_firstSlotWithGeo(SlotWithGeo& package) const
	{
		package.pSlot = &slot;
		package.geo = m_childCanvas;
	}

	//____ _slotGeo() ____________________________________________________________

	RectSPX ScrollCapsule::_slotGeo(const StaticSlot * pSlot) const
	{
		return m_viewRegion;
	}

	//____ _childOverflowChanged() _______________________________________________

	void ScrollCapsule::_childOverflowChanged( StaticSlot * pSlot, const BorderSPX& oldOverflow, const BorderSPX& newOverflow )
	{
		// We can ignore this since we always clip our only child.
	}

	//____ _childWindowSection() _________________________________________________

	RectSPX ScrollCapsule::_childWindowSection(const StaticSlot * pSlot) const
	{
		return m_viewRegion - m_childCanvas.pos();

	}

	//____ _childLocalToGlobal() _________________________________________________

	RectSPX ScrollCapsule::_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return _toGlobal(rect + m_childCanvas.pos());
	}

	//____ _globalToChildLocal() _________________________________________________

	RectSPX ScrollCapsule::_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const
	{
		return _toLocal(rect) - m_childCanvas.pos();
	}

	//____ _globalPtsToChildLocalSpx() ___________________________________________

	RectSPX ScrollCapsule::_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const
	{
		RectSPX rectSPX = m_pHolder ? m_pHolder->_globalPtsToChildLocalSpx(m_pSlot, rect) : align(ptsToSpx(rect, m_scale));

		return rectSPX - m_childCanvas.pos();
	}

	//____ _childLocalSpxToGlobalPts() ___________________________________________

	Rect ScrollCapsule::_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& _rect) const
	{
		RectSPX rect = _rect + m_childCanvas.pos();

		if( m_pHolder )
			return m_pHolder->_childLocalSpxToGlobalPts( m_pSlot, rect );
		else
			return spxToPts(rect, m_scale);
	}

	//____ _childRequestRender() _________________________________________________

	void ScrollCapsule::_childRequestRender(StaticSlot * pSlot, const RectSPX& rect)
	{
		RectSPX visible = RectSPX::overlap(rect + m_childCanvas.pos(), m_viewRegion);
		if (!visible.isEmpty())
			_requestRender(visible);
	}

	//____ _childRequestResize() _________________________________________________

	void ScrollCapsule::_childRequestResize(StaticSlot * pSlot)
	{
		_requestResize();
	}

	//____ _childRequestInView() _________________________________________________

	void ScrollCapsule::_childRequestInView(StaticSlot * pSlot)
	{
		// Our only child is always in view as much as possible, so just pass it on like any
		// normal container.

		_requestInView( m_viewRegion, m_viewRegion);
	}

	void ScrollCapsule::_childRequestInView(StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea)
	{
		if (!m_viewRegion.contains(niceToHaveArea + m_childCanvas.pos()))
		{
			RectSPX window = m_viewRegion - m_childCanvas.pos();
			CoordSPX	startPos = window.pos();

			for (int i = 0; i < 2; i++)
			{
				RectSPX inside = i == 0 ? niceToHaveArea : mustHaveArea;

				int diffLeft = inside.x - window.x;
				int diffRight = inside.right() - window.right();
				int diffTop = inside.y - window.y;
				int diffBottom = inside.bottom() - window.bottom();

				if (diffLeft > 0 && diffRight > 0)
					window.x += std::min(diffLeft, diffRight);
				else if (diffLeft < 0 && diffRight < 0)
					window.x += std::max(diffLeft, diffRight);

				if (diffTop > 0 && diffBottom > 0)
					window.y += std::min(diffTop, diffBottom);
				else if (diffTop < 0 && diffBottom < 0)
					window.y += std::max(diffTop, diffBottom);
			}

			if (window.pos() != startPos)
			{
//				if( m_pDefaultTransition )
//					_setViewTransition(window.pos(), nullptr);
//				else
					_setViewOffset(window.pos());
			}

			// Forward to any outer ScrollPanel

			RectSPX newMustHaveArea = RectSPX::overlap(mustHaveArea + m_childCanvas.pos(), m_viewRegion);
			RectSPX newNiceToHaveArea = RectSPX::overlap(niceToHaveArea + m_childCanvas.pos(), m_viewRegion);

			_requestInView(newMustHaveArea, newNiceToHaveArea);

		}
	}

	//____ _releaseChild() _______________________________________________________

	void ScrollCapsule::_releaseChild(StaticSlot * pSlot)
	{
		Capsule::_releaseChild(pSlot);
		_updateRegions();
	}

	//____ _replaceChild() _______________________________________________________

	void ScrollCapsule::_replaceChild(StaticSlot * pSlot, Widget * pWidget)
	{
		Capsule::_replaceChild(pSlot, pWidget);
		_updateRegions();
		_setViewOffset({0,0});
	}

	//____ _componentPos() ____________________________________________________

	CoordSPX ScrollCapsule::_componentPos(const Component* pComponent) const
	{
		if (pComponent == &scrollbarX)
			return m_scrollbarXRegion.pos();
		else
			return m_scrollbarYRegion.pos();
	}

	//____ _componentSize() ___________________________________________________

	SizeSPX ScrollCapsule::_componentSize(const Component* pComponent) const
	{
		if (pComponent == &scrollbarX)
			return m_scrollbarXRegion.size();
		else
			return m_scrollbarYRegion.size();

	}

	//____ _componentGeo() ____________________________________________________

	RectSPX ScrollCapsule::_componentGeo(const Component* pComponent) const
	{
		if (pComponent == &scrollbarX)
			return m_scrollbarXRegion;
		else
			return m_scrollbarYRegion;
	}

	//____ _setViewOffset() _______________________________________________________

	bool ScrollCapsule::_setViewOffset( CoordSPX _offset )
	{
		CoordSPX offset = align(_offset);

		auto oldChildCanvas = m_childCanvas;
		auto wantedChildCanvasPos = -offset;

		m_childCanvas.setPos(wantedChildCanvasPos);

		_childCanvasCorrection();

		if( oldChildCanvas.pos() != m_childCanvas.pos() )
		{
			_updateScrollbars(oldChildCanvas, m_viewRegion);
			_requestRender();
		}

		return ( m_childCanvas.pos() == wantedChildCanvasPos);
	}

	//____ _render() _____________________________________________________________

	void ScrollCapsule::_render(GfxDevice* pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		m_skin.render(pDevice, canvas, m_scale, m_state);

		if (!slot.isEmpty())
		{
			auto clipPop = limitClipList(pDevice, m_viewRegion + canvas.pos() );
			slot._widget()->_render(pDevice, m_childCanvas + canvas.pos(), m_viewRegion + canvas.pos());
			popClipList(pDevice, clipPop);
		}

		if (!m_scrollbarXRegion.isEmpty())
			scrollbarX._render(pDevice, m_scrollbarXRegion + canvas.pos(), m_scale);

		if (!m_scrollbarYRegion.isEmpty())
			scrollbarY._render(pDevice, m_scrollbarYRegion + canvas.pos(), m_scale);

		if (!m_cornerSkin.isEmpty() && !m_scrollbarXRegion.isEmpty() && !m_scrollbarYRegion.isEmpty() && !m_bOverlayScrollbars)
		{
			RectSPX cornerRect = {
				m_scrollbarYRegion.x,
				m_scrollbarXRegion.y,
				m_scrollbarYRegion.w,
				m_scrollbarXRegion.h
			};

			m_cornerSkin.render(pDevice, cornerRect + canvas.pos(), m_scale, m_state);
		}
	}

	//____ _alphaTest() __________________________________________________________

	bool ScrollCapsule::_alphaTest(const CoordSPX& ofs)
	{
		return true;	//TODO: Implement!!!
	}

	//____ _resize() _____________________________________________________________

	void ScrollCapsule::_resize(const SizeSPX& size, int scale)
	{
		auto oldScale = m_scale;
		auto oldCanvas = m_childCanvas;
		auto oldView = m_viewRegion;

		m_size = size;
		m_scale = scale;

		_updateRegions();
		_childCanvasCorrection();

		if (m_autoScrollAxis == Axis::X)
		{
			if (oldView.x + oldView.w == oldCanvas.x + oldCanvas.w)
				m_viewRegion.x = m_childCanvas.x + m_childCanvas.w - m_viewRegion.w;
		}
		else if (m_autoScrollAxis == Axis::Y)
		{
			if (oldView.y + oldView.h == oldCanvas.y + oldCanvas.h)
				m_viewRegion.y = m_childCanvas.y + m_childCanvas.h - m_viewRegion.h;
		}

		_updateScrollbars( oldCanvas, oldView );

		if( !slot.isEmpty() && (oldCanvas.size() != m_childCanvas.size() || scale != oldScale) )
		   slot._widget()->_resize(m_childCanvas.size(), scale);
	}

	//____ _setState() ___________________________________________________________

	void ScrollCapsule::_setState(State state)
	{
		//TODO: Implement!!!
	}

	//____ _calcOverflow() _______________________________________________________

	BorderSPX ScrollCapsule::_calcOverflow()
	{
		//TODO: Implement!!!

		return BorderSPX();
	}



	//____ _matchingHeight() _________________________________________________________

	spx ScrollCapsule::_matchingHeight(spx width, int scale) const
	{
		// If we don't scroll content at all or don't have any child, we just act as a normal capsule.

		if (!(m_bScrollX || m_bScrollY) || !slot._widget())
			return Capsule::_matchingHeight(width, scale);

		// If we scroll horizontally, we always have content of default width, thus also default height.

		if (m_bScrollX && scrollbarX.isDisplayable())
			return _defaultSize(scale).h;

		// We need to calculate the matching height based on the width and the content.

		SizeSPX contentBorder = m_skin.contentBorderSize(scale);

		spx contentWidth = width - contentBorder.w;
		if (contentWidth < 0)
			contentWidth = 0;		// This should probably be handled in a better way.

		int contentHeight = slot._widget()->_matchingHeight(contentWidth, scale);

		spx scrollbarYHeight = scrollbarY._defaultSize(scale).h;

		return std::max(contentHeight, scrollbarYHeight) + contentBorder.h;
	}

	//____ _matchingWidth() _________________________________________________________

	spx ScrollCapsule::_matchingWidth(spx height, int scale) const
	{
		// If we don't scroll content at all or don't have any child, we just act as a normal capsule.

		if (!(m_bScrollX || m_bScrollY) || !slot._widget())
			return Capsule::_matchingWidth(height, scale);

		// If we scroll vertically, we always have content of default height, thus also default width.

		if (m_bScrollY && scrollbarY.isDisplayable())
			return _defaultSize(scale).w;

		// We need to calculate the matching width based on the height and the content.

		SizeSPX contentBorder = m_skin.contentBorderSize(scale);

		spx contentHeight = height - contentBorder.h;
		if (contentHeight < 0)
			contentHeight = 0;		// This should probably be handled in a better way.

		int contentWidth = slot._widget()->_matchingWidth(contentHeight, scale);

		spx scrollbarXWidth = scrollbarX._defaultSize(scale).w;

		return std::max(contentWidth, scrollbarXWidth) + contentBorder.w;
	}

	//____ _defaultSize() _________________________________________________________

	SizeSPX ScrollCapsule::_defaultSize(int scale) const
	{
		SizeSPX sz = m_skin.contentBorderSize(scale);
			
		if (slot._widget())
		{
			// Scrollbars will only be shown if we have a child.

			SizeSPX childSize = slot._widget()->_defaultSize(scale);

			SizeSPX scrollbarXSize = scrollbarX._defaultSize(scale);
			SizeSPX scrollbarYSize = scrollbarY._defaultSize(scale);

			if( m_bScrollX && scrollbarX.isDisplayable() )
				sz.w += std::max(childSize.w, scrollbarXSize.w);
			else
				sz.w += childSize.w;

			if (m_bScrollY && scrollbarY.isDisplayable())
				sz.h += std::max(childSize.h, scrollbarXSize.h);
			else
				sz.h += childSize.h;

			// Add thickness of scrollbars if they are outside view and always visible.

			if (!m_bOverlayScrollbars && !m_bAutoHideScrollbars)
			{
				if( m_bScrollX && scrollbarX.isDisplayable() )
					childSize.h += scrollbarXSize.h;

				if (m_bScrollY && scrollbarY.isDisplayable())
					childSize.w += scrollbarYSize.w;
			}
		}

		return sz;
	}

	//____ _minSize() _________________________________________________________

	SizeSPX ScrollCapsule::_minSize(int scale) const
	{
		// We never make ScrollCapsule smaller than the scrollbars can be
		// displayed without artifacts.

		SizeSPX sz = m_skin.contentBorderSize(scale);

		if (slot._widget())
		{
			// Scrollbars will only be shown if we have a child.

			SizeSPX childSize = slot._widget()->_minSize(scale);

			SizeSPX scrollbarXSize = scrollbarX._defaultSize(scale);
			SizeSPX scrollbarYSize = scrollbarY._defaultSize(scale);

			if (m_bScrollX && scrollbarX.isDisplayable())
				sz.w += std::max(childSize.w, scrollbarXSize.w);
			else
				sz.w += childSize.w;

			if (m_bScrollY && scrollbarY.isDisplayable())
				sz.h += std::max(childSize.h, scrollbarXSize.h);
			else
				sz.h += childSize.h;

			// Add thickness of scrollbars if they are outside view (even if not always visible).

			if (!m_bOverlayScrollbars)
			{
				if (m_bScrollX && scrollbarX.isDisplayable())
					childSize.h += scrollbarXSize.h;

				if (m_bScrollY && scrollbarY.isDisplayable())
					childSize.w += scrollbarYSize.w;
			}
		}

		return sz;

	}

	//____ _maxSize() _________________________________________________________

	SizeSPX ScrollCapsule::_maxSize(int scale) const
	{
		if (!slot._widget())
			return { MAX_WIDGET_SIZE_SPX, MAX_WIDGET_SIZE_SPX };

		SizeSPX childSize = slot._widget()->_maxSize(scale);

		if (childSize.w == MAX_WIDGET_SIZE_SPX && childSize.h == MAX_WIDGET_SIZE_SPX)
			return childSize;

		// At least one dimension is smaller than maximum, so we need to do some calculations.

		SizeSPX sz = m_skin.contentBorderSize(scale);

		SizeSPX scrollbarXSize = scrollbarX._defaultSize(scale);
		SizeSPX scrollbarYSize = scrollbarY._defaultSize(scale);

		// Max size of ScrollCapsule is at least the default size of scrollbars if they are displayable.

		if (m_bScrollX && scrollbarX.isDisplayable())
			sz.w += std::max(childSize.w, scrollbarXSize.w);
		else
			sz.w += childSize.w;

		if (m_bScrollY && scrollbarY.isDisplayable())
			sz.h += std::max(childSize.h, scrollbarXSize.h);
		else
			sz.h += childSize.h;

		// Add thickness of scrollbars if they are outside view (even if not always visible).

		if (!m_bOverlayScrollbars)
		{
			if (m_bScrollX && scrollbarX.isDisplayable())
				childSize.h += scrollbarXSize.h;

			if (m_bScrollY && scrollbarY.isDisplayable())
				childSize.w += scrollbarYSize.w;
		}

		// Make sure we don't return a size larger than the maximum allowed.

		sz.w = std::min(sz.w, MAX_WIDGET_SIZE_SPX);
		sz.h = std::min(sz.h, MAX_WIDGET_SIZE_SPX);

		return sz;
	}

	//____ _updateRegions() _________________________________________________________

	void ScrollCapsule::_updateRegions()
	{
		RectSPX window = _contentRect();

		Widget* pChild = slot._widget();

		// If we don't have a child or don't scroll, the whole area is view region and there are no scrollbars.

		if (!pChild || !(m_bScrollX || m_bScrollY) )
		{
			m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };
			m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			m_viewRegion = window;
			m_childCanvas = window;
			return;
		}

		// If we scroll in both directions

		if (m_bScrollX && m_bScrollY)
		{
			SizeSPX canvasSize = pChild->_defaultSize(m_scale);

			spx scrollbarYWidth = scrollbarY._defaultSize(m_scale).w;
			spx scrollbarXHeight = scrollbarX._defaultSize(m_scale).h;

			if( m_bOverlayScrollbars )
			{
				m_viewRegion = window;

				if (m_childCanvas.w > window.w || !m_bAutoHideScrollbars)
					m_scrollbarXRegion = { window.x, window.y + window.h - scrollbarXHeight, window.w, scrollbarXHeight };
				else
					m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };

				if (m_childCanvas.h > window.h || !m_bAutoHideScrollbars)
					m_scrollbarYRegion = { window.x + window.w - scrollbarYWidth, window.y, scrollbarYWidth, window.h };
				else
					m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			}
			else
			{
				spx spaceForScrollbarX, spaceForScrollbarY;

				if( m_bAutoHideScrollbars )
				{
					spaceForScrollbarY = (canvasSize.h > window.h) ? scrollbarYWidth : 0;
					spaceForScrollbarX = (canvasSize.w > window.w - spaceForScrollbarY) ? scrollbarXHeight : 0;
					spaceForScrollbarY = (canvasSize.h > window.h - spaceForScrollbarX) ? scrollbarYWidth : 0;
				}
				else
				{
					spaceForScrollbarY = scrollbarYWidth;
					spaceForScrollbarX = scrollbarXHeight;
				}

				m_viewRegion = { window.x, window.y, window.w - spaceForScrollbarY, window.h - spaceForScrollbarX };
				m_scrollbarXRegion = { window.x, window.y + window.h - spaceForScrollbarX, window.w - spaceForScrollbarY, spaceForScrollbarX };
				m_scrollbarYRegion = { window.x + window.w - spaceForScrollbarY, window.y, spaceForScrollbarY, window.h - spaceForScrollbarX };

				m_childCanvas.w = std::max( canvasSize.w, window.w - spaceForScrollbarY );
				m_childCanvas.h = std::max( canvasSize.h, window.h - spaceForScrollbarX );
			}

		}
		else if (m_bScrollY)
		{
			spx scrollbarYWidth = scrollbarY._defaultSize(m_scale).w;

			if (!m_bOverlayScrollbars && !m_bAutoHideScrollbars)
			{
				// One and only scrollbar always present and outside view.

				m_viewRegion = { window.x, window.y, window.w - scrollbarYWidth, window.h };
				m_scrollbarYRegion = { window.x + window.w - scrollbarYWidth, window.y, scrollbarYWidth, window.h };
				m_scrollbarXRegion = { window.x, window.y + window.h, window.w - scrollbarYWidth, 0 };
				m_childCanvas.w = m_viewRegion.w;
				m_childCanvas.h = pChild->_matchingHeight(m_viewRegion.w, m_scale);
			}
			else if( m_bOverlayScrollbars)
			{
				m_viewRegion = window;
				m_childCanvas.w = m_viewRegion.w;
				m_childCanvas.h = pChild->_matchingHeight(m_viewRegion.w, m_scale);

				if (m_childCanvas.h > window.h || !m_bAutoHideScrollbars)
					m_scrollbarYRegion = { window.x + window.w - scrollbarYWidth, window.y, scrollbarYWidth, window.h };
				else
					m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };

				m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };
			}
			else
			{
				// We have a scrollbar outside view with autohide, this is the tricky alternative.
				// First we determine height of canvas without scrollbar, 
				// then we determine if scrollbar is needed and if so, we adjust view region and canvas size accordingly.

				spx canvasHeight = pChild->_matchingHeight(window.w, m_scale);

				if (canvasHeight > window.h)
				{
					m_viewRegion = { window.x, window.y, window.w - scrollbarYWidth, window.h };
					m_childCanvas.w = m_viewRegion.w;
					m_childCanvas.h = pChild->_matchingHeight(m_viewRegion.w, m_scale);
					m_scrollbarYRegion = { window.x + window.w - scrollbarYWidth, window.y, scrollbarYWidth, window.h };
				}
				else
				{
					m_viewRegion = window;
					m_childCanvas.w = m_viewRegion.w;
					m_childCanvas.h = m_viewRegion.h;
					m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
				}
				m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };
			}
		}
		else if (m_bScrollX)
		{
			spx scrollbarXHeight = scrollbarX._defaultSize(m_scale).h;

			if (!m_bOverlayScrollbars && !m_bAutoHideScrollbars)
			{
				// One and only scrollbar always present and outside view.

				m_viewRegion = { window.x, window.y, window.w, window.h - scrollbarXHeight };
				m_scrollbarXRegion = { window.x, window.y + window.h - scrollbarXHeight, window.w, scrollbarXHeight };
				m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h - scrollbarXHeight };
				m_childCanvas.w = pChild->_matchingWidth(m_viewRegion.h, m_scale);
				m_childCanvas.h = m_viewRegion.h;
			}
			else if (m_bOverlayScrollbars)
			{
				m_viewRegion = window;
				m_childCanvas.w = pChild->_matchingWidth(m_viewRegion.h, m_scale);
				m_childCanvas.h = m_viewRegion.h;

				if (m_childCanvas.w > window.w || !m_bAutoHideScrollbars)
					m_scrollbarXRegion = { window.x, window.y + window.h - scrollbarXHeight, window.w, scrollbarXHeight };
				else
					m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };

				m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			}
			else
			{
				// We have a scrollbar outside view with autohide, this is the tricky alternative.
				// First we determine width of canvas without scrollbar, 
				// then we determine if scrollbar is needed and if so, we adjust view region and canvas size accordingly.

				spx canvasWidth = pChild->_matchingWidth(window.h, m_scale);

				if (canvasWidth > window.w)
				{
					m_viewRegion = { window.x, window.y, window.w, window.h - scrollbarXHeight };
					m_childCanvas.w = pChild->_matchingWidth(m_viewRegion.h, m_scale);
					m_childCanvas.h = m_viewRegion.h;
					m_scrollbarXRegion = { window.x, window.y + window.h - scrollbarXHeight, window.w, scrollbarXHeight };
				}
				else
				{
					m_viewRegion = window;
					m_childCanvas.w = m_viewRegion.w;
					m_childCanvas.h = m_viewRegion.h;
					m_scrollbarXRegion = { window.x, window.y + window.h, window.x, 0 };
				}
				m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			}
		}

	}

	//____ _childCanvasCorrection() ______________________________________________

	void ScrollCapsule::_childCanvasCorrection()
	{
		if( !m_childCanvas.contains(m_viewRegion) )
		{
			if( m_childCanvas.x > m_viewRegion.x )
				m_childCanvas.x = m_viewRegion.x;

			if( m_childCanvas.x + m_childCanvas.w < m_viewRegion.x + m_viewRegion.w )
				m_childCanvas.x = m_viewRegion.x + m_viewRegion.w - m_childCanvas.w;

			if( m_childCanvas.y > m_viewRegion.y )
				m_childCanvas.y = m_viewRegion.y;

			if( m_childCanvas.y + m_childCanvas.h < m_viewRegion.y + m_viewRegion.h )
				m_childCanvas.y = m_viewRegion.y + m_viewRegion.h - m_childCanvas.h;
		}
	}

	//____ _updateScrollbars() ___________________________________________________

	void ScrollCapsule::_updateScrollbars( const RectSPX& oldCanvas, const RectSPX& oldView )
	{
		if (scrollbarX.isDisplayable() )
		{
			spx newViewOfs = m_viewRegion.x - m_childCanvas.x;
			spx newViewLen = m_viewRegion.w;
			spx newContentLen = m_childCanvas.w;

			spx oldViewOfs = oldView.x - oldCanvas.x;
			spx oldViewLen = oldView.w;
			spx oldContentLen = oldCanvas.w;

			if( newViewOfs != oldViewOfs || newViewLen != oldViewLen || newContentLen != oldContentLen )
				scrollbarX._update(newViewOfs, oldViewOfs, newViewLen, oldViewLen, newContentLen, oldContentLen);

			if (newViewLen == newContentLen && oldViewLen != oldContentLen)
				scrollbarX._setState(State::Disabled);
			else if (newViewLen != newContentLen && oldViewLen == oldContentLen)
				scrollbarX._setState(State::Default);
		}

		if (scrollbarY.isDisplayable() )
		{
			spx newViewOfs = m_viewRegion.y - m_childCanvas.y;
			spx newViewLen = m_viewRegion.h;
			spx newContentLen = m_childCanvas.h;

			spx oldViewOfs = oldView.y - oldCanvas.y;
			spx oldViewLen = oldView.h;
			spx oldContentLen = oldCanvas.h;

			if( newViewOfs != oldViewOfs || newViewLen != oldViewLen || newContentLen != oldContentLen )
				scrollbarY._update(newViewOfs, oldViewOfs, newViewLen, oldViewLen, newContentLen, oldContentLen);

			if (newViewLen == newContentLen && oldViewLen != oldContentLen)
				scrollbarY._setState(State::Disabled);
			else if (newViewLen != newContentLen && oldViewLen == oldContentLen)
				scrollbarY._setState(State::Default);
		}
	}

	//____ _scrollbarStep() ______________________________________________________

	void ScrollCapsule::_scrollbarStep(const Scroller* pComponent, int dir)
	{
		if (pComponent == &scrollbarX)
			_setViewOffset({ m_viewRegion.x + dir * ptsToSpx(m_stepSizeX,m_scale), (m_viewRegion.y - m_childCanvas.y) });
		else
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x), m_viewRegion.y + dir * ptsToSpx(m_stepSizeY,m_scale) });
	}

	//____ _scrollbarPage() ______________________________________________________

	void ScrollCapsule::_scrollbarPage(const Scroller* pComponent, int dir)
	{ 
		if( pComponent == &scrollbarX )
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x) + dir * (m_viewRegion.w - ptsToSpx(m_pageOverlapX,m_scale)), (m_viewRegion.y - m_childCanvas.y) });
		else
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x), (m_viewRegion.y - m_childCanvas.y) + dir * (m_viewRegion.h - ptsToSpx(m_pageOverlapY,m_scale)) });
	}

	//____ _scrollbarWheel() ______________________________________________________

	void ScrollCapsule::_scrollbarWheel(const Scroller* pComponent, int dir)
	{
		if (Base::inputHandler()->modifierKeys() & m_wheelAccelerator)
			dir *= m_wheelAccelFactor;

		if (pComponent == &scrollbarX)
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x) - dir * ptsToSpx(m_wheelStepSizeX,m_scale), (m_viewRegion.y - m_childCanvas.y) });
		else
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x), (m_viewRegion.y - m_childCanvas.y) - dir * ptsToSpx(m_wheelStepSizeY,m_scale) });
	}

	//____ _scrollbarMove() ______________________________________________________

	spx ScrollCapsule::_scrollbarMove(const Scroller* pComponent, spx pos)
	{
		if (pComponent == &scrollbarX)
		{
			_setViewOffset({ pos - m_viewRegion.x, (m_viewRegion.y - m_childCanvas.y) });
			return m_viewRegion.x - m_childCanvas.x;
		}
		else
		{
			_setViewOffset({ (m_viewRegion.x - m_childCanvas.x), pos - m_viewRegion.y });
			return m_viewRegion.y - m_childCanvas.y;
		}
	}

	//____ _scrollbarOfsLenContent() _____________________________________________

	std::tuple<spx, spx, spx> ScrollCapsule::_scrollbarOfsLenContent(const Scroller* pComponent)
	{
		if (pComponent == &scrollbarX)
			return std::make_tuple(m_viewRegion.x - m_childCanvas.x,m_viewRegion.w, m_childCanvas.w);
		else
			return std::make_tuple(m_viewRegion.y - m_childCanvas.y,m_viewRegion.h, m_childCanvas.h);
	}



}
