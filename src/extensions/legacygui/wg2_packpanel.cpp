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
#include <wg2_packpanel.h>
#include <wg2_base.h>
#include <algorithm>

static const char	c_widgetType[] = {"PackPanel"};
static const char	c_hookType[] = {"PackHook"};


//____ WgPackHook::Constructor ________________________________________________

WgPackHook::WgPackHook( WgPackPanel * pParent )
{
	m_pParent = pParent;
	m_weight = 1.f;
	m_baseline = 0.f;
}

//____ WgPackHook::Type() _____________________________________________________

const char * WgPackHook::Type( void ) const
{
	return ClassType();
}

//____ WgPackHook::ClassType() ________________________________________________

const char * WgPackHook::ClassType()
{
	return c_hookType;
}

//____ WgPackHook::Parent() ___________________________________________________

WgPackPanel * WgPackHook::Parent() const
{
	return m_pParent;
}

//____ WgPackHook::SetWeight() ________________________________________________

bool WgPackHook::SetWeight( float weight )
{
	if( weight < 0 )
		return false;

	if( weight != m_weight )
	{
		m_weight = weight;
		Parent()->_refreshChildGeo();
	}
	return true;
}

//____ WgPackHook::SetBaseline() ______________________________________________

bool WgPackHook::SetBaseline( float baseline )
{
	wg::limit(baseline, 0.f, 1.f);

	if( baseline != m_baseline )
	{
		m_baseline = baseline;
		Parent()->_refreshBaseline();
	}
	return true;
}


//____ WgPackHook::_parent() __________________________________________________

WgContainer * WgPackHook::_parent() const
{
	return m_pParent;
}

//____ Constructor ____________________________________________________________

WgPackPanel::WgPackPanel()
{
	m_bHorizontal = true;
	m_bBaselineMode = false;
	m_pSizeBroker = nullptr;
	m_bSiblingsOverlap = false;
}

//____ Destructor _____________________________________________________________

WgPackPanel::~WgPackPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgPackPanel::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgPackPanel::GetClass()
{
	return c_widgetType;
}


void WgPackPanel::SetSkin(wg::Skin* pSkin)
{
	bool bGeoAffected = false;
	if (!m_pSkin || !pSkin || (pSkin->defaultSize() != m_pSkin->defaultSize()) || (pSkin->_contentBorder(m_scale, m_state) != m_pSkin->_contentBorder(m_scale, m_state)) )
		bGeoAffected = true;

	WgPanel::SetSkin(pSkin);

	if (bGeoAffected)
	{
		m_bChildGeoNeedsRefresh = true;
		_updatePreferredPixelSize();
		if (m_bChildGeoNeedsRefresh)
			_refreshChildGeo();
	}

}


//____ SetOrientation() ______________________________________________________

void WgPackPanel::SetOrientation( wg::Axis orientation )
{
	bool bHorizontal = orientation==wg::Axis::X?true:false;
	if( m_bHorizontal != bHorizontal )
	{
		m_bHorizontal = bHorizontal;

		if( m_bFreezeGeo )
			m_bWantsGeoUpdate = true;
		else
		{
			m_bChildGeoNeedsRefresh = true;
			_updatePreferredPixelSize();
			if( m_bChildGeoNeedsRefresh )
				_refreshChildGeo();
		}
	}
}


//____ SetSizeBroker() _______________________________________________________

void WgPackPanel::SetSizeBroker( WgSizeBroker * pBroker )
{
	if( m_pSizeBroker != pBroker )
	{
		m_pSizeBroker = pBroker;

		if( m_bFreezeGeo )
			m_bWantsGeoUpdate = true;
		else
		{
			m_bChildGeoNeedsRefresh = true;
			_updatePreferredPixelSize();
			if( m_bChildGeoNeedsRefresh )
				_refreshChildGeo();
		}
	}
}

//____ SetBaselineMode() ______________________________________________________

void WgPackPanel::SetBaselineMode( bool bBaseline )
{
	if( bBaseline != m_bBaselineMode )
	{
		m_bBaselineMode = bBaseline;
		_refreshBaseline();
	}
}

//____ PreferredPixelSize() _______________________________________________________

