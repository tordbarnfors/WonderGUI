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
#include <wg2_flowpanel.h>
#include <wg2_base.h>

static const char	c_widgetType[] = {"FlowPanel"};
static const char	c_hookType[] = {"FlowHook"};


//____ WgFlowHook::Constructor ________________________________________________

WgFlowHook::WgFlowHook( WgFlowPanel * pParent )
{
	m_pParent = pParent;
	m_weight = 1.f;
}

//____ WgFlowHook::Type() _____________________________________________________

const char * WgFlowHook::Type( void ) const
{
	return ClassType();
}

//____ WgFlowHook::ClassType() ________________________________________________

const char * WgFlowHook::ClassType()
{
	return c_hookType;
}

//____ WgFlowHook::Parent() ___________________________________________________

WgFlowPanel * WgFlowHook::Parent() const
{
	return m_pParent;
}

//____ WgFlowHook::SetWeight() ________________________________________________

bool WgFlowHook::SetWeight( float weight )
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

//____ WgFlowHook::_parent() __________________________________________________

WgContainer * WgFlowHook::_parent() const
{
	return m_pParent;
}

//____ Constructor ____________________________________________________________

WgFlowPanel::WgFlowPanel()
{
	m_bHorizontal = true;
	m_pSizeBroker = 0;
}

//____ Destructor _____________________________________________________________

WgFlowPanel::~WgFlowPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgFlowPanel::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgFlowPanel::GetClass()
{
	return c_widgetType;
}

//____ SetOrientation() ______________________________________________________

void WgFlowPanel::SetOrientation( wg::Axis orientation )
{
	bool bHorizontal = orientation==wg::Axis::X?true:false;
	if( m_bHorizontal != bHorizontal )
	{
		m_bHorizontal = bHorizontal;
		_updatePreferredPixelSize();
		_refreshChildGeo();
	}
}


//____ SetSizeBroker() _______________________________________________________
/*
void WgFlowPanel::SetSizeBroker( WgSizeBroker * pBroker )
{
	if( m_pSizeBroker != pBroker )
	{
		m_pSizeBroker = pBroker;
		_updatePreferredPixelSize();
		_refreshChildGeo();
	}
}
*/

//____ PreferredPixelSize() _______________________________________________________

WgSize WgFlowPanel::PreferredPixelSize() const
{
	return m_preferredSize;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgFlowPanel::MatchingPixelHeight( int width ) const
{
	if( m_pSkin )
		width -= _skinContentPadding( m_pSkin, m_scale).w;

	int height = 0;

	if( m_bHorizontal )
	{
		WgFlowHook * pH = FirstHook();

		WgSize	row;

		while( pH )
		{
			if (pH->IsVisible())
			{
				WgSize sz = pH->_paddedPreferredPixelSize(m_scale);

				if (sz.w > width)
				{
					height += row.h;
					height += pH->_paddedMatchingPixelHeight(width, m_scale);
					row.clear();
				}
				else
				{
					if (sz.w + row.w > width)
					{
						height += row.h;
						row.clear();
					}

					row.w += sz.w;

					if (sz.h > row.h)
						row.h = sz.h;
				}

			}
			pH = pH->Next();
		}
		height += row.h;
	}
	else
	{
		height = m_preferredSize.h;
	}
	return height;
}

//____ MatchingPixelWidth() _______________________________________________________

int WgFlowPanel::MatchingPixelWidth( int height ) const
{
	if( m_pSkin )
		height -= _skinContentPadding( m_pSkin, m_scale).h;

	int width = 0;

	if( !m_bHorizontal )
	{
		WgFlowHook * pH = FirstHook();

		WgSize	row;

		while( pH )
		{
			if (pH->IsVisible())
			{
				WgSize sz = pH->_paddedPreferredPixelSize(m_scale);

				if (sz.h > height)
				{
					width += row.w;
					width += pH->_paddedMatchingPixelWidth(height, m_scale);
					row.clear();
				}
				else
				{
					if (sz.h + row.h > height)
					{
						width += row.w;
						row.clear();
					}

					row.h += sz.h;

					if (sz.w > row.w)
						row.w = sz.w;
				}

			}
			pH = pH->Next();
		}
		width += row.w;
	}
	else
	{
		width = m_preferredSize.w;
	}
	return width;
}

//____ PadAllChildren() _______________________________________________________

void WgFlowPanel::PadAllChildren( WgBorders padding )
{
    WgFlowHook * pHook = static_cast<WgFlowHook *>(m_hooks.first());
    while( pHook )
    {
        pHook->m_padding = padding;
        pHook->m_preferredSize = pHook->_paddedPreferredPixelSize(m_scale);
        pHook = pHook->Next();
    }
    
    _refreshAllWidgets();
}


//____ _firstHookWithGeo() _____________________________________________________

WgHook* WgFlowPanel::_firstHookWithGeo( WgRect& geo ) const
{
	WgFlowHook * p = FirstHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _nextHookWithGeo() _____________________________________________________

WgHook* WgFlowPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgFlowHook * p = static_cast<WgFlowHook*>(pHook)->Next();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook* WgFlowPanel::_lastHookWithGeo( WgRect& geo ) const
{
	WgFlowHook * p = LastHook();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _prevHookWithGeo() _____________________________________________________

WgHook* WgFlowPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgFlowHook * p = static_cast<WgFlowHook*>(pHook)->Prev();
	if( p )
		geo = p->m_geo;
	return p;
}

//____ _hookGeo() _____________________________________________________________

WgRect WgFlowPanel::_hookGeo( const WgVectorHook * pHook )
{
	return static_cast<const WgFlowHook*>(pHook)->m_geo;
}

//____ _onRenderRequested() ____________________________________________________

void WgFlowPanel::_onRenderRequested( WgVectorHook * pHook )
{
	WgFlowHook * p = static_cast<WgFlowHook*>(pHook);
	_requestRender( p->m_geo );
}

void WgFlowPanel::_onRenderRequested( WgVectorHook * pHook, const WgRect& rect )
{
	WgFlowHook * p = static_cast<WgFlowHook*>(pHook);
	_requestRender( rect + p->m_geo.pos() );
}

//____ _onResizeRequested() _____________________________________________________

void WgFlowPanel::_onResizeRequested( WgVectorHook * pHook )
{
	// Update cached preferred size of child

	WgFlowHook * p = static_cast<WgFlowHook*>(pHook);
	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);

	//

	_refreshAllWidgets();
}

//____ _onWidgetAppeared() ______________________________________________________

void WgFlowPanel::_onWidgetAppeared( WgVectorHook * pInserted )
{
	// Update cached preferred size of child

	WgFlowHook * p = static_cast<WgFlowHook*>(pInserted);
	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);

	//

	_refreshAllWidgets();
}

