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
#include <wg2_canvascapsule.h>
#include <wg2_gfxdevice.h>
#include <wg2_eventhandler.h>

#include <wg_gfxbase.h>

static const char    c_widgetType[] = {"CanvasCapsule"};

//____ Constructor ____________________________________________________________

WgCanvasCapsule::WgCanvasCapsule() : m_tintColor(0xFFFFFFFF), m_tintMode(WG_TINTMODE_OPAQUE), m_blendMode(WgBlendMode::Blend), m_pFactory(nullptr), m_pCanvas(nullptr),
m_fadeStartColor(0xFFFFFFFF), m_fadeEndColor(0xFFFFFFFF), m_fadeTime(0), m_fadeTimeCounter(0)
{
	m_bOpaque = false;
}

//____ Destructor _____________________________________________________________

WgCanvasCapsule::~WgCanvasCapsule()
{
}

//____ Type() _________________________________________________________________

const char * WgCanvasCapsule::Type( void ) const
{
	return GetClass();
}

//____ GetClass() ____________________________________________________________

const char * WgCanvasCapsule::GetClass()
{
	return c_widgetType;
}

//____ SetSurfaceFactory() ____________________________________________________

void WgCanvasCapsule::SetSurfaceFactory(wg::SurfaceFactory * pFactory)
{
	if (pFactory == m_pFactory)
		return;

	if (m_pCanvas)
	{
		m_pCanvas = nullptr;
		_requestRender();
	}

	m_pFactory = pFactory;
}

//____ SetCanvasFillColor() ____________________________________________________

void WgCanvasCapsule::SetCanvasFillColor(WgColor color)
{
    if( color != m_canvasFillColor )
    {
        m_canvasFillColor = color;
        _requestRender();
    }
}


//____ SetColor() ______________________________________________________________

void WgCanvasCapsule::SetColor( const WgColor& color)
{
	if (m_fadeEndColor != m_tintColor)
		_stopReceiveTicks();

	if( color != m_tintColor )
	{
		m_tintColor = color;
		_requestRender();
	}
}

//____ SetTintMode() ___________________________________________________________

void WgCanvasCapsule::SetTintMode( WgTintMode mode )
{
	if( mode != m_tintMode )
	{
		m_tintMode = mode;
		_requestRender();
	}
}

//____ SetBlendMode() __________________________________________________________

void WgCanvasCapsule::SetBlendMode( WgBlendMode mode )
{
	if( mode != m_blendMode )
	{
		m_blendMode = mode;
		_requestRender();
	}
}

//____ StartFade() ____________________________________________________________

void WgCanvasCapsule::StartFade(WgColor destination, int ms)
{
	if (destination == m_tintColor)
		return;

	m_fadeStartColor = m_tintColor;
	m_fadeEndColor = destination;
	m_fadeTimeCounter = 0;
	m_fadeTime = ms;

	_startReceiveTicks();
}

//____ StopFade() _____________________________________________________________

void WgCanvasCapsule::StopFade()
{
	m_fadeEndColor = m_tintColor;
	_stopReceiveTicks();
}

//____ FindWidget() ___________________________________________________________

WgWidget * WgCanvasCapsule::FindWidget(const WgCoord& ofs, WgSearchMode mode)
{
	if (mode == WgSearchMode::Geometry || m_tintColor.a > 0 || m_blendMode == WgBlendMode::Replace)
	{
		WgRect childGeo;
		WgHook * pHook = _lastHookWithGeo( childGeo );
		WgWidget * pResult = 0;

		
		if( childGeo.contains( ofs ) )
		{
			WgCoord flippedOfs = wg::Util::unflipCoord(ofs - childGeo.pos(), m_flip, PixelSize() );
			
			if( pHook->Widget()->IsContainer() )
			{
				pResult = pHook->Widget()->CastToContainer()->FindWidget( flippedOfs, mode );
			}
			else if( mode == WgSearchMode::Geometry || pHook->Widget()->MarkTest( flippedOfs ) )
			{
				pResult = pHook->Widget();
			}
		}

		// Return us if search mode is GEOMETRY

		if( !pResult && (mode == WgSearchMode::Geometry || MarkTest(ofs)) )
			pResult = this;

		return pResult;
	}



	return nullptr;
}

