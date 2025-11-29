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
#include <wg2_widget.h>
#include <wg2_base.h>
#include <wg2_types.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>
#include <wg_dataset.h>
#include <wg_gfxbase.h>
#include <wg_surfacefactory.h>
#include <wg_util.h>

#	include <wg2_rootpanel.h>
#	include <wg2_eventhandler.h>

//____ Constructor ____________________________________________________________

WgWidget::WgWidget():m_id(0), m_pHook(0), m_pointerStyle(WgPointerStyle::Arrow),
					m_markOpacity( 1 ), m_bEnabled(true), m_bOpaque(false),
					m_bFocused(false), m_bSelected(false), m_bTabLock(false), m_bReceiveTick(false), m_scale(WG_SCALE_BASE),
					m_bPickable(false), m_bDropTarget(false)
{

	if( WgBase::s_bTrackingWidgets )
		WgBase::s_trackedWidgets.push_back(this);
}

//____ Destructor _____________________________________________________________

WgWidget::~WgWidget()
{
	if( m_pHook )
	{
		m_pHook->Parent()->ReleaseChild(this);
	}

	if( WgBase::s_bTrackingWidgets )
		WgBase::s_trackedWidgets.erase(std::remove(WgBase::s_trackedWidgets.begin(), WgBase::s_trackedWidgets.end(), this));
}


//____ GetPointerStyle() ________________________________________

WgPointerStyle WgWidget::GetPointerStyle() const
{
	return m_pointerStyle;
}

//____ SetSkin() ______________________________________________________________

void WgWidget::SetSkin(wg::Skin * pSkin)
{
	if (pSkin != m_pSkin)
	{
		m_pSkin = pSkin;
		_requestResize();
		_requestRender();
	}
}

//____ SetEnabled() _______________________________________________________________

void WgWidget::SetEnabled( bool bEnabled )
{
	if( m_bEnabled != bEnabled || IsContainer() )
	{
		m_bEnabled = bEnabled;
		m_state.setDisabled(!bEnabled);
		if( bEnabled )
			_onEnable();
		else
			_onDisable();
	}
}

//____ MarkTest() _____________________________________________________________

bool WgWidget::MarkTest( const WgCoord& ofs )
{
	switch( m_markOpacity )
	{
	case 0:
		return true;
	case 256:
		return false;
	default:
		return _onAlphaTest(ofs);
	}
}

//____ CloneContent() _________________________________________________________

bool WgWidget::CloneContent( const WgWidget * _pOrg )
{
	if( _pOrg->Type() != Type() )
		return false;

	m_id			= _pOrg->m_id;
	m_pSkin         = _pOrg->m_pSkin;
	m_scale         = _pOrg->m_scale;
	m_pointerStyle 	= _pOrg->m_pointerStyle;

	m_tooltip		= _pOrg->m_tooltip;
	m_markOpacity	= _pOrg->m_markOpacity;

	m_bOpaque        = _pOrg->m_bOpaque;
	m_bTabLock        = _pOrg->m_bTabLock;

	m_bReceiveTick  = _pOrg->m_bReceiveTick;    // Set if Widget should reveive periodic Tick() events.

	m_bEnabled        = _pOrg->m_bEnabled;
	m_bSelectable     = _pOrg->m_bSelectable;

	m_state           = _pOrg->m_state;

	m_bPickable         = _pOrg->m_bPickable;
	m_pickCategory     = _pOrg->m_pickCategory;
	m_bDropTarget    = _pOrg->m_bDropTarget;



	_onCloneContent( _pOrg );
	return true;
}

//____ _onNewHook() ___________________________________________________________

void WgWidget::_onNewHook( WgHook * pHook )
{
	if( pHook )
	{
		WgRoot * pNewRoot = pHook->Root();
		WgRoot * pOldRoot = m_pHook ? m_pHook->Root() : 0;


		if( pNewRoot && pNewRoot != pOldRoot )
			_onNewRoot( pNewRoot );
	}

	m_pHook = pHook;

}