WgSize WgPackPanel::PreferredPixelSize() const
{
	return m_pSkin ? m_preferredSize + _skinContentPadding( m_pSkin, m_scale ): m_preferredSize;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgPackPanel::MatchingPixelHeight( int width ) const
{
	if( m_pSkin )
		width -= _skinContentPadding( m_pSkin, m_scale ).w;

	int height = 0;

	if( m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array

			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(wg::GfxBase::memStackAlloc(arrayBytes));

			int nItems = _populateSizeBrokerArray(pItemArea);

			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->SetItemLengths( pItemArea, nItems, width );

			WgPackHook * pH = FirstHook();
			WgSizeBrokerItem * pI = pItemArea;


			if( m_bBaselineMode )
			{
				int maxAscend = 0;
				int maxDescend = 0;

				while( pH )
				{
					if( pH->IsVisible() )
					{
						int b = pH->_paddedMatchingPixelHeight( pI->output, m_scale );
						int ascend = int(b * pH->m_baseline);
						int descend = b - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;
					}
					pH = pH->Next();
				}

				height = maxAscend + maxDescend;
			}
			else
			{
				while( pH )
				{
					if( pH->IsVisible() )
					{
						int itemHeight = pH->_paddedMatchingPixelHeight( pI->output, m_scale );
						if( itemHeight > height )
							height = itemHeight;
						pI++;
					}

					pH = pH->Next();
				}

			}


			// Release temporary memory area

			wg::GfxBase::memStackFree(arrayBytes);
		}
		else
		{
			WgPackHook * pH = FirstHook();

			if( m_bBaselineMode )
			{
				int maxAscend = 0;
				int maxDescend = 0;

				while( pH )
				{
					if( pH->IsVisible() )
					{
						int b = pH->m_preferredSize.h;
						int ascend = int(b * pH->m_baseline);
						int descend = b - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;
					}
					pH = pH->Next();
				}

				height = maxAscend + maxDescend;
			}
			else
			{
				while( pH )
				{
					if( pH->IsVisible() && pH->m_preferredSize.h > height )
						height = pH->m_preferredSize.h;

					pH = pH->Next();
				}

			}
		}
	}
	else
	{
		WgPackHook * p = FirstHook();

		while( p )
		{
			if( p->IsVisible() )
				height += p->_paddedMatchingPixelHeight( width, m_scale );

			p = p->Next();
		}
	}

	if( m_pSkin )
		height += _skinContentPadding( m_pSkin, m_scale ).h;

	return height;
}

//____ MatchingPixelWidth() _______________________________________________________

int WgPackPanel::MatchingPixelWidth( int height ) const
{
	if( m_pSkin )
		height -= _skinContentPadding( m_pSkin, m_scale ).h;

	int width = 0;

	if( !m_bHorizontal )
	{
		if( m_pSizeBroker )
		{
			// Allocate and populate SizeBroker array

			int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
			WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(wg::GfxBase::memStackAlloc(arrayBytes));

			int nItems = _populateSizeBrokerArray(pItemArea);

			// Retrieve item lengths and find height of highest item.

			m_pSizeBroker->SetItemLengths( pItemArea, nItems, height );

			WgPackHook * pH = FirstHook();
			WgSizeBrokerItem * pI = pItemArea;

			if( m_bBaselineMode )
			{
				int maxAscend = 0;
				int maxDescend = 0;

				while( pH )
				{
					if( pH->IsVisible() )
					{
						int b = pH->_paddedMatchingPixelWidth( pI->output, m_scale );
						int ascend = int(b * pH->m_baseline);
						int descend = b - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;
					}
					pH = pH->Next();
				}

				width = maxAscend + maxDescend;
			}
			else
			{
				while( pH )
				{
					if( pH->IsVisible() )
					{
						int itemWidth = pH->_paddedMatchingPixelWidth( pI->output, m_scale );
						if( itemWidth > width )
							width = itemWidth;
						pI++;
					}

					pH = pH->Next();
				}

			}


			// Release temporary memory area

			wg::GfxBase::memStackFree(arrayBytes);
		}
		else
		{
			WgPackHook * pH = FirstHook();

			if( m_bBaselineMode )
			{
				int maxAscend = 0;
				int maxDescend = 0;

				while( pH )
				{
					if( pH->IsVisible() )
					{
						int b = pH->m_preferredSize.w;
						int ascend = int(b * pH->m_baseline);
						int descend = b - ascend;

						if( ascend > maxAscend )
							maxAscend = ascend;

						if( descend > maxDescend )
							maxDescend = descend;
					}
					pH = pH->Next();
				}

				width = maxAscend + maxDescend;
			}
			else
			{
				while( pH )
				{
					if( pH->IsVisible() && pH->m_preferredSize.w > width )
						width = pH->m_preferredSize.w;

					pH = pH->Next();
				}

			}
		}
	}
	else
	{
		WgPackHook * p = FirstHook();

		if( m_bBaselineMode )
		{
			while( p )
			{
				if( p->IsVisible() )
				{
					int h = std::min( height, p->m_preferredSize.h );
					width += p->_paddedMatchingPixelWidth( h, m_scale );
				}
				p = p->Next();
			}
		}
		else
		{
			while( p )
			{
				if( p->IsVisible() )
					width += p->_paddedMatchingPixelWidth( height, m_scale );

				p = p->Next();
			}
		}
	}

	if( m_pSkin )
		width += _skinContentPadding( m_pSkin, m_scale ).w;

	return width;
}