//____ SetCanvasLayers() ___________________________________________________________

void WgCanvasCapsule::SetCanvasLayers(wg::CanvasLayers * pLayers)
{
    if( pLayers != m_pCanvasLayers )
    {
        m_pCanvasLayers = pLayers;
        _requestRender();
    }
}

//____ SetRenderLayer() ___________________________________________________________

void WgCanvasCapsule::SetRenderLayer(int layer)
{
    if( layer != m_renderLayer )
    {
        m_renderLayer = layer;
        _requestRender();
    }
}

//____ SetPreferredSize() ________________________________________________________

void WgCanvasCapsule::SetPreferredSize( WgSize preferred )
{
    if( m_preferredSize != preferred )
    {
        m_preferredSize = preferred;
        _requestResize();
    }
}

//____ PreferredPixelSize() ______________________________________________________

WgSize WgCanvasCapsule::PreferredPixelSize() const
{
    if( m_preferredSize.w != -1 )
        return m_preferredSize * m_scale / WG_SCALE_BASE;
	else
	{
		WgSize szFlipped;

		if (m_hook.Widget())
		{
			WgSize sz = m_hook.Widget()->PreferredPixelSize();
			szFlipped = wg::Util::flipSize(sz, m_flip);
		}
		else
			szFlipped = WgSize(1, 1);


		if (m_pSkin)
			szFlipped += _skinContentPadding(m_pSkin, m_scale);

		return szFlipped;
	}
}

//____ MatchingPixelHeight() ________________________________________

int WgCanvasCapsule::MatchingPixelHeight( int width ) const
{
	WgSize padding;
	if( m_pSkin )
		padding = _skinContentPadding( m_pSkin, m_scale);

	if( m_hook.Widget() )
	{
		auto& mtx = wg::Util::flipMatrix(m_flip);
		if( mtx.xy == 0 && mtx.yx == 0 )
			return m_hook.Widget()->MatchingPixelHeight( width - padding.w ) + padding.h;
		else
			return m_hook.Widget()->MatchingPixelWidth( width - padding.w ) + padding.h;
	}
	else
		return WgWidget::MatchingPixelHeight(width);
}

//____ MatchingPixelWidth() ___________________________________________

int WgCanvasCapsule::MatchingPixelWidth( int height ) const
{
	WgSize padding;
	if( m_pSkin )
		padding = _skinContentPadding( m_pSkin, m_scale);

	if( m_hook.Widget() )
	{
		auto& mtx = wg::Util::flipMatrix(m_flip);
		if( mtx.xy == 0 && mtx.yx == 0 )
			return m_hook.Widget()->MatchingPixelWidth( height - padding.h ) + padding.w;
		else
			return m_hook.Widget()->MatchingPixelHeight( height - padding.h ) + padding.w;
	}
	else
		return WgWidget::MatchingPixelWidth(height);
}


//____ ForceRedraw() ___________________________________________________________

void WgCanvasCapsule::ForceRedraw()
{
    m_dirtyPatches.clear();
    m_dirtyPatches.push(PixelSize());
    _requestRender();
}

//____ SetFlip() ______________________________________________________________

void WgCanvasCapsule::SetFlip(wg::GfxFlip flip)
{
	if (flip != m_flip)
	{
		m_flip = flip;
		_requestRender();
		_requestResize();
	}
}


//____ _onEvent() _____________________________________________________________

void WgCanvasCapsule::_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler)
{
	switch( pEvent->Type() )
	{
		case WG_EVENT_TICK:
		{
			const WgEvent::Tick * pTick = static_cast<const WgEvent::Tick*>(pEvent);

			int ms = pTick->Millisec();

			m_fadeTimeCounter += pTick->Millisec();

			WgColor newColor;
			if( m_fadeTimeCounter >= m_fadeTime )
				newColor = m_fadeEndColor;
			else
			{
				int r = ((int)m_fadeEndColor.r) - m_fadeStartColor.r;
				int g = ((int)m_fadeEndColor.g) - m_fadeStartColor.g;
				int b = ((int)m_fadeEndColor.b) - m_fadeStartColor.b;
				int a = ((int)m_fadeEndColor.a) - m_fadeStartColor.a;

				float progress = m_fadeTimeCounter / (float) m_fadeTime;

				newColor.r = m_fadeStartColor.r + (int) (r * progress);
				newColor.g = m_fadeStartColor.g + (int)(g * progress);
				newColor.b = m_fadeStartColor.b + (int)(b * progress);
				newColor.a = m_fadeStartColor.a + (int)(a * progress);
			}

			if (newColor != m_tintColor)
			{
				m_tintColor = newColor;
				_requestRender();

				if (newColor == m_fadeEndColor)
					_stopReceiveTicks();
			}

		}
			break;

		default:
			WgCapsule::_onEvent(pEvent,pHandler);
			break;
	}

}