//____ _onNewRoot() ___________________________________________________________

void WgWidget::_onNewRoot( WgRoot * pRoot )
{
	if( m_bReceiveTick && pRoot )
		pRoot->EventHandler()->_addTickReceiver(this);
}

//____ _startReceiveTicks() ___________________________________________________

void WgWidget::_startReceiveTicks()
{
	if( !m_bReceiveTick )
	{
		m_bReceiveTick = true;

		if( m_pHook )
		{
			WgRoot * pRoot = m_pHook->Root();
			if( pRoot )
				pRoot->EventHandler()->_addTickReceiver(this);
		}
	}
}

//____ _stopReceiveTicks() ____________________________________________________

void WgWidget::_stopReceiveTicks()
{
	m_bReceiveTick = false;
}

//____ LockPointer() __________________________________________________________

bool WgWidget::LockPointer()
{
    WgRoot * pRoot = m_pHook->Root();
    if( pRoot )
        m_bPointerLocked = pRoot->EventHandler()->_widgetLockPointer(this);
    
    return m_bPointerLocked;
}

//____ ReleasePointer() __________________________________________________________

void WgWidget::ReleasePointer()
{
    WgRoot * pRoot = m_pHook->Root();
    if( pRoot )
        pRoot->EventHandler()->_widgetReleasePointer(this);
    
    m_bPointerLocked = false;
}

//____ Local2absPixel() ____________________________________________________________

WgCoord WgWidget::Local2absPixel( const WgCoord& cord ) const
{
	WgCoord c = ScreenPixelPos();
	c.x += cord.x;
	c.y += cord.y;
	return c;
}

//____ Abs2localPixel() ____________________________________________________________

WgCoord WgWidget::Abs2localPixel( const WgCoord& cord ) const
{
	WgCoord c = ScreenPixelPos();
	return WgCoord( cord.x - c.x, cord.y - c.y );
}

//____ Local2absPoint() ____________________________________________________________

WgCoord WgWidget::Local2absPoint( const WgCoord& cord ) const
{
	return (Local2absPixel(cord*m_scale/WG_SCALE_BASE)*WG_SCALE_BASE) / m_scale;
}

//____ Abs2localPoint() ____________________________________________________________

WgCoord WgWidget::Abs2localPoint( const WgCoord& cord ) const
{
	return (Abs2localPixel(cord*m_scale/WG_SCALE_BASE)*WG_SCALE_BASE) / m_scale;
}

//____ Screenshot() ___________________________________________________________

wg::Surface_p WgWidget::Screenshot( const wg::Surface::Blueprint& bp )
{
	return Screenshot( WgRect(), bp );
}