//____ FreezeGeo() ____________________________________________________________

void WgPackPanel::FreezeGeo()
{
	m_bFreezeGeoExt = true;
	m_bFreezeGeo = true;
}

//____ UnfreezeGeo() __________________________________________________________

void WgPackPanel::UnfreezeGeo()
{
	m_bFreezeGeoExt = false;
	m_bFreezeGeo = false;
	if( m_bWantsGeoUpdate )
	{
		_refreshAllWidgets();
		m_bWantsGeoUpdate = false;
	}
}


//____ _setScale() ____________________________________________________________

void WgPackPanel::_setScale(int scale)
{
	m_bFreezeGeo = true;
	WgVectorPanel::_setScale(scale);
	m_bFreezeGeo = m_bFreezeGeoExt;

	if (!m_bFreezeGeo && m_bWantsGeoUpdate)
	{
		_refreshAllWidgets();
		m_bWantsGeoUpdate = false;
	}
}



//____ _firstHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_firstHookWithGeo( WgRect& geo ) const
{
	WgPackHook * p = FirstHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->Next();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_lastHookWithGeo( WgRect& geo ) const
{
	WgPackHook * p = LastHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook* WgPackPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook)->Prev();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _hookGeo() _____________________________________________________________

WgRect WgPackPanel::_hookGeo( const WgVectorHook * pHook )
{
	return static_cast<const WgPackHook*>(pHook)->m_geo;
}

//____ _onRenderRequested() ____________________________________________________

void WgPackPanel::_onRenderRequested( WgVectorHook * pHook )
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	_requestRender( p->m_geo );
}

void WgPackPanel::_onRenderRequested( WgVectorHook * pHook, const WgRect& rect )
{
	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	_requestRender( rect + p->m_geo.pos() );
}

//____ _onResizeRequested() _____________________________________________________