//____ _getBlendMode() _________________________________________________________

WgBlendMode WgCanvasCapsule::_getBlendMode() const
{
	return m_blendMode;
}

//____ _renderPatches() ________________________________________________________

void WgCanvasCapsule::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{
	// Make sure we have children

    if (!m_hook.Widget() && m_canvasFillColor.a == 0)
		return;

	
	WgSize canvasSize = wg::Util::unflipSize(_canvas.size(), m_flip);

	
	// Make sure we have a canvas

	if (!m_pCanvas)
	{
        wg::SurfaceFactory_p pFactory = m_pFactory ? m_pFactory : wg::GfxBase::defaultSurfaceFactory();
        
		if (!pFactory)
			return;                            // No SurfaceFactory set!
		
		WgSize maxSize = pFactory->maxSize();


		if (canvasSize.w > maxSize.w || canvasSize.h > maxSize.h)
			return;                            // Can't create a canvas of the required size!

		
		m_pCanvas = pFactory->createSurface( WGBP(Surface,
												  _.size = canvasSize,
												  _.format = WgPixelType::BGRA_8,
												  _.canvas = true) );
		m_dirtyPatches.clear();
		m_dirtyPatches.add(canvasSize);
	}

	// Go through dirty patches from screen canvas and update our back canvas where they overlap with our own

	WgPatches renderStack;

	for (const WgRect * pScreenRect = _pPatches->begin(); pScreenRect != _pPatches->end(); pScreenRect++)
	{
		WgRect r = WgRect::overlap( _canvas, *pScreenRect );

		if( r.w > 0 && r.h > 0 )
		{
			bool    bIntersected = false;
			r.x -= _canvas.x;
			r.y -= _canvas.y;

			WgRect r2 = wg::Util::unflipRect(r, m_flip, _canvas.size());

			
			for (const WgRect * pLocalDirt = m_dirtyPatches.begin(); pLocalDirt != m_dirtyPatches.end(); pLocalDirt++)
			{
				if (pLocalDirt->isOverlapping(r2))
				{
					renderStack.push(WgRect::overlap(*pLocalDirt,r2));
					bIntersected = true;
				}
			}

			if (bIntersected)
				m_dirtyPatches.sub(r2);
		}
	}

	// Save old tint color and blend mode.

	WgBlendMode        oldBM;
	WgColor            oldTC;

	oldBM = pDevice->blendMode();
	oldTC = pDevice->tintColor();


	if (!renderStack.isEmpty())
	{
		int nRects = renderStack.size();
		const wg::RectI * pRects = renderStack.begin();
		
		int nBytesReserved = nRects*sizeof(wg::RectSPX);
		
		wg::RectSPX * pClipRects = (wg::RectSPX*) wg::GfxBase::memStackAlloc(nBytesReserved);
		
		for(int i = 0 ; i < nRects ; i++)
			pClipRects[i] = pRects[i]*64;
		
		pDevice->beginCanvasUpdate(m_pCanvas, nRects, pClipRects, m_pCanvasLayers, m_renderLayer);

//        pDevice->beginCanvasUpdate(m_pCanvas, 0, nullptr, m_pCanvasLayers, m_renderLayer);
//		int bytesToRelease = _convertAndPushClipList( pDevice, renderStack.size(), renderStack.begin() );

		pDevice->setBlendMode(WgBlendMode::Replace);
		pDevice->setTintColor(WgColor::White);

		pDevice->fill(m_canvasFillColor);

		pDevice->setBlendMode(WgBlendMode::Blend);
        
        if(m_hook.Widget())
            m_hook.Widget()->_renderPatches(pDevice, canvasSize, canvasSize, &renderStack);

//		_popAndReleaseClipList( pDevice, bytesToRelease);
        pDevice->endCanvasUpdate();

		wg::GfxBase::memStackFree(nBytesReserved);
	}

	// Set our tint color and blend mode for blitting from back canvas to screen.

	pDevice->setBlendMode(m_blendMode);

	if( m_tintMode == WG_TINTMODE_OPAQUE )
		pDevice->setTintColor(m_tintColor);
	else    // MULTIPLY
		pDevice->setTintColor(m_tintColor*oldTC);

	// Render patches

	WgWidget::_renderPatches(pDevice, _canvas, _canvas, _pPatches);

	// Reset old blend mode and tint color

	pDevice->setBlendMode(oldBM);
	pDevice->setTintColor(oldTC);
}

