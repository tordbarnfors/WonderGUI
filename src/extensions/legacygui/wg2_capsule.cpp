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
#include <wg2_capsule.h>
#include <wg2_patches.h>

static const char	c_hookType[] = {"Capsule"};


//_____________________________________________________________________________
const char * WgCapsule::CapsuleHook::Type( void ) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgCapsule::CapsuleHook::ClassType()
{
	return c_hookType;
}

//_____________________________________________________________________________
WgCoord WgCapsule::CapsuleHook::PixelPos() const
{
	return m_pParent->_childPos();
}

//_____________________________________________________________________________
WgSize WgCapsule::CapsuleHook::PixelSize() const
{
	return m_pParent->_childSize();
}

//_____________________________________________________________________________
WgRect WgCapsule::CapsuleHook::PixelGeo() const
{
	return m_pParent->_childGeo();
}

//_____________________________________________________________________________
WgCoord WgCapsule::CapsuleHook::ScreenPixelPos() const
{
	return m_pParent->_childPos() + m_pParent->ScreenPixelPos();
}

//_____________________________________________________________________________
WgRect WgCapsule::CapsuleHook::ScreenPixelGeo() const
{
	return m_pParent->_childGeo() + m_pParent->ScreenPixelPos();
}

//____ Constructor ____________________________________________________________

WgCapsule::WgCapsule()
{
	m_hook.m_pParent = this;
	m_bSiblingsOverlap = false;
}



//____ IsCapsule() ______________________________________________________________

bool WgCapsule::IsCapsule() const
{
	return true;
}

//____ CastToCapsule() _______________________________________________________

WgCapsule * WgCapsule::CastToCapsule()
{
	return this;
}

const WgCapsule * WgCapsule::CastToCapsule() const
{
	return this;
}

//____ SetChild() ______________________________________________________________

WgHook * WgCapsule::SetChild( WgWidget * pWidget )
{
	if( !pWidget )
		  return NULL;

	WgSize sz = PixelSize();
	if( m_pSkin )
		sz -= _skinContentPadding( m_pSkin, m_scale);

	m_hook._attachWidget(pWidget);
	pWidget->_onNewSize(sz);

	_childAddedRemovedOrReplaced();
	return &m_hook;
}

//____ DeleteChild() ___________________________________________________________

bool WgCapsule::DeleteChild()
{
	WgWidget * pWidget = m_hook._releaseWidget();
	if( pWidget )
	{
		delete pWidget;
		_childAddedRemovedOrReplaced();
		return true;
	}

	return false;
}

//____ ReleaseChild() __________________________________________________________

WgWidget * WgCapsule::ReleaseChild()
{
	WgWidget * pWidget = m_hook._releaseWidget();
	if( pWidget )
		_childAddedRemovedOrReplaced();

	return pWidget;
}

//____ DeleteChild() ___________________________________________________________

bool WgCapsule::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return DeleteChild();

	return false;
}

//____ ReleaseChild() __________________________________________________________

WgWidget * WgCapsule::ReleaseChild( WgWidget * pWidget )
{
	if(pWidget == m_hook.Widget() )
		return ReleaseChild();

	return 0;
}

//____ DeleteAllChildren() _____________________________________________________

bool WgCapsule::DeleteAllChildren()
{
	return DeleteChild();
}

//____ ReleaseAllChildren() ____________________________________________________

bool WgCapsule::ReleaseAllChildren()
{
	ReleaseChild();
	return true;
}

//____ MatchingPixelHeight() ________________________________________________________

int WgCapsule::MatchingPixelHeight( int width ) const
{
	WgSize padding;
	if( m_pSkin )
		padding = _skinContentPadding( m_pSkin, m_scale);

	if( m_hook.Widget() )
		return m_hook.Widget()->MatchingPixelHeight( width - padding.w ) + padding.h;
	else
		return WgWidget::MatchingPixelHeight(width);
}

//____ MatchingPixelWidth() ________________________________________________________

int WgCapsule::MatchingPixelWidth( int height ) const
{
	WgSize padding;
	if( m_pSkin )
		padding = _skinContentPadding( m_pSkin, m_scale);

	if( m_hook.Widget() )
		return m_hook.Widget()->MatchingPixelWidth( height - padding.h ) + padding.w;
	else
		return WgWidget::MatchingPixelWidth(height);
}

//____ PreferredPixelSize() ___________________________________________________________

WgSize WgCapsule::PreferredPixelSize() const
{
	WgSize sz;

	if( m_hook.Widget() )
		sz = m_hook.Widget()->PreferredPixelSize();
	else
		sz = WgSize(1,1);

	if( m_pSkin )
		sz += _skinContentPadding( m_pSkin, m_scale);

	return sz;
}