void WgPackPanel::_onResizeRequested( WgVectorHook * pHook )
{
	// Update cached preferred size of child

	WgPackHook * p = static_cast<WgPackHook*>(pHook);
	WgSize oldPreferred = p->m_preferredSize;
	WgSize newPreferred = p->_paddedPreferredPixelSize(m_scale);
	p->m_preferredSize = newPreferred;

	WgBorders padding = p->m_padding.scale(m_scale);

	int oldWidthForHeight = p->m_preferredWidthForHeight;
	int newWidthForHeight = p->m_pWidget->MatchingPixelWidth(p->m_geo.h) + padding.width();
	p->m_preferredWidthForHeight = newWidthForHeight;

	int oldHeightForWidth = p->m_preferredHeightForWidth;
	int newHeightForWidth = p->m_pWidget->MatchingPixelHeight(p->m_geo.w) + padding.height();
	p->m_preferredHeightForWidth = newHeightForWidth;

	// Early out if we can determine that the request will have no effect on geo

	bool bNeedFullPreferredSizeRefresh = false;

	if( m_bBaselineMode )
	{
		bNeedFullPreferredSizeRefresh = true;
	}
	else
	{
		if( newPreferred == oldPreferred && newHeightForWidth == oldHeightForWidth && newWidthForHeight == oldWidthForHeight )
		{
//            _requestResize();
			return;
		}

		if( m_bHorizontal )
		{
			if( newPreferred.h != oldPreferred.h && oldPreferred.h == m_preferredSize.h )
				bNeedFullPreferredSizeRefresh = true;

			if( newPreferred.h <=  m_preferredSize.h && oldPreferred.h < m_preferredSize.h )
			{
				if( newHeightForWidth == oldHeightForWidth && newWidthForHeight == oldWidthForHeight )
					return;
			}

		}
		else
		{
			if( newPreferred.w != oldPreferred.w && oldPreferred.w == m_preferredSize.w )
				bNeedFullPreferredSizeRefresh = true;

			if( newPreferred.w <=  m_preferredSize.w && oldPreferred.w < m_preferredSize.w )
			{
				if( newHeightForWidth == oldHeightForWidth &&
					newWidthForHeight == oldWidthForHeight )
					return;
			}
		}
	}

	//

	if (m_bFreezeGeo)
		m_bWantsGeoUpdate = true;
	else
	{
		m_bChildGeoNeedsRefresh = true;

		if( bNeedFullPreferredSizeRefresh )
			_updatePreferredPixelSize();
		else
		{
			if( m_bHorizontal )
			{
				m_preferredSize.w += newPreferred.w - oldPreferred.w;
				if( newPreferred.h > m_preferredSize.h )
					m_preferredSize.h = newPreferred.h;
			}
			else
			{
				m_preferredSize.h += newPreferred.h - oldPreferred.h;
				if( newPreferred.w > m_preferredSize.w )
					m_preferredSize.w = newPreferred.w;
			}
			_requestResize();
		}

		if( m_bChildGeoNeedsRefresh )
			_refreshChildGeo();
	}
}

//____ _onWidgetsAppeared() ______________________________________________________

void WgPackPanel::_onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
	// Update cached preferred size of children

	WgPackHook * p = static_cast<WgPackHook*>(pFirst);

	do {
		p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);

		WgBorders padding = p->m_padding.scale(m_scale);
		p->m_preferredWidthForHeight = p->m_pWidget->MatchingPixelWidth(p->m_geo.h) + padding.width();
		p->m_preferredHeightForWidth = p->m_pWidget->MatchingPixelHeight(p->m_geo.w) + padding.height();

		p = static_cast<WgPackHook*>(p->Next());

	} while (p != pLast );

	// Update cached preferred size of us

	if( m_bFreezeGeo )
		m_bWantsGeoUpdate = true;
	else
		_refreshAllWidgets();
}

//____ _onWidgetAppeared() ______________________________________________________

void WgPackPanel::_onWidgetAppeared( WgVectorHook * pInserted )
{
	// Update cached preferred size of child

	WgPackHook * p = static_cast<WgPackHook*>(pInserted);
	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);

	WgBorders padding = p->m_padding.scale(m_scale);
	p->m_preferredWidthForHeight = p->m_pWidget->MatchingPixelWidth(p->m_geo.h) + padding.width();
	p->m_preferredHeightForWidth = p->m_pWidget->MatchingPixelHeight(p->m_geo.w) + padding.height();

	// Update cached preferred size of us

	if( m_bFreezeGeo )
		m_bWantsGeoUpdate = true;
	else
		_refreshAllWidgets();

/*
	if( m_bBaselineMode )
	{
		_refreshAllWidgets();
	}
	else
	{
		if( m_bHorizontal )
		{
			m_preferredSize.w += p->m_preferredSize.w;
			m_preferredSize.h = std::max( m_preferredSize.h, p->m_preferredSize.h);
		}
		else
		{
			m_preferredSize.h += p->m_preferredSize.h;
			m_preferredSize.w = std::max( m_preferredSize.w, p->m_preferredSize.w);
		}

		m_bChildGeoNeedsRefresh = true;
		_requestResize();
		if( m_bChildGeoNeedsRefresh )
			_refreshChildGeo();
	}
*/
}

//____ _onWidgetDisappeared() ___________________________________________________

void WgPackPanel::_onWidgetDisappeared( WgVectorHook * pToBeRemoved )
{
	if( m_bFreezeGeo )
		m_bWantsGeoUpdate = true;
	else
		_refreshAllWidgets();
}

//____ _onWidgetsDisappeared() ___________________________________________________