wg::Surface_p WgWidget::Screenshot( const WgRect& _rect, const wg::Surface::Blueprint& _bp )
{
	auto pDevice = wg::GfxBase::defaultGfxDevice();
	auto pFactory =  wg::GfxBase::defaultSurfaceFactory();

	if( !pDevice || !pFactory )
		return nullptr;

	
	WgRect rect = _rect.isEmpty() ? WgRect(0,0,PixelSize()) : WgRect::overlap(PixelSize(), _rect*m_scale/WG_SCALE_BASE);

	wg::Surface::Blueprint bp = _bp;
	bp.size = rect.size();
	bp.format = wg::PixelFormat::BGRA_8;
	bp.canvas = true;
	bp.scale = m_scale/4096.f*64;
	
	auto pCanvas = pFactory->createSurface( bp );
    if(!pCanvas)
        return nullptr;

	WgPatches patches;
	patches.add( rect.size() );

	WgRect geoWithOffset = { -rect.x, -rect.y, rect.w, rect.h };
	
    bool    bWasRendering = pDevice->isRendering();
    
	if( !bWasRendering )
        pDevice->beginRender();

    pDevice->beginCanvasUpdate(pCanvas);
    pDevice->setBlendMode(wg::BlendMode::Replace);
    pDevice->fill( WgColor::Transparent );
    pDevice->setBlendMode(wg::BlendMode::Blend);
    _renderPatches(pDevice, geoWithOffset, geoWithOffset, &patches);
    pDevice->endCanvasUpdate();

    if( !bWasRendering )
        pDevice->endRender();

	return pCanvas;
}
//____ CommonAncestor() ___________________________________________________
/**
    @brief Finds closest common ancestor of widgets.

    Finds the closest common ancestor between this and pOtherWidget (if any).
    The result can be one of the specified widgets if one is an ancestor
    of the other.

    @param pOtherWidget    The widget this is checked against.

    @return The closest common ancestor, which can be one of the widgets specifed,
            or nullptr if they are not related.
*/
WgWidget * WgWidget::CommonAncestor(WgWidget* pOtherWidget)
{
    if (pOtherWidget == nullptr)
        return nullptr;

    const int c_maxLevels = 128;

    WgWidget* list1[c_maxLevels];
    WgWidget* list2[c_maxLevels];

    list1[0] = this;
    list2[0] = pOtherWidget;

    int nb1 = _listAncestors(list1+1, c_maxLevels-1) + 1;
    int nb2 = pOtherWidget->_listAncestors(list2+1, c_maxLevels-1) + 1;

    if (nb1 == c_maxLevels-1 || nb2 == c_maxLevels-1)
    {
//        wg::GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::Internal, "Widget hierarchy too deep for operation.", this, Widget::TYPEINFO, __func__, __FILE__, __LINE__);
        return nullptr;
    }

    int maxSearchDepth = std::min(nb1, nb2);

    int i = 0;
    while (i < maxSearchDepth && list1[nb1-1 - i] == list2[nb2-1 - i])
        i++;

    if (i == 0 )
        return nullptr;

    return list1[nb1 - i];
}


//____ setDropTarget() ____________________________________________________

void WgWidget::setDropTarget(bool bDropTarget)
{
	m_bDropTarget = bDropTarget;
}

//____ setPickable() ____________________________________________________________

void WgWidget::setPickable( bool bPickable, int category )
{
	m_bPickable = bPickable;
	m_pickCategory = category;
}


//____ _eventHandler() __________________________________________________________

WgEventHandler * WgWidget::_eventHandler() const
{
	if( m_pHook )
	{
		WgRoot * pRoot = m_pHook->Root();
		if( pRoot )
			return pRoot->EventHandler();
	}
	return 0;
}

//____ MatchingPixelHeight() _______________________________________________________

int WgWidget::MatchingPixelHeight( int width ) const
{
	return PreferredPixelSize().h;		// Default is to stick with best height no matter what width.
}

//____ MatchingPixelWidth() _______________________________________________________

int WgWidget::MatchingPixelWidth( int height ) const
{
	return PreferredPixelSize().w;		// Default is to stick with best width no matter what height.
}

//____ MatchingPointHeight() _______________________________________________________

int WgWidget::MatchingPointHeight( int pixelWidth ) const
{
    return MatchingPixelHeight(pixelWidth) * WG_SCALE_BASE / Scale();
}

//____ MinPixelSize() ______________________________________________________________

WgSize WgWidget::MinPixelSize() const
{
	return WgSize(0,0);
}

//____ MaxPixelSize() ______________________________________________________________

WgSize WgWidget::MaxPixelSize() const
{
	return WgSize(2<<20,2<<20);
}

//____ SetMarked() ____________________________________________________________

bool WgWidget::SetMarked()
{
	return false;
}

//____ SetSelected() __________________________________________________________

bool WgWidget::SetSelected()
{
	if (m_bEnabled)
	{
		m_bSelected = true;

		if (!m_state.isSelekted())
		{
			m_state.setSelekted(true);
			_requestRender();
		}
		return true;
	}
	else
		return false;
}

//____ SetNormal() ____________________________________________________________

