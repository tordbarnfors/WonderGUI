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
#include <wg2_stackpanel.h>
#include <wg2_util.h>
#include <wg2_patches.h>

static const char	c_widgetType[] = {"StackPanel"};
static const char	c_hookType[] = {"StackHook"};


void WgStackHook::SetSizePolicy( SizePolicy policy )
{
	if( policy != m_sizePolicy )
	{
		m_pParent->_onRenderRequested(this);
		m_sizePolicy = policy;
		m_pParent->_onRenderRequested(this);
	};
}

void WgStackHook::SetOrigo( WgOrigo origo )
{
	if( origo != m_origo )
	{
		m_pParent->_onRenderRequested(this);
		m_origo = origo;
		m_pParent->_onRenderRequested(this);
	}
}

WgStackHook::WgStackHook( WgStackPanel * pParent ): m_pParent(pParent), m_origo( WgOrigo::NorthWest ), m_sizePolicy( STRETCH )
{
}


const char * WgStackHook::Type( void ) const
{
	return ClassType();
}

const char * WgStackHook::ClassType()
{
	return c_hookType;
}


WgContainer * WgStackHook::_parent() const
{
	return m_pParent;
}



WgRect WgStackHook::_getGeo( const WgRect& parentGeo ) const
{
	WgRect base = parentGeo - m_padding;

	if( base.w <= 0 || base.h <= 0 )
		return WgRect(0,0,0,0);

	switch( m_sizePolicy )
	{
		default:
		case DEFAULT:
		{
			WgSize	size = m_pWidget->PreferredPixelSize();
			WgRect geo = WgUtil::OrigoToRect( m_origo, base, size );

			if( geo.w > base.w )
			{
				geo.x = 0;
				geo.w = base.w;
			}

			if( geo.h > base.h )
			{
				geo.y = 0;
				geo.h = base.h;
			}
			return geo;
		}
		case STRETCH:
		{
			return base;
		}
		case SCALE:
		{
			WgSize	orgSize = m_pWidget->PreferredPixelSize();
			WgSize	size;

			float	fracX = orgSize.w / (float) base.w;
			float	fracY = orgSize.h / (float) base.h;

			if( fracX > fracY )
			{
				size.w = base.w;
				size.h = int (orgSize.h / fracX);
			}
			else
			{
				size.h = base.h;
				size.w = (int) (orgSize.w / fracY);
			}

			return WgUtil::OrigoToRect( m_origo, base, size );
		}
	}
}


//____ Constructor ____________________________________________________________

WgStackPanel::WgStackPanel()
{
	m_bSiblingsOverlap = true;
}

//____ Destructor _____________________________________________________________