void WgPackPanel::_onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
    if( m_bFreezeGeo )
        m_bWantsGeoUpdate = true;
    else
        _refreshAllWidgets();
}


//____ _onWidgetsReordered() ____________________________________________________

void WgPackPanel::_onWidgetsReordered()
{
	if( m_bFreezeGeo )
		m_bWantsGeoUpdate = true;
	else
		_refreshAllWidgets();
}

//____ _refreshAllWidgets() _____________________________________________________

void WgPackPanel::_refreshAllWidgets()
{
	m_bChildGeoNeedsRefresh = true;
	_updatePreferredPixelSize();
	if( m_bChildGeoNeedsRefresh )
		_refreshChildGeo();
}


//____ _newHook() ____________________________________________________________

WgVectorHook * WgPackPanel::_newHook()
{
	return new WgPackHook(this);
}

//____ _onNewSize() ____________________________________________________________

void WgPackPanel::_onNewSize( const WgSize& size )
{
	_refreshChildGeo();
}


//____ _updatePreferredPixelSize() ______________________________________________________

void WgPackPanel::_updatePreferredPixelSize()
{
	int length = 0;
	int breadth = 0;

	WgPackHook * pH = FirstHook();

	if( m_bBaselineMode )
	{
		int maxAscend = 0;
		int maxDescend = 0;

		if( m_bHorizontal )
		{
			while( pH )
			{
				if( pH->IsVisible() )
				{
					length += pH->m_preferredSize.w;

					int b = pH->m_preferredSize.h;
					int ascend = int(b * pH->m_baseline);
					int descend = b - ascend;

					if( ascend > maxAscend )
						maxAscend = ascend;

					if( descend > maxDescend )
						maxDescend = descend;
				}
				pH = pH->Next();
			}
		}
		else
		{
			while( pH )
			{
				if( pH->IsVisible() )
				{
					length += pH->m_preferredSize.h;

					int b = pH->m_preferredSize.w;
					int ascend = int(b * pH->m_baseline);
					int descend = b - ascend;

					if( ascend > maxAscend )
						maxAscend = ascend;

					if( descend > maxDescend )
						maxDescend = descend;
				}
				pH = pH->Next();
			}
		}

		breadth = maxAscend + maxDescend;
	}
	else
	{
		if( m_bHorizontal )
		{
			while( pH )
			{
				if( pH->IsVisible() )
				{
					length += pH->m_preferredSize.w;
					if( pH->m_preferredSize.h > breadth )
						breadth = pH->m_preferredSize.h;
				}
				pH = pH->Next();
			}
		}
		else
		{
			while( pH)
			{
				if( pH->IsVisible() )
				{
					length += pH->m_preferredSize.h;
					if( pH->m_preferredSize.w > breadth )
						breadth = pH->m_preferredSize.w;
				}
				pH = pH->Next();
			}
		}
	}

	//

	WgSize size = m_bHorizontal?WgSize(length,breadth):WgSize(breadth,length);
// This optimization caused trouble for wrap text. MatchingHeight might get different even if PreferredSize remains the same.
//	if( size != m_preferredSize )
//	{
		m_preferredSize = size;
		_requestResize();
//	}
}

//____ _refreshBaseline() ________________________________________________________

void WgPackPanel::_refreshBaseline()
{
	m_bChildGeoNeedsRefresh = true;
	_updatePreferredPixelSize();

	if( m_bChildGeoNeedsRefresh )
		_refreshChildGeo();
}


//____ _refreshChildGeo() _________________________________________________________