bool WgWidget::SetNormal()
{
	if (m_bEnabled)
	{
		m_bSelected = false;
		if (m_state.isSelekted())
		{
			m_state.setSelekted(false);
			_requestRender();
		}
		return true;
	}
	else
		return false;
}

//____ SetSelectable() ________________________________________________________

void WgWidget::SetSelectable(bool bSelectable)
{
	m_bSelectable = bSelectable;
}

//____IsDisplayed() ___________________________________________________________

bool WgWidget::IsDisplayed() const
{
	if( m_pHook )
		return m_pHook->_isDisplayed();
	
	return false;
}

//____ Mode() _________________________________________________________________

WgMode WgWidget::Mode() const
{
	return WgUtil::StateToMode(m_state);

//	if (m_bSelected)
//		return WG_MODE_SELECTED;
//	else if( m_bEnabled )
//		return WG_MODE_NORMAL;
//	else
//		return WG_MODE_DISABLED;
}

//____ PreferredPointSize() ___________________________________________________

WgSize WgWidget::PreferredPointSize() const
{
	WgSize sz = PreferredPixelSize()*WG_SCALE_BASE;

	sz.w += m_scale - 1;		// Preferred size should be rounded up.
	sz.h += m_scale - 1;

	return sz / m_scale;
};

//____ MinPointSize() ___________________________________________________

WgSize WgWidget::MinPointSize() const
{
	WgSize sz = MinPixelSize()*WG_SCALE_BASE;

	sz.w += m_scale - 1;		// Min size should be rounded up.
	sz.h += m_scale - 1;

	return sz / m_scale;
};


//____ MaxPointSize() ___________________________________________________

WgSize WgWidget::MaxPointSize() const
{
	WgSize sz = MaxPixelSize();
	return sz*WG_SCALE_BASE / m_scale;
};



//____ _getBlendMode() _________________________________________________________

WgBlendMode WgWidget::_getBlendMode() const
{
	WgContainer * pParent = Parent();
	if( pParent )
		return pParent->_getBlendMode();
	else
		return WgBlendMode::Blend;		// We always start out with WgBlendMode::Blend.
}

//____ _listAncestors() ________________________________________________________

int WgWidget::_listAncestors(WgWidget* array[], int max)
{
    int n = 0;
    WgWidget* p = Parent();
    while (p != nullptr && n < max)
    {
        array[n++] = p;
        p = p->Parent();
    }
    return n;
}

//____ _queueEvent() __________________________________________________________

void WgWidget::_queueEvent( WgEvent::Event * pEvent )
{
	if( m_pHook )
	{
		WgRoot * pRoot = m_pHook->Root();
		if( pRoot )
		{
			pRoot->EventHandler()->QueueEvent(pEvent);
			return;
		}
	}

	delete pEvent;		// Can't queue event, silently delete it.
}

//____ _requestPreRenderCall() ___________________________________________________

bool WgWidget::_requestPreRenderCall()
{
	if( m_pHook )
	{
		WgRoot * pRoot = m_pHook->Root();
		if( pRoot )
		{
			pRoot->_addPreRenderCall(this);
			return true;
		}
	}

	return false;
}


//____ _preRender() ____________________________________________________________

void WgWidget::_preRender()
{
	// By default we do nothing.
}

//____ _convertAndPushClipList() ________________________________________________

int WgWidget::_convertAndPushClipList( wg::GfxDevice * pDevice, int nRects, const WgRect * pRects )
{
	// Compare cliplist to what we already have. Only push if different.

	assert( nRects != 0 );
	
	// TODO: Fix this!. But remove this optimization for now
    /*
	if( nRects == pDevice->clipListSize() )
	{
		const wg::RectI * pCurrent = pDevice->clipList();
		int i = 0;
		while( i < nRects )
		{
			if( pCurrent[i] != pRects[i]*64 )
				break;
			i++;
		}
		
		if( i == nRects )
			return 0;				// Cliplists are identical. We don't push.
	}
	*/
	
	int nBytes = nRects*sizeof(wg::RectSPX);
	
	wg::RectSPX * pClipRects = (wg::RectSPX*) wg::GfxBase::memStackAlloc(nBytes);
	
	for(int i = 0 ; i < nRects ; i++)
		pClipRects[i] = pRects[i]*64;
	
	pDevice->pushClipList(nRects, pClipRects);
	return nBytes;
}

