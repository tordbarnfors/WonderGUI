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
#include <wg2_modallayer.h>
#include <wg2_util.h>
#include <wg2_patches.h>
#include <wg2_eventhandler.h>

static const char	c_widgetType[] = {"ModalLayer"};
static const char	c_hookType[] = {"ModalHook"};

// Improve WgModalHook geometry handling, should be able to run on PreferredSize by default, answering to resize-requests.


//_____________________________________________________________________________
const char * WgModalHook::Type( void ) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgModalHook::ClassType()
{
	return c_hookType;
}

//_____________________________________________________________________________
void WgModalHook::Top()
{
	_moveLast();
	_requestRender();
	m_pParent->_updateKeyboardFocus();
}

//_____________________________________________________________________________
bool WgModalHook::SetGeo( const WgRect& geometry, WgOrigo origo )
{
	m_placementGeo 	= geometry;
	m_origo 		= origo;

	if( m_placementGeo.w <= 0 )
		m_placementGeo.w = 1;

	if( m_placementGeo.h <= 0 )
		m_placementGeo.h = 1;

	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetGeo( const WgCoord& ofs, WgOrigo origo )
{
	m_placementGeo.setPos(ofs);
	m_origo	= origo;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetOfs( const WgCoord& ofs )
{
	m_placementGeo.setPos(ofs);
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetOfsX( int x )
{
	m_placementGeo.x = x;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetOfsY( int y )
{
	m_placementGeo.y = y;
	return _refreshRealGeo();
}


//_____________________________________________________________________________
bool WgModalHook::SetSize( WgSize sz )
{
	if( sz.w < 0 || sz.h < 0 )
		return false;

	m_placementGeo.setSize( sz );
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetWidth( int width )
{
	if( width < 0 )
		return false;

	m_placementGeo.w = width;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::SetHeight( int height )
{
	if( height < 0 )
		return false;

	m_placementGeo.h = height;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::Move( const WgCoord& ofs )
{
	m_placementGeo += ofs;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::MoveX( int x )
{
	m_placementGeo.x += x;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
bool WgModalHook::MoveY( int y )
{
	m_placementGeo.y += y;
	return _refreshRealGeo();
}

//_____________________________________________________________________________
WgModalLayer* WgModalHook::Parent() const
{
	return m_pParent;
}

//_____________________________________________________________________________
WgModalHook::WgModalHook( WgModalLayer * pParent )
{
	m_pParent = pParent;
}

//_____________________________________________________________________________
bool WgModalHook::_refreshRealGeo()	// Return false if we couldn't get exactly the requested (floating) geometry.
{
	WgSize sz = m_placementGeo.size();

	if( sz.w == 0 && sz.h == 0 )
		sz = m_pWidget->PreferredPixelSize();
	else if( sz.w == 0 )
		sz.w = m_pWidget->MatchingPixelWidth(sz.h);
	else if( sz.h == 0 )
		sz.h = m_pWidget->MatchingPixelHeight(sz.w);

	if( sz.w <= 0 )
		sz.w = 1;
	if( sz.h <= 0 )
		sz.h = 1;

	WgCoord ofs = WgUtil::OrigoToOfs( m_origo, m_pParent->PixelSize() ) - WgUtil::OrigoToOfs( m_origo, sz );
	ofs += m_placementGeo.pos();

	WgRect newGeo( ofs, sz );

	if( newGeo != m_geo )
	{
		_requestRender();
		m_geo = WgRect( ofs, sz );
		_requestRender();
	}

	return true;
}

//_____________________________________________________________________________
void WgModalHook::_requestResize()
{
	_refreshRealGeo();
}

//_____________________________________________________________________________
WgHook * WgModalHook::_prevHook() const
{
	WgModalHook * p = _prev();

	if( p )
		return p;
	else if( m_pParent->m_baseHook.Widget() )
		return &m_pParent->m_baseHook;
	else
		return 0;
}

//_____________________________________________________________________________
WgHook * WgModalHook::_nextHook() const
{
	WgModalHook * p = _prev();

	// We have multiple inheritance, so lets make the cast in a safe way, preserving NULL-pointer as NULL.

	if( p )
		return p;
	else
		return 0;
}

//_____________________________________________________________________________
WgContainer * WgModalHook::_parent() const
{
	return m_pParent;
}

//____ Constructor ____________________________________________________________

WgModalLayer::WgModalLayer()
{
}

//____ Destructor _____________________________________________________________

WgModalLayer::~WgModalLayer()
{
	// Children are deleted automaticallly when their hooks are deteled.
}

//____ Type() _________________________________________________________________

const char *WgModalLayer::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgModalLayer::GetClass()
{
	return c_widgetType;
}

//____ AddModal() ________________________________________________________

WgModalHook * WgModalLayer::AddModal( WgWidget * pWidget, const WgRect& geometry, WgOrigo origo )
{
	// Create Hook and fill in members.

	WgModalHook * pHook = new WgModalHook( this );
	pHook->_attachWidget(pWidget);
	pHook->m_origo = origo;
	pHook->m_placementGeo = geometry;
	m_modalHooks.pushBack(pHook);

	// Refresh geometry and request render.

	pHook->_refreshRealGeo();
	_updateKeyboardFocus();
	return pHook;
}

//____ DeleteAllModal() _________________________________________________

bool WgModalLayer::DeleteAllModal()
{
	m_modalHooks.clear();
	_requestRender();
	_updateKeyboardFocus();
	return true;
}

//____ ReleaseAllModal() ________________________________________________

bool WgModalLayer::ReleaseAllModal()
{
	WgModalHook * pHook = m_modalHooks.first();
	while( pHook )
	{
		pHook->_releaseWidget();
		pHook = pHook->_next();
	}

	m_modalHooks.clear();
	_requestRender();
	_updateKeyboardFocus();
	return true;
}

//____ DeleteChild() __________________________________________________________

bool WgModalLayer::DeleteChild( WgWidget * pWidget )
{
	if( !pWidget || pWidget->Parent() != this )
		return false;

	if( pWidget == m_baseHook.Widget() )
		return DeleteBase();
	else
	{
		WgModalHook * pHook = (WgModalHook *) pWidget->Hook();
		pHook->_requestRender();
		delete pHook;
		_updateKeyboardFocus();
		return true;
	}
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgModalLayer::ReleaseChild( WgWidget * pWidget )
{
	if( !pWidget || pWidget->Parent() != this )
		return 0;

	if( pWidget == m_baseHook.Widget() )
		return ReleaseBase();
	else
	{
		WgModalHook * pHook = (WgModalHook *) pWidget->Hook();
		pHook->_requestRender();
		pHook->_releaseWidget();
		delete pHook;
		_updateKeyboardFocus();
		return pWidget;
	}

}

//____ DeleteAllChildren() ______________________________________________________

bool WgModalLayer::DeleteAllChildren()
{
	DeleteBase();
	DeleteAllModal();
	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgModalLayer::ReleaseAllChildren()
{
	ReleaseBase();
	ReleaseAllModal();
	return true;
}

//____ FirstModal() ______________________________________________________

WgModalHook * WgModalLayer::FirstModal()
{
	return m_modalHooks.first();
}

//____ LastModal() _______________________________________________________

WgModalHook * WgModalLayer::LastModal()
{
	return m_modalHooks.last();
}

//____ MatchingPixelHeight() _______________________________________________________

int WgModalLayer::MatchingPixelHeight( int width ) const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->MatchingPixelHeight( width );
	else
		return WgWidget::MatchingPixelHeight(width);
}

//____ MatchingPixelWidth() _______________________________________________________

int WgModalLayer::MatchingPixelWidth( int height ) const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->MatchingPixelWidth( height );
	else
		return WgWidget::MatchingPixelWidth(height);
}

//____ PreferredPixelSize() _____________________________________________________________

WgSize WgModalLayer::PreferredPixelSize() const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->PreferredPixelSize();
	else
		return WgSize(1,1);
}

//____ FindWidget() ____________________________________________________________

WgWidget *  WgModalLayer::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	// In search mode ACTION_TARGET we always return either the topmost non-hidden modal Widget (or its children),
	// or us.

	if( mode == WgSearchMode::ActionTarget )
	{
		WgModalHook * pHook = m_modalHooks.last();

		if( pHook )
		{
			if( pHook->Widget()->IsContainer() )
			{
				WgWidget * pResult = pHook->Widget()->CastToContainer()->FindWidget( ofs - pHook->PixelPos(), mode );
				if( pResult )
					return pResult;
			}
			else
			{
				if( pHook->Widget()->MarkTest(ofs - pHook->PixelPos()) )
					return pHook->Widget();
				else
					return this;
			}
		}
		else if( m_baseHook.Widget() )
		{
			if( m_baseHook.Widget()->IsContainer() )
			{
				WgWidget * pResult = m_baseHook.Widget()->CastToContainer()->FindWidget( ofs - m_baseHook.PixelPos(), mode );
				if( pResult )
					return pResult;
			}
			else
				return m_baseHook.Widget();
		}
		else
			return 0;
	}

	// For the rest of the modes we can rely on the default method.

	return WgContainer::FindWidget( ofs, mode );
}

//____ _updateKeyboardFocus() _______________________________________________________

void WgModalLayer::_updateKeyboardFocus()
{
	// Get event handler, verify that we have a root

	if( !Hook() )
		return;

	WgEventHandler * pHandler = Hook()->EventHandler();
	if( !pHandler )
		return;

	// Retrieve focused Widget and verify it being a descendant to us.

	WgWidget * pFocused = pHandler->KeyboardFocus();

	WgWidget * p = pFocused;
	while( p && p->Parent() && p->Parent() != this )
		p = p->Parent()->CastToWidget();

	if( p && p->Parent() != this )
		return;								// Focus belongs to a Widget that is not a descendant to us,
											// so we can't save and shouldn't steal focus.

	// Save old focus so we can return it properly in the future.
	if( p )
	{
		if( p == m_baseHook.Widget() )
			m_pBaseKeyFocus = pFocused;
		else
		{
			WgModalHook * pHook = static_cast<WgModalHook*>(p->Hook());
			pHook->m_pKeyFocus = pFocused;
		}
	}

	// Find which child-branch to focus and switch to our previously saved focus

	WgModalHook * pHook = m_modalHooks.last();

	WgWidget * 	pSavedFocus = 0;
	WgHook *	pBranch	= 0;

	if( pHook )
	{
		pSavedFocus = pHook->m_pKeyFocus.GetRealPtr();
		pHook->m_pKeyFocus = 0;								// Needs to be cleared for the future.
		pBranch = pHook;
	}
	else if( m_baseHook.Widget() )
	{
		pSavedFocus = m_pBaseKeyFocus.GetRealPtr();
		m_pBaseKeyFocus = 0;								// Needs to be cleared for the future.
		pBranch = &m_baseHook;
	}

	// Verify that saved focus still is within branch and is not hidden

	if( pSavedFocus )
	{
		WgHook * p = pSavedFocus->Hook();
		while( p && p != pBranch )
		{
			WgWidgetHolder * pParent = p->Parent();
			if( pParent && pParent->IsWidget() )
				p = pParent->CastToWidget()->Hook();
			else
				p = 0;
		}

		if( p != pBranch )
			pSavedFocus = 0;				// Previously focused Widget is no longer a child of focused branch.
	}

	// Switch to previously saved focus, or null if not applicable

	pHandler->SetKeyboardFocus( pSavedFocus );
}

//____ _onNewSize() ___________________________________________________________

void WgModalLayer::_onNewSize( const WgSize& sz )
{
	WgContainer::_onNewSize(sz);

	// Refresh modal widgets geometry, their positions might have changed.

	WgModalHook * pHook = m_modalHooks.first();

	while( pHook )
	{
		pHook->_refreshRealGeo();
		pHook = pHook->_next();
	}
}

//____ _onCloneContent() ______________________________________________________

void WgModalLayer::_onCloneContent( const WgWidget * _pOrg )
{
}

//____ _onEvent() ______________________________________________________________

void WgModalLayer::_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler )
{

	if( !m_modalHooks.isEmpty() && FindWidget( _pEvent->PointerPixelPos(), WgSearchMode::ActionTarget ) == this )
	{
		switch( _pEvent->Type() )
		{
			case WG_EVENT_MOUSEBUTTON_PRESS:
			{
				const WgEvent::MouseButtonEvent * pEvent = static_cast<const WgEvent::MouseButtonEvent*>(_pEvent);
				pHandler->QueueEvent( new WgEvent::ModalBlockedPress( pEvent->Button(), this) );
			}
			break;

			case WG_EVENT_MOUSEBUTTON_RELEASE:
			{
				const WgEvent::MouseButtonEvent * pEvent = static_cast<const WgEvent::MouseButtonEvent*>(_pEvent);
				pHandler->QueueEvent( new WgEvent::ModalBlockedPress( pEvent->Button(), this) );
			}
			break;

			case WG_EVENT_MOUSE_MOVE:
			{
				pHandler->QueueEvent( new WgEvent::ModalMoveOutside(this) );
			}
			break;

			default:
				break;
		}
	}

	// Forward all events

	WgLayer::_onEvent(_pEvent,pHandler);
}

//____ _firstHook() ___________________________________________________________

WgHook* WgModalLayer::_firstHook() const
{
	if( m_baseHook.Widget() )
		return const_cast<BaseHook*>(&m_baseHook);
	else
		return m_modalHooks.first();
}

//____ _lastHook() ____________________________________________________________

WgHook* WgModalLayer::_lastHook() const
{
	return m_modalHooks.last();
}

//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgModalLayer::_firstHookWithGeo( WgRect& geo ) const
{
	if( m_baseHook.Widget() )
	{
		geo = WgRect(0,0,m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
	{
		WgModalHook * p = m_modalHooks.first();
		if( p )
			geo = p->m_geo;

		return p;
	}
}

//____ _nextHookWithGeo() _______________________________________________________

WgHook * WgModalLayer::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgHook * p = pHook->Next();
	if( p )
		geo = ((WgModalHook*)p)->m_geo;

	return p;
}

//____ _lastHookWithGeo() _____________________________________________________

WgHook * WgModalLayer::_lastHookWithGeo( WgRect& geo ) const
{
	WgModalHook * p = m_modalHooks.last();
	if( p )
	{
		geo = p->m_geo;
		return p;
	}
	else if( m_baseHook.Widget() )
	{
		geo = WgRect(0,0,m_size);
		return const_cast<BaseHook*>(&m_baseHook);
	}
	else
		return 0;
}

//____ _prevHookWithGeo() _______________________________________________________

WgHook * WgModalLayer::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	WgHook * p = pHook->Prev();
	if( p )
		geo = p->PixelGeo();

	return p;
}