void WgPackPanel::_refreshChildGeo()
{
	m_bChildGeoNeedsRefresh = false;

	if( m_hooks.isEmpty() )
		return;

	WgRect contentRect = PixelSize();
	if (m_pSkin)
		contentRect = _skinContentRect( m_pSkin, contentRect, WgStateEnum::Default, m_scale);

	WgSize size = contentRect.size();

	int wantedLength = m_bHorizontal?m_preferredSize.w:m_preferredSize.h;
	int givenLength = m_bHorizontal?size.w:size.h;
	int givenBreadth = m_bHorizontal?size.h:size.w;

	// Allocate temporary space for childrens individual sizes

	int childGeosBytes = sizeof(WgRect)*m_hooks.size();
	WgRect * pChildGeos = reinterpret_cast<WgRect*>(wg::GfxBase::memStackAlloc(childGeosBytes));

	// Optimized special case, just copy preferred to length.

	if( !m_pSizeBroker || (wantedLength == givenLength && !m_pSizeBroker->OverridesDefaultSizes()) )
	{
		WgCoord pos = contentRect.pos();
		WgPackHook * p = FirstHook();
		WgRect * pGeo = pChildGeos;

		if( m_bHorizontal )
		{
			int endPos = contentRect.x + contentRect.w;
			while( p )
			{
				pGeo->x = pos.x;
				pGeo->y = pos.y;

				if( p->IsVisible() )
				{
					int breadth = m_bBaselineMode ? std::min(p->m_preferredSize.h,givenBreadth) : givenBreadth;
					int length = p->_paddedMatchingPixelWidth(breadth,m_scale);

					if( pos.x + length > endPos )
						length = endPos - pos.x;

					pGeo->w = length;
					pos.x += length;
				}
				else
					pGeo->w = 0;

				p = p->Next();
				pGeo++;
			}
		}
		else
		{
			int endPos = contentRect.y + contentRect.h;
			while( p )
			{
				pGeo->x = pos.x;
				pGeo->y = pos.y;

				if( p->IsVisible() )
				{
					int breadth = m_bBaselineMode ? std::min(p->m_preferredSize.w,givenBreadth) : givenBreadth;
					int length = p->_paddedMatchingPixelHeight(breadth,m_scale);

					if( pos.y + length > endPos )
						length = endPos - pos.y;

					pGeo->h = length;
					pos.y += length;
				}
				else
					pGeo->h = 0;

				p = p->Next();
				pGeo++;
			}

		}

	}
	else
	{
		// Allocate and populate SizeBroker array

		int arrayBytes = sizeof(WgSizeBrokerItem)*m_hooks.size();
		WgSizeBrokerItem * pItemArea = reinterpret_cast<WgSizeBrokerItem*>(wg::GfxBase::memStackAlloc(arrayBytes));

		int nItems = _populateSizeBrokerArray(pItemArea, givenBreadth);

		// Retrieve length and set geo for all children, call _requestRender() and _onNewSize() where needed.

		m_pSizeBroker->SetItemLengths( pItemArea, nItems, givenLength );

		WgPackHook * pH = FirstHook();
		WgSizeBrokerItem * pI = pItemArea;

		WgCoord pos = contentRect.pos();
		WgRect * pGeo = pChildGeos;

		if( m_bHorizontal )
		{
			while( pH )
			{
				pGeo->x = pos.x;
				pGeo->y = pos.y;
				if( pH->IsVisible() )
				{
					pGeo->w = pI->output;
					pos.x += pI->output;
					pI++;
				}
				else
					pGeo->w = 0;

				pH = pH->Next();
				pGeo++;
			}
		}
		else
		{
			while( pH )
			{
				pGeo->x = pos.x;
				pGeo->y = pos.y;
				if( pH->IsVisible() )
				{
					pGeo->h = pI->output;
					pos.y += pI->output;
					pI++;
				}
				else
					pGeo->h = 0;

				pH = pH->Next();
				pGeo++;
			}
		}

		// Release SizeBroker array

		wg::GfxBase::memStackFree(arrayBytes);
	}

	// Make adjustments to breadth according to mode and possibly adjust position.

	if( m_bBaselineMode )
	{
		WgPackHook * p = FirstHook();
		WgRect * pGeo = pChildGeos;

		int baseLineOfs = 0;

		if( m_bHorizontal )
		{
			while( p )
			{
				if( pGeo->w == p->m_preferredSize.w )
					pGeo->h = p->m_preferredSize.h;
				else
					pGeo->h = p->_paddedMatchingPixelHeight(pGeo->w, m_scale);

				int ascend = int(pGeo->h * p->m_baseline);
				if( ascend > baseLineOfs )
					baseLineOfs = ascend;

				p = p->Next();
				pGeo++;
			}

			p = FirstHook();
			pGeo = pChildGeos;
			while( p )
			{
				// Todo adjust y,h so that it fits.

				int ascend = int(pGeo->h * p->m_baseline);
				pGeo->y = contentRect.y + baseLineOfs - ascend;
				p = p->Next();
				pGeo++;
			}
		}
		else
		{
			while( p )
			{
				if( pGeo->h == p->m_preferredSize.h )
					pGeo->w = p->m_preferredSize.w;
				else
					pGeo->w = p->_paddedMatchingPixelWidth(pGeo->h, m_scale);

				int ascend = int(pGeo->w * p->m_baseline);
				if( ascend > baseLineOfs )
					baseLineOfs = ascend;

				p = p->Next();
				pGeo++;
			}

			p = FirstHook();
			pGeo = pChildGeos;
			while( p )
			{
				// Todo adjust x,w so that it fits.

				int ascend = int(pGeo->w * p->m_baseline);
				pGeo->x = contentRect.x + baseLineOfs - ascend;
				p = p->Next();
				pGeo++;
			}
		}

		m_baseline = baseLineOfs/(float) givenBreadth;
	}
	else
	{
		WgPackHook * p = FirstHook();
		WgRect * pGeo = pChildGeos;

		if( m_bHorizontal )
		{
			while( p )
			{
				pGeo->h = size.h;
				p = p->Next();
				pGeo++;
			}
		}
		else
		{
			while( p )
			{
				pGeo->w = size.w;
				p = p->Next();
				pGeo++;
			}
		}
	}


	// Update childrens real geo

	WgPackHook * p = FirstHook();
	WgRect * pNewGeo = pChildGeos;

	while( p )
	{
		if( p->IsVisible() )
		{
			WgRect geo = (* pNewGeo) - p->m_padding.scale(m_scale);
			
			if( geo.w < 0 )
				geo.w = 0;
			if( geo.h < 0 )
				geo.h = 0;
			
			if( geo != p->m_geo )
			{
				_requestRender(p->m_geo);
				_requestRender(geo);

				int oldW = p->m_geo.w;
				int oldH = p->m_geo.h;
				p->m_geo = geo;
				if( geo.w != oldW || geo.h != oldH )
					p->m_pWidget->_onNewSize( geo.size() );
			}
		}
		else
		{
			if( p->m_geo.w != 0 && p->m_geo.h != 0 )
			{
				_requestRender(p->m_geo);
				p->m_geo.w = pNewGeo->w;
				p->m_geo.h = pNewGeo->h;
				p->m_pWidget->_onNewSize( pNewGeo->size() );
			}
		}
		p = p->Next();
		pNewGeo++;
	}

	// Release temporary memory

	wg::GfxBase::memStackFree(childGeosBytes);
}