//____ _childAddedRemovedOrReplaced() _________________________________________

void WgCapsule::_childAddedRemovedOrReplaced()
{
	_requestRender();
	_requestResize();
}

//____ _childPos() _____________________________________________________________

WgCoord WgCapsule::_childPos()
{
	if( m_pSkin )
		return _skinContentOfs( m_pSkin, m_state, m_scale);
	else
		return WgCoord();
}

//____ _childSize() ____________________________________________________________

WgSize WgCapsule::_childSize()
{
	if( m_pSkin )
		return PixelSize() - _skinContentPadding( m_pSkin, m_scale);
	else
		return PixelSize();
}

//____ _childGeo() _____________________________________________________________

WgRect WgCapsule::_childGeo()
{
	if( m_pSkin )
		return _skinContentRect( m_pSkin, { 0,0, PixelSize() }, m_state, m_scale );
	else
		return { 0,0, PixelSize() };
}

//____ _onCollectPatches() _____________________________________________________

void WgCapsule::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	if( m_pSkin )
		container.add( WgRect::overlap( geo, clip ) );
	else if( m_hook.Widget() )
		m_hook.Widget()->_onCollectPatches( container, geo, clip );
}

//____ _onMaskPatches() ________________________________________________________

void WgCapsule::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	if( m_pSkin && m_pSkin->isOpaque(m_state) )
		patches.sub( WgRect::overlap( geo, clip ) );
	else if( m_hook.Widget() )
	{
		WgRect childGeo;
		if( m_pSkin )
			childGeo = _skinContentRect( m_pSkin, geo,m_state,m_scale);
		else
			childGeo = geo;

		m_hook.Widget()->_onMaskPatches( patches, childGeo, clip, blendMode );
	}
}

//____ _onCloneContent() _______________________________________________________

void WgCapsule::_onCloneContent( const WgWidget * _pOrg )
{
}

//____ _onNewSize() ____________________________________________________________

void WgCapsule::_onNewSize( const WgSize& size )
{
	if( m_hook.Widget() )
	{
		WgSize sz = size;
		if( m_pSkin )
			sz -= _skinContentPadding( m_pSkin, m_scale);

		m_hook.Widget()->_onNewSize(sz);
	}
}

//____ _onRenderRequested() ___________________________________________________

void WgCapsule::_onRenderRequested()
{
	if( m_pSkin )
		_requestRender( _skinContentRect( m_pSkin, PixelSize(), m_state, m_scale) );
	else
		_requestRender();
}

void WgCapsule::_onRenderRequested(const WgRect& rect)
{
	if( m_pSkin )
		_requestRender( rect + _skinContentOfs( m_pSkin, m_state,m_scale) );
	else
		_requestRender(rect);
}

//____ _onResizeRequested() ___________________________________________________

void WgCapsule::_onResizeRequested()
{
	_requestResize();
}


//____ _firstHook() ____________________________________________________________

WgHook* WgCapsule::_firstHook() const
{
	if( m_hook.Widget() )
		return const_cast<CapsuleHook*>(&m_hook);
	else
		return 0;
}

//____ _lastHook() _____________________________________________________________

WgHook* WgCapsule::_lastHook() const
{
	if( m_hook.Widget() )
		return const_cast<CapsuleHook*>(&m_hook);
	else
		return 0;
}

//____ _firstHookWithGeo() _____________________________________________________

WgHook * WgCapsule::_firstHookWithGeo( WgRect& geo ) const
{
	if( m_hook.Widget() )
	{
		if( m_pSkin )
			geo = _skinContentRect( m_pSkin, PixelSize(), m_state, m_scale );
		else
			geo = WgRect(0,0,PixelSize());

		return const_cast<CapsuleHook*>(&m_hook);
	}

	return 0;
}

//____ _nextHookWithGeo() ______________________________________________________

WgHook * WgCapsule::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	return 0;
}

//____ _lastHookWithGeo() ______________________________________________________

WgHook * WgCapsule::_lastHookWithGeo( WgRect& geo ) const
{
	if( m_hook.Widget() )
	{
		if( m_pSkin )
			geo = _skinContentRect( m_pSkin, PixelSize(), m_state, m_scale );
		else
			geo = WgRect(0,0,PixelSize());

		return const_cast<CapsuleHook*>(&m_hook);
	}

	return 0;
}

//_____ _prevHookWithGeo() _____________________________________________________

WgHook * WgCapsule::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	return 0;
}
