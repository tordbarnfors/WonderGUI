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

namespace wg
{
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

		bool bScrollX = m_bScrollX && scrollbarX.isDisplayable();
		bool bScrollY = m_bScrollY && scrollbarY.isDisplayable();



		// If we don't have a child or don't scroll, the whole area is view region and there are no scrollbars.

		if (!pChild || !(bScrollX || bScrollY) )
		{
			m_scrollbarXRegion = { window.x, window.y + window.h, window.w, 0 };
			m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			m_viewRegion = window;
			m_childCanvas = window;
			return;
		}

		// If we scroll in both directions

		if (bScrollX && bScrollY)
		{
/*
			SizeSPX canvasSize = pChild->_defaultSize(m_scale);

			bool bScrollbarX = (canvasSize.w > window.w || !m_bAutoHideScrollbars);
			bool bScrollbarY = (canvasSize.w > window.w || !m_bAutoHideScrollbars);




			if (canvasSize.w < window.w && canvasSize.h < window.h)
			{
				m_viewRegion = window;
				m_childCanvas = window;
			}

*/


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
					m_childCanvas.h = canvasHeight;
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
					m_childCanvas.w = canvasWidth;
					m_childCanvas.h = m_viewRegion.h;
					m_scrollbarXRegion = { window.x, window.y + window.h, window.x, 0 };
				}
				m_scrollbarYRegion = { window.x + window.w, window.y, 0, window.h };
			}
		}

	}




	void ScrollCapsule::_scrollbarStep(const Scroller* pComponent, int dir)
	{
	}

	void ScrollCapsule::_scrollbarPage(const Scroller* pComponent, int dir)
	{ 
	}

	void ScrollCapsule::_scrollbarWheel(const Scroller* pComponent, int dir)
	{

	}
		
	spx ScrollCapsule::_scrollbarMove(const Scroller* pComponent, spx pos)
	{
		return 0;
	}

	std::tuple<spx, spx, spx> ScrollCapsule::_scrollbarOfsLenContent(const Scroller* pComponent)
	{
		return std::make_tuple(0, 0, 0);
	}



}