//____ _onRender() ____________________________________________________________

void WgCanvasCapsule::_onRender(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window)
{
	// Make sure to render any skin we might have

	WgWidget::_onRender(pDevice, _canvas, _window);

	// Copy from our back canvas to the screen canvas

	pDevice->setBlitSource(m_pCanvas);
	pDevice->flipBlit( WgCoord(_canvas.x, _canvas.y)*64, m_pCanvas->pixelSize()*64, m_flip );
}


//____ _onCloneContent() _______________________________________________________

void WgCanvasCapsule::_onCloneContent( const WgWidget * _pOrg )
{
	WgCanvasCapsule * pOrg = (WgCanvasCapsule*) _pOrg;

	m_tintColor        = pOrg->m_tintColor;
	m_blendMode        = pOrg->m_blendMode;
	m_tintMode         = pOrg->m_tintMode;
}

//____ _onNewSize() ___________________________________________________________

void WgCanvasCapsule::_onNewSize(const WgSize& size)
{
	WgSize flippedSize = wg::Util::flipSize(size, m_flip);
	
	if (m_pCanvas && flippedSize != m_pCanvas->pixelSize() )
	{
		m_pCanvas = nullptr;
		_requestRender();
	}

	if( m_hook.Widget() )
	{
		WgSize sz = flippedSize;
		if( m_pSkin )
			sz -= _skinContentPadding( m_pSkin, m_scale);

		m_hook.Widget()->_onNewSize(sz);
	}
}

//____ _onRenderRequested() ___________________________________________________

void WgCanvasCapsule::_onRenderRequested()
{
	m_dirtyPatches.clear();

	WgSize sz = wg::Util::flipSize(PixelSize(), m_flip);
	m_dirtyPatches.add(sz);
	_requestRender();
}

void WgCanvasCapsule::_onRenderRequested(const WgRect& rect)
{
	m_dirtyPatches.add(rect);

	WgSize flippedSize = wg::Util::flipSize(PixelSize(), m_flip);

	WgRect unflippedRect = wg::Util::flipRect(rect, m_flip, flippedSize);

	_requestRender(unflippedRect);
}

//____ _onCollectPatches() ____________________________________________________

void WgCanvasCapsule::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	if (m_tintColor.a > 0 || m_blendMode == WgBlendMode::Replace)
		container.add( WgRect::overlap( geo, clip ) );
}

//____ _onMaskPatches() _______________________________________________________

void WgCanvasCapsule::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	//TODO: Support recursive masking.

	if( m_pCanvas && ((m_tintColor.a == 255 && m_pCanvas->pixelDescription()->A_mask == 0) || m_blendMode == WgBlendMode::Replace) )
		patches.sub(WgRect::overlap(geo, clip));

	return;
}

//____ _childSize() _____________________________________________________________

WgSize WgCanvasCapsule::_childSize()
{
	
	WgSize sz = wg::Util::flipSize(PixelSize(), m_flip);
	
	if( m_pSkin )
		sz -= _skinContentPadding( m_pSkin, m_scale);

	return sz;
}

//____ _childGeo() _____________________________________________________________

WgRect WgCanvasCapsule::_childGeo()
{
	WgSize sz = wg::Util::flipSize(PixelSize(), m_flip);

	if( m_pSkin )
		return _skinContentRect( m_pSkin, { 0,0, sz }, m_state, m_scale );
	else
		return { 0,0, sz };
}