WgStackPanel::~WgStackPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgStackPanel::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgStackPanel::GetClass()
{
	return c_widgetType;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgStackPanel::MatchingPixelHeight( int width ) const
{
	int height = 0;

	WgStackHook * pHook = FirstHook();
	while( pHook )
	{
		if (pHook->m_bVisible)
		{
			int h = pHook->Widget()->MatchingPixelHeight(width);
			if( h > height )
				height = h;
		}
		pHook = pHook->Next();
	}

	return height;
}

//____ MatchingPixelWidth() _______________________________________________________

int WgStackPanel::MatchingPixelWidth( int height ) const
{
	int width = 0;

	WgStackHook * pHook = FirstHook();
	while( pHook )
	{
		if (pHook->m_bVisible)
		{
			int w = pHook->Widget()->MatchingPixelWidth(height);
			if (w > width)
				width = w;
		}
		pHook = pHook->Next();
	}

	return width;
}


//____ PreferredPixelSize() _____________________________________________________________

WgSize WgStackPanel::PreferredPixelSize() const
{
	return m_preferredSize;
}

//____ _onNewSize() ___________________________________________________________

void WgStackPanel::_onNewSize( const WgSize& size )
{
	m_size = size;
	_adaptChildrenToSize();
}


//____ _hookGeo() _____________________________________________________________

WgRect WgStackPanel::_hookGeo( const WgVectorHook * pHook )
{
	return ((WgStackHook*)pHook)->_getGeo(m_size);
}

//____ _onResizeRequested() ____________________________________________________

void WgStackPanel::_onResizeRequested( WgVectorHook * _pHook )
{
	_refreshPreferredPixelSize();
}

//____ _onRenderRequested() ____________________________________________________

void WgStackPanel::_onRenderRequested( WgVectorHook * _pHook )
{
	WgStackHook * pHook = static_cast<WgStackHook*>(_pHook);

	_onRenderRequested(pHook, pHook->_getGeo(WgRect(0,0,m_size)));
}

void WgStackPanel::_onRenderRequested( WgVectorHook * _pHook, const WgRect& _rect )
{
	WgStackHook * pHook = static_cast<WgStackHook*>(_pHook);

	if( !pHook->IsVisible() )
		return;

	// Put our rectangle into patches

	WgRect rect = _rect + pHook->_getGeo(WgRect(0,0,m_size)).pos();


	WgPatches patches;
	patches.add( rect );

	// Remove portions of patches that are covered by opaque upper siblings

	WgStackHook * pCover = ((WgStackHook*)pHook)->Next();
	while( pCover )
	{
		WgRect geo = pCover->_getGeo(m_size);
		if( pCover->IsVisible() && geo.isOverlapping( rect ) )
			pCover->Widget()->_onMaskPatches( patches, geo, WgRect(0,0,65536,65536 ), _getBlendMode() );

		pCover = pCover->Next();
	}

	// Make request render calls

	for( const WgRect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
		_requestRender( * pRect );
}

//____ _onWidgetAppeared() _____________________________________________________

void WgStackPanel::_onWidgetAppeared( WgVectorHook * pInserted )
{
	bool	bRequestResize = false;

	// Check if we need to resize to fit Widget in current width

	int height = pInserted->Widget()->MatchingPixelHeight(m_size.w);
	if( height > m_size.h )
		bRequestResize = true;

	// Update bestSize

	WgSize preferred = pInserted->Widget()->PreferredPixelSize();

	if (preferred.w > m_preferredSize.w || preferred.h > m_preferredSize.h)
	{
		m_preferredSize = preferred;
		bRequestResize = true;
	}

	if( bRequestResize )
		_requestResize();

	// Adapt inserted Widget to our size

	pInserted->Widget()->_onNewSize(m_size);

	// Force a render.

	_onRenderRequested( pInserted );
}

//____ _onWidgetsAppeared() _____________________________________________________

void WgStackPanel::_onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
	bool    bRequestResize = false;

	WgVectorHook * p = pFirst;

	do {
		// Check if we need to resize to fit Widget in current width

		int height = p->Widget()->MatchingPixelHeight(m_size.w);
		if( height > m_size.h )
			bRequestResize = true;

		// Update bestSize

		WgSize preferred = p->Widget()->PreferredPixelSize();

		if (preferred.w > m_preferredSize.w || preferred.h > m_preferredSize.h)
		{
			m_preferredSize = preferred;
			bRequestResize = true;
		}


		p = static_cast<WgVectorHook*>(p->Next());

	} while (p != pLast );


	if( bRequestResize )
		_requestResize();

	// Adapt inserted Widgets to our size and force render

	p = pFirst;
	do {
		p->Widget()->_onNewSize(m_size);
		_onRenderRequested( p );
		p->Next();
	} while (p != pLast);

}


//____ _onWidgetDisappeared() __________________________________________________

void WgStackPanel::_onWidgetDisappeared( WgVectorHook * pToBeRemoved )
{
	bool	bRequestResize = false;

	// Get dirty rectangles for all visible sections of pToBeRemoved.
	_requestRender(static_cast<WgStackHook*>(pToBeRemoved)->_getGeo(WgRect(0, 0, m_size)));

//	_onRenderRequested( pToBeRemoved );

	// Update m_preferredSize, skiping pToBeRemoved

	WgSize	preferredSize;
	WgStackHook * pHook = FirstHook();
	while( pHook )
	{
		if( pHook != pToBeRemoved && pHook->m_bVisible )
		{
			WgSize sz = pHook->Widget()->PreferredPixelSize();
			if( sz.w > preferredSize.w )
				preferredSize.w = sz.w;
			if( sz.h > preferredSize.h )
				preferredSize.h = sz.h;
		}
		pHook = pHook->Next();
	}

	if (preferredSize != m_preferredSize)
	{
		m_preferredSize = preferredSize;
		bRequestResize = true;
	}

	// Check if removal might affect height for current width

	int height = pToBeRemoved->Widget()->MatchingPixelHeight(m_size.w);
	if( height >= m_size.h )
		bRequestResize = true;

	//

	if( bRequestResize )
		_requestResize();
}


//____ _onWidgetsDisappeared() _________________________________________________

void WgStackPanel::_onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast )
{
    // Update m_preferredSize, skipping disappearing ones

    WgSize    preferredSize;
    WgStackHook * pHook = FirstHook();
    while( pHook )
    {
        if( pHook == pFirst )
        {
            pHook = (WgStackHook *) pLast->Next();
            if( pHook == nullptr )
                break;
        }
        
        if( pHook->m_bVisible )
        {
            WgSize sz = pHook->Widget()->PreferredPixelSize();
            if( sz.w > preferredSize.w )
                preferredSize.w = sz.w;
            if( sz.h > preferredSize.h )
                preferredSize.h = sz.h;
        }
        pHook = pHook->Next();
    }

    if (preferredSize != m_preferredSize)
        m_preferredSize = preferredSize;

    _requestRender();
    _requestResize();
}


