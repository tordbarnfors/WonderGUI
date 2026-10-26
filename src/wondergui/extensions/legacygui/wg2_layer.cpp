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
#include <wg2_layer.h>
#include <wg2_patches.h>

static const char    c_basehookType[] = {"LayerBasehook"};


//____ Constructor ____________________________________________________________

WgLayer::WgLayer()
{
	m_baseHook.m_pParent = this;
}

//____ IsLayer() ______________________________________________________________

bool WgLayer::IsLayer() const
{
	return true;
}

//____ CastToLayer() _______________________________________________________

WgLayer * WgLayer::CastToLayer()
{
	return this;
}

const WgLayer * WgLayer::CastToLayer() const
{
	return this;
}

//____ SetBase() _________________________________________________________

WgHook * WgLayer::SetBase( WgWidget * pWidget )
{
	// Replace Widget

	WgWidget * pOldWidget = m_baseHook._releaseWidget();
	if( pOldWidget )
		delete pOldWidget;
	m_baseHook._attachWidget(pWidget);
	_onBaseChanged();
	return &m_baseHook;
}

//____ Base() ____________________________________________________________

WgWidget * WgLayer::Base()
{
	return m_baseHook.Widget();
}

//____ DeleteBase() ______________________________________________________

bool WgLayer::DeleteBase()
{
	WgWidget * pWidget = m_baseHook._releaseWidget();
	if( pWidget )
	{
		_onBaseChanged();
		delete pWidget;
		return true;
	}

	return false;
}

//____ ReleaseBase() _____________________________________________________

WgWidget * WgLayer::ReleaseBase()
{
	WgWidget * pWidget = m_baseHook._releaseWidget();
	if( pWidget )
		_onBaseChanged();

	return pWidget;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgLayer::MatchingPixelHeight( int width ) const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->MatchingPixelHeight( width );
	else
		return WgWidget::MatchingPixelHeight(width);
}

//____ MatchingPixelWidth() _______________________________________________________

int WgLayer::MatchingPixelWidth( int height ) const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->MatchingPixelWidth( height );
	else
		return WgWidget::MatchingPixelWidth(height);
}

//____ PreferredPixelSize() _____________________________________________________________

WgSize WgLayer::PreferredPixelSize() const
{
	if( m_baseHook.Widget() )
		return m_baseHook.Widget()->PreferredPixelSize();
	else
		return WgSize(1,1);
}


//____ _onRequestRender() _____________________________________________________

void WgLayer::_onRequestRender( const WgRect& rect, const WgLayerHook * pHook )
{
	// Clip our geometry and put it in a dirtyrect-list

	WgPatches patches;
	patches.add( WgRect::overlap( rect, WgRect(0,0,m_size)) );

	// Remove portions of dirty rect that are covered by opaque upper siblings,
	// possibly filling list with many small dirty rects instead.

	WgLayerHook * pCover;

	if( pHook )
		pCover = pHook->Next();
	else
		pCover = _firstLayerHook();

	while( pCover )
	{
		if( pCover->m_geo.isOverlapping( rect ) )
			pCover->Widget()->_onMaskPatches( patches, pCover->m_geo, WgRect(0,0,65536,65536 ), _getBlendMode() );

		pCover = pCover->Next();
	}

	// Make request render calls

	for( const WgRect * pRect = patches.begin() ; pRect < patches.end() ; pRect++ )
		_requestRender( * pRect );
}


//____ _onNewSize() ___________________________________________________________

void WgLayer::_onNewSize(const WgSize& sz)
{
	m_size = sz;

	if (m_baseHook.Widget())
		m_baseHook.Widget()->_onNewSize(sz);

	WgContainer::_onNewSize(sz);
}

//____ _onBaseChanged() _______________________________________________________

void WgLayer::_onBaseChanged()
{
	_onRequestRender( WgRect(0,0,m_size), 0 );
	_requestResize();

	if (m_baseHook.Widget())
		m_baseHook.Widget()->_onNewSize(m_size);
}

//_____________________________________________________________________________
void WgLayer::BaseHook::_requestRender()
{
	m_pParent->_onRequestRender( WgRect( 0,0, m_pParent->m_size ), 0 );
}

//_____________________________________________________________________________
void WgLayer::BaseHook::_requestRender( const WgRect& rect )
{
	m_pParent->_onRequestRender( rect, 0 );
}

//_____________________________________________________________________________
void WgLayer::BaseHook::_requestResize()
{
	m_pParent->_requestResize();					// Just forward to our parent
}


//_____________________________________________________________________________
WgCoord WgLayerHook::ScreenPixelPos() const
{
	return Parent()->ScreenPixelPos() + m_geo.pos();
}

//_____________________________________________________________________________
WgRect WgLayerHook::ScreenPixelGeo() const
{
	return m_geo + Parent()->ScreenPixelPos();
}

//_____________________________________________________________________________
void WgLayerHook::_requestRender()
{
	Parent()->_onRequestRender( m_geo, this );
}

//_____________________________________________________________________________
void WgLayerHook::_requestRender( const WgRect& rect )
{
	Parent()->_onRequestRender( rect + m_geo.pos(), this );
}

//_____________________________________________________________________________
WgLayer* WgLayerHook::Parent() const
{
	return static_cast<WgLayer*>(_parent());
}

//_____________________________________________________________________________
const char * WgLayer::BaseHook::Type( void ) const
{
	return ClassType();
}

//_____________________________________________________________________________
const char * WgLayer::BaseHook::ClassType()
{
	return c_basehookType;
}