//____ _popAndReleaseClipList() _______________________________________________

void WgWidget::_popAndReleaseClipList( wg::GfxDevice * pDevice, int bytesToRelease)
{
    // TODO: Fix this!. But remove this optimization for now
	//if( bytesToRelease == 0 )
	//	return;						// No cliplist was pushed.
		
	pDevice->popClipList();
	wg::GfxBase::memStackFree(bytesToRelease);
}


//____ _renderPatches() ________________________________________________________

void WgWidget::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	if( _pPatches->isEmpty() )
		return;

	int bytesToRelease = _convertAndPushClipList(pDevice, _pPatches->size(), _pPatches->begin());
	_onRender( pDevice, _canvas, _window );
	_popAndReleaseClipList(pDevice, bytesToRelease);
}

//____ Fillers _______________________________________________________________

void WgWidget::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
		container.add( WgRect::overlap( geo, clip ) );
}

void WgWidget::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	if( ((m_bOpaque || (m_pSkin && m_pSkin->isOpaque())) && blendMode == WgBlendMode::Blend) || blendMode == WgBlendMode::Replace )
	{
		patches.sub( WgRect::overlap( geo, clip ) );
	}
}

void WgWidget::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	if (m_pSkin)
	{
		_renderSkin( m_pSkin, pDevice, m_state, _canvas, m_scale);
	}
}

void WgWidget::_onNewSize( const WgSize& size )
{
	_requestRender();
}

void WgWidget::_setScale( int scale )
{
	m_scale = scale;
}

void WgWidget::_onRefresh()
{
	_requestRender();
}

void WgWidget::_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler )
{
	WgState state = m_state;
	bool    bForward = true;

	switch( _pEvent->Type() )
	{
		case WG_EVENT_DROP_PICK:
		{
			auto pEvent = static_cast<const WgEvent::DropPick*>(_pEvent);
			if (!pEvent->hasPayload())
			{
				const_cast<WgEvent::DropPick*>(pEvent)->setPayload(wg::Dataset<WgWidgetWeakPtr>::create(this));
			}
			bForward = false;
			break;
		}
		case WG_EVENT_DROP_ENTER:
			state.setTargeted(true);
			bForward = false;
			break;
		case WG_EVENT_DROP_LEAVE:
			state.setTargeted(false);
			bForward = false;
			break;

        case WG_EVENT_MOUSE_ENTER:
            if( m_bFullStateSupport )
                state.setHovered(true);
            break;
            
        case WG_EVENT_MOUSE_LEAVE:
            if( m_bFullStateSupport )
                state.setHovered(false);
            
		default:
			break;
	}

    if( m_bFullStateSupport )
    {
        
    }
    
    
	if( state != m_state )
	{
		//TODO: We should have a virtual method for setting state, just like WG3.

        _setState(state);
	}

	if( bForward && (_pEvent->IsKeyEvent() || _pEvent->IsMouseEvent() || _pEvent->Type() == WG_EVENT_MOUSEWHEEL_ROLL) && _pEvent->Type() != WG_EVENT_MOUSE_ENTER && _pEvent->Type() != WG_EVENT_MOUSE_LEAVE )
		pHandler->ForwardEvent( _pEvent );
}


//____ _setState() _________________________________________________________

void WgWidget::_setState( WgState state )
{
    WgState oldState = m_state;
    m_state = state;

//    OO(skin)._stateChanged(state, oldState);
//    m_bOpaque = OO(skin)._isOpaque(state);
    
    _requestRender();
}

bool WgWidget::_onAlphaTest( const WgCoord& ofs )
{
	if (m_pSkin)
		return _markTestSkin( m_pSkin, ofs, PixelSize(), WgStateEnum::Default, m_markOpacity, m_scale);
	else
		return false;
}