//____ _onWidgetsAppeared() ______________________________________________________

void WgFlowPanel::_onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
	// Update cached preferred size of new children

	WgFlowHook * p = static_cast<WgFlowHook*>(pFirst);

	p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);
	while( p != pLast )
	{
		p = static_cast<WgFlowHook*>(p->Next());
		p->m_preferredSize = p->_paddedPreferredPixelSize(m_scale);
	}

	//

	_refreshAllWidgets();
}


//____ _onWidgetDisappeared() ___________________________________________________

void WgFlowPanel::_onWidgetDisappeared( WgVectorHook * pToBeRemoved )
{
	_refreshAllWidgets();
}

//____ _onWidgetsDisappeared() ___________________________________________________

void WgFlowPanel::_onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
    _refreshAllWidgets();
}


//____ _onWidgetsReordered() ____________________________________________________

void WgFlowPanel::_onWidgetsReordered()
{
	_refreshChildGeo();
}

//____ _refreshAllWidgets() _____________________________________________________

void WgFlowPanel::_refreshAllWidgets()
{
	_updatePreferredPixelSize();
	_refreshChildGeo();
}


//____ _newHook() ____________________________________________________________

WgVectorHook * WgFlowPanel::_newHook()
{
	return new WgFlowHook(this);
}

//____ _onNewSize() ____________________________________________________________

void WgFlowPanel::_onNewSize( const WgSize& size )
{
	_refreshChildGeo();
}

//____ _updatePreferredPixelSize() ______________________________________________________

void WgFlowPanel::_updatePreferredPixelSize()
{
	int length = 0;
	int breadth = 0;

	{
		WgFlowHook * p = FirstHook();

		if( m_bHorizontal )
		{
			while( p )
			{
				if( p->IsVisible() )
				{
					length += p->m_preferredSize.w;
					if( p->m_preferredSize.h > breadth )
						breadth = p->m_preferredSize.h;
				}
				p = p->Next();
			}
		}
		else
		{
			while( p )
			{
				if( p->IsVisible() )
				{
					length += p->m_preferredSize.h;
					if( p->m_preferredSize.w > breadth )
						breadth = p->m_preferredSize.w;
				}
				p = p->Next();
			}
		}
	}

	//

	WgSize size = m_bHorizontal?WgSize(length,breadth):WgSize(breadth,length);

	if( m_pSkin )
		size += _skinContentPadding( m_pSkin, m_scale);

	if( size != m_preferredSize )
	{
		m_preferredSize = size;
		_requestResize();
	}
}

//____ _refreshChildGeo() _________________________________________________________

