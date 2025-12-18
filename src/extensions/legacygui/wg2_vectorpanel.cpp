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
#include <wg2_vectorpanel.h>


WgCoord WgVectorHook::PixelPos() const
{
	return Parent()->_hookGeo(this).pos();
}

WgSize WgVectorHook::PixelSize() const
{
	return Parent()->_hookGeo(this).size();
}

WgRect WgVectorHook::PixelGeo() const
{
	return Parent()->_hookGeo(this);
}

WgCoord WgVectorHook::ScreenPixelPos() const
{
	return Parent()->_hookGeo(this).pos() + Parent()->ScreenPixelPos();

}

WgRect WgVectorHook::ScreenPixelGeo() const
{
	return Parent()->_hookGeo(this) + Parent()->ScreenPixelPos();
}

bool WgVectorHook::MoveForward()
{
	if( _moveUp() )
	{
		Parent()->_onWidgetsReordered();
		return true;
	}

	return false;
}

bool WgVectorHook::MoveBackward()
{
	if( _moveDown() )
	{
		Parent()->_onWidgetsReordered();
		return true;
	}

	return false;
}

bool WgVectorHook::MoveBefore( WgVectorHook * pSibling )
{
	if( pSibling && pSibling->Parent() == Parent() )
	{
		if( _moveBefore(pSibling) )
		{
			Parent()->_onWidgetsReordered();
			return true;
		}
	}
	return false;
}

bool WgVectorHook::MoveAfter( WgVectorHook * pSibling )
{
	if( pSibling && pSibling->Parent() == Parent() )
	{
		if( _moveAfter(pSibling) )
		{
			Parent()->_onWidgetsReordered();
			return true;
		}
	}
	return false;
}

bool WgVectorHook::MoveFirst()
{
	if( _moveFirst() )
	{
		Parent()->_onWidgetsReordered();
		return true;
	}

	return false;
}

bool WgVectorHook::MoveLast()
{
	if( _moveLast() )
	{
		Parent()->_onWidgetsReordered();
		return true;
	}

	return false;
}

bool WgVectorHook::SetVisible( bool bVisible )
{
	if( bVisible != m_bVisible )
	{
		m_bVisible = bVisible;
		if( bVisible )
			Parent()->_onWidgetAppeared(this);
		else
			Parent()->_onWidgetDisappeared(this);
	}

	return true;
}

WgVectorPanel * WgVectorHook::Parent() const
{
	return static_cast<WgVectorPanel*>(_parent());
}


WgVectorHook::WgVectorHook()
{
}

WgVectorHook::~WgVectorHook()
{
}


void WgVectorHook::_requestRender()
{
	WgVectorPanel * p = Parent();

	p->_onRenderRequested(this);
}

void WgVectorHook::_requestRender( const WgRect& rect )
{
	Parent()->_onRenderRequested(this, rect);
}

void WgVectorHook::_requestResize()
{
	Parent()->_onResizeRequested(this);
}


WgHook * WgVectorHook::_prevHook() const
{
	return _prev();
}

WgHook * WgVectorHook::_nextHook() const
{
	return _next();
}

//____ Constructor() __________________________________________________________

WgVectorPanel::WgVectorPanel() : m_sortOrder(WgSortOrder::Ascending), m_pSortFunc(0), m_nChildPanels(0)
{

}


//____ Destructor() ___________________________________________________________

WgVectorPanel::~WgVectorPanel()
{
}


//____ AddChild() _____________________________________________________________

WgVectorHook * WgVectorPanel::AddChild( WgWidget * pWidget )
{
	if( !pWidget )
		return 0;

	WgVectorHook * pHook = _newHook();
	m_hooks.pushBack(pHook);
	pHook->_attachWidget( pWidget );

	_onWidgetAppeared(pHook);
	return pHook;
}

//____ AddChildren() _____________________________________________________________

WgVectorHook * WgVectorPanel::AddChildren( WgWidget * pWidgets[], int nb )
{
	WgVectorHook * pFirst = nullptr;
	WgVectorHook * pLast = nullptr;

	if( nb == 0 )
		return nullptr;

	for( int i = 0 ; i < nb ; i++ )
	{
		WgVectorHook * pHook = _newHook();
		m_hooks.pushBack(pHook);
		pHook->_attachWidget( pWidgets[i] );

		if( !pFirst )
			pFirst = pHook;
		pLast = pHook;
	}

	_onWidgetsAppeared(pFirst, pLast);
	return pFirst;
}


