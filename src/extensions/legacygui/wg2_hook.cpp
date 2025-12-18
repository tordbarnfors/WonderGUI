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
#include <wg2_hook.h>
#include <wg2_panel.h>
#include <wg2_widget.h>
#include <wg2_rootpanel.h>
#include <assert.h>


//____ Destructor _____________________________________________________________

WgHook::~WgHook()
{
	if( m_pWidget )
	{
		m_pWidget->m_pHook = 0;
		delete m_pWidget;
	}
}

//____ _attachWidget() __________________________________________________________

void WgHook::_attachWidget( WgWidget * pWidget )
{
	assert( pWidget != nullptr );
	assert( pWidget->Parent() == nullptr );

	if( m_pWidget )
		m_pWidget->_onNewHook(0);

	// Update scale while we don't have any parent and won't cause a flood of _requestResize().

	int scale;
	WgContainer * p =  _parent();
	if( p )
		scale = p->m_scale;
	else
		scale = Root()->Scale();

	if( scale != pWidget->m_scale )
		pWidget->_setScale( scale );

	//

	m_pWidget = pWidget;
	if( pWidget )
		pWidget->_onNewHook(this);
}

//____ _relinkWidget() __________________________________________________________

void WgHook::_relinkWidget()
{
	if( m_pWidget )
		m_pWidget->m_pHook = this;
}

//____ _releaseWidget() _________________________________________________________

WgWidget* WgHook::_releaseWidget()
{
	WgWidget * p = m_pWidget;
	m_pWidget = 0;

	if( p )
		p->m_pHook = 0;

	return p;
}

//____ _requestFocus() _________________________________________________________

bool WgHook::_requestFocus()
{
	return Parent()->_focusRequested(this, m_pWidget);
}

//____ _releaseFocus() _________________________________________________________

bool WgHook::_releaseFocus()
{
	return Parent()->_focusReleased(this, m_pWidget);
}

//____ _requestInView() ________________________________________________________

void WgHook::_requestInView()
{
	Parent()->_inViewRequested(this);
}

void WgHook::_requestInView( const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	Parent()->_inViewRequested( this, mustHaveArea, niceToHaveArea );
}

//____ _isDisplayed() _________________________________________________________

bool WgHook::_isDisplayed() const
{
	WgWidgetHolder * pHolder = _holder();

	if( pHolder->IsWidget() )
	{
		WgHook * pHook = pHolder->CastToWidget()->Hook();
		if( pHook )
			return pHook->_isDisplayed();
	}
	else if( pHolder->IsRoot() )
		return true;

	return false;
}


//____ PointPos() ______________________________________________________________

WgCoord WgHook::PointPos() const
{
    int scale = Parent() ? Parent()->Scale() : 4096;

    return (PixelPos() * WG_SCALE_BASE ) / scale;
}

WgSize WgHook::PointSize() const
{
    int scale = Parent() ? Parent()->Scale() : 4096;

    return (PixelSize() * WG_SCALE_BASE ) / scale;
}

WgRect WgHook::PointGeo() const
{
	WgRect geo = PixelGeo();
    int scale = Parent() ? Parent()->Scale() : 4096;

	geo.x = (geo.x * WG_SCALE_BASE) / scale;
	geo.y = (geo.y * WG_SCALE_BASE) / scale;
	geo.w = (geo.w * WG_SCALE_BASE) / scale;
	geo.h = (geo.h * WG_SCALE_BASE) / scale;

	return geo;
}

WgCoord WgHook::ScreenPointPos() const
{
    int scale = Parent() ? Parent()->Scale() : 4096;
    
    return (ScreenPixelPos() * WG_SCALE_BASE ) / scale;
}

WgRect WgHook::ScreenPointGeo() const
{
	WgRect geo = ScreenPixelGeo();
    int scale = Parent() ? Parent()->Scale() : 4096;

	geo.x = (geo.x * WG_SCALE_BASE) / scale;
	geo.y = (geo.y * WG_SCALE_BASE) / scale;
	geo.w = (geo.w * WG_SCALE_BASE) / scale;
	geo.h = (geo.h * WG_SCALE_BASE) / scale;

	return geo;
}


//____ Root() _________________________________________________________________

WgRoot * WgHook::Root() const
{
	WgWidgetHolder * pHolder = _holder();

	if( pHolder->IsWidget() )
	{
		WgHook * pHook = pHolder->CastToWidget()->Hook();
		if( pHook )
			return pHook->Root();
	}
	else if( pHolder->IsRoot() )
		return pHolder->CastToRoot();

	return 0;
}

//____ EventHandler() __________________________________________________________

WgEventHandler * WgHook::EventHandler() const
{
	WgRoot * pRoot = Root();
	if( pRoot )
		return pRoot->EventHandler();

	return 0;
}