void WgWidget::_onEnable()
{
    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setDisabled(false);
        _setState(state);
    }
	_requestRender();
}

void WgWidget::_onDisable()
{
    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setDisabled(true);
        _setState(state);
    }
	_requestRender();
}

void WgWidget::_onGotInputFocus()
{
	m_bFocused = true;
    
    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setFocused(true);
        _setState(state);
    }
    else
        m_state.setFocused(true);

	_queueEvent(new WgEvent::FocusGained(this));
}

void WgWidget::_onLostInputFocus()
{
	m_bFocused = false;

    if( m_bFullStateSupport )
    {
        WgState state = m_state;
        state.setFocused(false);
        _setState(state);
    }
    else
        m_state.setFocused(false);

    _queueEvent(new WgEvent::FocusLost(this));
}


bool WgWidget::TempIsInputField() const
{
	return false;
}

Wg_Interface_TextHolder* WgWidget::TempGetText()
{
	return 0;
}

//____ _renderSkin() ____________________________________________________________

void WgWidget::_renderSkin( wg::Skin * pSkin, wg::GfxDevice * pDevice, wg::State state, const wg::RectI& rect, int scale, float value1, float value2 )
{
	pSkin->_render( pDevice, rect*64, scale >> 6, state, value1, value2 );
}

//____ _markTestSkin() ______________________________________________________________

bool WgWidget::_markTestSkin( wg::Skin * pSkin, const wg::CoordI& ofs, const wg::RectI& canvas, wg::State state, int opacityTreshold, int scale, float value1, float value2) const
{
	int alphaOverride = m_markOpacity >= 0 ? m_markOpacity*4096/255 : -1;
	
	return pSkin->_markTest( ofs*64, canvas*64, scale >> 6, state, value1, value2, alphaOverride );
}

//____ _skinMinSize() __________________________________________________________

wg::SizeI WgWidget::_skinMinSize( wg::Skin * pSkin, int scale ) const
{
	return pSkin->_minSize(scale >> 6) / 64;
}

//____ _skinPreferredSize() __________________________________________________________

wg::SizeI WgWidget::_skinPreferredSize( wg::Skin * pSkin, int scale ) const
{
	return pSkin->_defaultSize(scale >> 6) / 64;
}

//____ _skinSizeForContent() __________________________________________________________

wg::SizeI WgWidget::_skinSizeForContent(wg::Skin * pSkin, const wg::SizeI contentSize, int scale) const
{
	return pSkin->_sizeForContent(contentSize*64, scale >> 6) / 64;
}

//____ _skinContentPadding() __________________________________________________________

wg::SizeI WgWidget::_skinContentPadding(wg::Skin * pSkin, int scale) const
{
	return pSkin->_contentBorder(scale >> 6, m_state) / 64;
}

//____ _skinContentOfs() __________________________________________________________

wg::CoordI WgWidget::_skinContentOfs(wg::Skin * pSkin, wg::State state, int scale) const
{
	return pSkin->_contentOfs(scale >> 6, state) / 64;
}

//____ _skinContentRect() __________________________________________________________

wg::RectI WgWidget::_skinContentRect(wg::Skin * pSkin, const wg::RectI& canvas, wg::State state, int scale) const
{
	return pSkin->_contentRect(canvas * 64, scale >> 6, state) / 64;
}

//____ _skinDirtyRect() _____________________________________________________________

wg::RectI WgWidget::_skinDirtyRect(wg::Skin * pSkin, const wg::RectI& canvas, int scale, wg::State newState, wg::State oldState,
                                   float newValue, float oldValue, float newValue2, float oldValue2, int newAnimPos, int oldAnimPos ) const
{
	return pSkin->_dirtyRect(canvas*64, scale >> 6,newState,oldState,newValue,oldValue,newValue2,oldValue2,newAnimPos,oldAnimPos) / 64;
}