//____ InsertChild() __________________________________________________________

WgVectorHook * WgVectorPanel::InsertChild( WgWidget * pWidget, WgWidget * pSibling )
{
	if( !pWidget || !pSibling || !pSibling->Parent() || pSibling->Parent() != this )
		return 0;

	WgVectorHook * pHook = _newHook();
	pHook->_moveBefore(static_cast<WgVectorHook*>(pSibling->Hook()));

	pHook->_attachWidget( pWidget );

	_onWidgetAppeared(pHook);
	return pHook;
}

//____ InsertChildSorted() ____________________________________________________

WgVectorHook * WgVectorPanel::InsertChildSorted( WgWidget * pWidget )
{
	//TODO: Implement

	return 0;
}

//____ DeleteChild() __________________________________________________________

bool WgVectorPanel::DeleteChild( WgWidget * pWidget )
{
	if( !pWidget || !pWidget->Hook() || pWidget->Hook()->Parent() != this )
		return false;

	// Disconnect and notify subclass that widget has disappeared

	WgVectorHook * pHook = (WgVectorHook *) pWidget->Hook();
	pHook->_disconnect();

	if( pHook->IsVisible() )
		_onWidgetDisappeared( pHook );

	// Delete the hook & widget and return

	delete pHook;
	return true;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgVectorPanel::ReleaseChild( WgWidget * pWidget )
{
	if( !pWidget || !pWidget->Hook() || pWidget->Hook()->Parent() != this )
		return 0;

	// Disconnect and notify subclass that widget has disappeared

	WgVectorHook * pHook = (WgVectorHook *) pWidget->Hook();
	pHook->_disconnect();

	if( pHook->IsVisible() )
		_onWidgetDisappeared( pHook );

	pHook->_releaseWidget();
	delete pHook;
	return pWidget;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgVectorPanel::DeleteAllChildren()
{
	m_hooks.clear();
	_refreshAllWidgets();

	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgVectorPanel::ReleaseAllChildren()
{
	WgVectorHook * pHook = m_hooks.first();
	while( pHook )
	{
		pHook->_releaseWidget();
		pHook = pHook->Next();
	}

	m_hooks.clear();
	_refreshAllWidgets();

	return true;
}

//____ SortChildren() ___________________________________________________________

void WgVectorPanel::SortChildren()
{
	//TODO: Implement
}

//____ SetSortOrder() _________________________________________________________

void WgVectorPanel::SetSortOrder( WgSortOrder order )
{
	m_sortOrder = order;
}

//____ SetSortFunction() ______________________________________________________

void WgVectorPanel::SetSortFunction( WgWidgetSortFunc pSortFunc )
{
	m_pSortFunc = pSortFunc;
}

//____ HideChildren() _________________________________________________________

void WgVectorPanel::HideChildren( int index, int amount )
{
    if( index >= m_hooks.size() || index < 0 || amount <= 0 )
        return;
    
    WgVectorHook * pHook = m_hooks.get(index);
    if( pHook == nullptr )
        return;

    WgVectorHook * pFirst = pHook;
    WgVectorHook * pLast = pHook;
    
    for( int i = 0 ; i < amount ; i++ )
    {
        pHook->m_bVisible = false;
      
        pHook = pHook->Next();
        if( pHook == nullptr )
            break;
    
        pLast = pHook;
    }

    //TODO: Shouldn't really mark any widgets as disappeared that already were hidden.
    // But it will work for now...

    _onWidgetsDisappeared( pFirst, pLast );
}

//____ UnhideChildren() _______________________________________________________

void WgVectorPanel::UnhideChildren( int index, int amount )
{
    if( index >= m_hooks.size() || index < 0 || amount <= 0 )
        return;
    
    WgVectorHook * pHook = m_hooks.get(index);
    if( pHook == nullptr )
        return;

    WgVectorHook * pFirst = pHook;
    WgVectorHook * pLast = pHook;
    
    for( int i = 0 ; i < amount ; i++ )
    {
        pHook->m_bVisible = true;

        pHook = pHook->Next();
        if( pHook == nullptr )
            break;
    
        pLast = pHook;
    }

    //TODO: Shouldn't really mark any widgets as appeared that already were visible.
    // But it will work for now...

    _onWidgetsAppeared( pFirst, pLast );
}



//____ _onCloneContent() ______________________________________________________

void WgVectorPanel::_onCloneContent( const WgWidget * _pOrg )
{
	//TODO: Implement
}