//____ _populateSizeBrokerArray() ___________________________________________

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const
{
	WgPackHook * pH = FirstHook();
	WgSizeBrokerItem * pI = pArray;

	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->m_preferredSize.w;
				pI->min = pH->_paddedMinPixelSize(m_scale).w;
				pI->max = pH->_paddedMaxPixelSize(m_scale).w;
				pI->weight = pH->m_weight;
				// This can't be undefined
				pI->output = 0;
				pI++;
			}
			pH = pH->Next();
		}
	}
	else
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->m_preferredSize.h;
				pI->min = pH->_paddedMinPixelSize(m_scale).h;
				pI->max = pH->_paddedMaxPixelSize(m_scale).h;
				pI->weight = pH->m_weight;
				// This can't be undefined
				pI->output = 0;
				pI++;
			}
			pH = pH->Next();
		}
	}

	return (int) (pI - pArray);
}

int WgPackPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const
{
	WgPackHook * pH = FirstHook();
	WgSizeBrokerItem * pI = pArray;

	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->_paddedMatchingPixelWidth(forcedBreadth,m_scale);
				pI->min = pH->_paddedMinPixelSize(m_scale).w;
				pI->max = pH->_paddedMaxPixelSize(m_scale).w;
				pI->weight = pH->m_weight;
				// This can't be undefined
				pI->output = 0;
				pI++;
			}
			pH = pH->Next();
		}
	}
	else
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->_paddedMatchingPixelHeight(forcedBreadth, m_scale);
				pI->min = pH->_paddedMinPixelSize(m_scale).h;
				pI->max = pH->_paddedMaxPixelSize(m_scale).h;
				pI->weight = pH->m_weight;
				// This can't be undefined
				pI->output = 0;
				pI++;
			}
			pH = pH->Next();
		}
	}

	return (int) (pI - pArray);
}