//____ _onWidgetsReordered() ___________________________________________________

void WgStackPanel::_onWidgetsReordered()
{
	_requestRender();
}

//____ _refreshAllWidgets() ____________________________________________________

void WgStackPanel::_refreshAllWidgets()
{
	_refreshPreferredPixelSize();
	_adaptChildrenToSize();
	_requestRender();
}

//____ _newHook() _____________________________________________________________

WgVectorHook * WgStackPanel::_newHook()
{
	return new WgStackHook(this);
}

//____ _refreshPreferredPixelSize() _____________________________________________________

void WgStackPanel::_refreshPreferredPixelSize()
{
	WgSize	preferredSize;

	WgStackHook * pHook = FirstHook();
	while( pHook )
	{
		if( pHook->m_bVisible )
		{
			WgSize sz = pHook->_paddedPreferredPixelSize(m_scale);
			if( sz.w > preferredSize.w )
				preferredSize.w = sz.w;
			if( sz.h > preferredSize.h )
				preferredSize.h = sz.h;
		}
		pHook = pHook->Next();
	}

	if( m_preferredSize != preferredSize)
	{
		m_preferredSize = preferredSize;
		_requestResize();
	}
}

//____ _adaptChildrenToSize() ___________________________________________________________

void WgStackPanel::_adaptChildrenToSize()
{
	WgStackHook * pHook = FirstHook();
	while( pHook )
	{
		pHook->Widget()->_onNewSize( pHook->_getGeo(m_size) );
		pHook = pHook->Next();
	}
}

//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgStackPanel::_firstHookWithGeo( WgRect& writeGeo ) const
{
	WgStackHook * p = FirstHook();
	if( p )
		writeGeo = p->_getGeo(m_size);

	return p;
}

//____ _nextHookWithGeo() ______________________________________________________

WgHook * WgStackPanel::_nextHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const
{
	WgStackHook * p = ((WgStackHook*)pHook)->Next();
	if( p )
		writeGeo = p->_getGeo(m_size);

	return p;
}

//____ _lastHookWithGeo() ______________________________________________________

WgHook * WgStackPanel::_lastHookWithGeo( WgRect& writeGeo ) const
{
	WgStackHook * p = LastHook();
	if( p )
		writeGeo = p->_getGeo(m_size);

	return p;
}

//____ _prevHookWithGeo() ______________________________________________________

WgHook * WgStackPanel::_prevHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const
{
	WgStackHook * p = ((WgStackHook*)pHook)->Prev();
	if( p )
		writeGeo = p->_getGeo(m_size);

	return p;
}