void WgFlowPanel::_refreshChildGeo()
{
	if( m_hooks.isEmpty() )
		return;

	WgRect canvas = PixelSize();
	if( m_pSkin )
		canvas = _skinContentRect( m_pSkin, canvas, m_state, m_scale);

	WgFlowHook * pH = FirstHook();
	WgRect  newGeo;

	if( m_bHorizontal )
	{
		WgRect	row( canvas.x, canvas.y, 0,0 );

		while (pH)
		{
			if (pH->IsVisible())
			{
				WgSize sz = pH->_paddedPreferredPixelSize(m_scale);
				WgBorders padding = pH->m_padding.scale(m_scale);

				if (sz.w > canvas.w)
				{
					row.y += row.h;
					row.x = canvas.x;
					row.h = 0;
					row.w = 0;

					int paddedHeight = pH->_paddedMatchingPixelHeight(canvas.w, m_scale);

					newGeo = {	row.x+padding.left,
								row.y+padding.top,
								canvas.w-padding.width(),
								paddedHeight-padding.height() };

					row.y += paddedHeight;
				}
				else
				{
					if (sz.w + row.w > canvas.w)
					{
						row.y += row.h;
						row.x = canvas.x;
						row.w = 0;
						row.h = 0;
					}

					newGeo = {	row.x + row.w + padding.left,
								row.y + padding.top,
								sz - padding.size() };

					row.w += sz.w;

					if (sz.h > row.h)
						row.h = sz.h;
				}

				if (newGeo != pH->m_geo)
				{
					_requestRender(newGeo);
					_requestRender(pH->m_geo);

					int oldW = pH->m_geo.w;
					int oldH = pH->m_geo.h;
					pH->m_geo = newGeo;
					if (newGeo.w != oldW || newGeo.h != oldH)
						pH->m_pWidget->_onNewSize(newGeo.size());
				}

			}
			else
			{
				if (pH->m_geo.w != 0 && pH->m_geo.h != 0)
					_requestRender(pH->m_geo);

				pH->m_geo.x = row.x + row.w;
				pH->m_geo.y = row.y;
				pH->m_geo.w = 0;
				pH->m_geo.h = 0;
			}

			pH = pH->Next();
		}		
	}
	else
	{
		WgRect	col( canvas.x, canvas.y, 0,0 );

		while (pH)
		{
			if (pH->IsVisible())
			{
				WgSize sz = pH->_paddedPreferredPixelSize(m_scale);
				WgBorders padding = pH->m_padding.scale(m_scale);

				if (sz.h > canvas.h)
				{
					col.x += col.w;
					col.y = canvas.y;
					col.w = 0;
					col.h = 0;

					int paddedWidth = pH->_paddedMatchingPixelWidth(canvas.h, m_scale);

					newGeo = {	col.x+padding.left,
								col.y+padding.top,
								paddedWidth-padding.width(),
								canvas.h-padding.height() };

					col.x += paddedWidth;
				}
				else
				{
					if (sz.h + col.h > canvas.h)
					{
						col.x += col.w;
						col.y = canvas.y;
						col.w = 0;
						col.h = 0;
					}

					newGeo = {	col.x + padding.left,
								col.y + col.h + padding.top,
								sz - padding.size() };

					col.h += sz.h;

					if (sz.w > col.w)
						col.w = sz.w;
				}

				if (newGeo != pH->m_geo)
				{
					_requestRender(newGeo);
					_requestRender(pH->m_geo);

					int oldW = pH->m_geo.w;
					int oldH = pH->m_geo.h;
					pH->m_geo = newGeo;
					if (newGeo.w != oldW || newGeo.h != oldH)
						pH->m_pWidget->_onNewSize(newGeo.size());
				}

			}
			else
			{
				if (pH->m_geo.w != 0 && pH->m_geo.h != 0)
					_requestRender(pH->m_geo);

				pH->m_geo.x = col.x;
				pH->m_geo.y = col.y + col.h;
				pH->m_geo.w = 0;
				pH->m_geo.h = 0;
			}

			pH = pH->Next();
		}

	}
	

}

//____ _populateSizeBrokerArray() ___________________________________________

int WgFlowPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const
{
	WgFlowHook * pH = FirstHook();
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
				pI++;
			}
			pH = pH->Next();
		}
	}

	return int(pI - pArray);
}

int WgFlowPanel::_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const
{
	WgFlowHook * pH = FirstHook();
	WgSizeBrokerItem * pI = pArray;

	if( m_bHorizontal )
	{
		while( pH )
		{
			if( pH->IsVisible() )
			{
				pI->preferred = pH->_paddedMatchingPixelWidth(forcedBreadth, m_scale);
				pI->min = pH->_paddedMinPixelSize(m_scale).w;
				pI->max = pH->_paddedMaxPixelSize(m_scale).w;
				pI->weight = pH->m_weight;
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
				pI++;
			}
			pH = pH->Next();
		}
	}

	return int(pI - pArray);
}
