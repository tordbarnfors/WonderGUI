/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg2_rootpanel.h>
#include <new>

#ifndef WG2_GFXDEVICE_DOT_H
#	include <wg2_gfxdevice.h>
#endif

#include <wg_gfxbase.h>

#include <wg2_eventhandler.h>

static const char	c_hookType[] = {"RootHook"};


//____ Constructor ____________________________________________________________

WgRootPanel::WgRootPanel()
{
	m_bVisible = true;
	m_bHasGeo = false;
	m_geo = WgRect(0,0,0,0);
	m_pGfxDevice = 0;
	m_pEventHandler = new WgEventHandler(this);
	m_hook.m_pRoot = this;
	m_scale = WG_SCALE_BASE;

}


WgRootPanel::WgRootPanel( wg::Surface * pCanvasSurface, wg::GfxDevice * pGfxDevice )
{
	m_canvas.ref = wg::CanvasRef::None;
	m_canvas.pSurface = pCanvasSurface;
	m_canvas.size = pCanvasSurface->pixelSize()*64;

	m_bVisible = true;
	m_bHasGeo = false;
    m_geo = pCanvasSurface->pixelSize();
	m_pGfxDevice = pGfxDevice;
	m_pEventHandler = new WgEventHandler(this);
	m_hook.m_pRoot = this;
	m_scale = WG_SCALE_BASE;
}

WgRootPanel::WgRootPanel( wg::CanvasRef ref, wg::GfxDevice * pGfxDevice )
{
	m_canvas.ref = ref;
	m_canvas.pSurface = nullptr;
	m_canvas.size = pGfxDevice->canvas(ref).size;

    m_bVisible = true;
    m_bHasGeo = false;
    m_geo = m_canvas.size/64;
    m_pGfxDevice = pGfxDevice;
    m_pEventHandler = new WgEventHandler(this);
    m_hook.m_pRoot = this;
    m_scale = WG_SCALE_BASE;
}

//____ Destructor _____________________________________________________________

WgRootPanel::~WgRootPanel()
{
	delete m_pEventHandler;
}

//____ SetMaxDirtyRects() _____________________________________________________

void WgRootPanel::SetMaxDirtyRects( int max )
{
	m_maxDirtyRects = max;
}

//____ SetDirtyRectAlignment() ________________________________________________

void WgRootPanel::SetDirtyRectAlignment( int alignment )
{
	m_dirtyRectAlignment = alignment;
}

//____ SetMaxDirtyRectPixels() ________________________________________________

void WgRootPanel::SetMaxDirtyRectPixels( int maxDirtyRectPixels )
{
	m_maxDirtyRectPixels = maxDirtyRectPixels;
}



//____ SetGfxDevice() _________________________________________________________

bool WgRootPanel::SetGfxDevice( wg::GfxDevice * pDevice )
{
	m_pGfxDevice = pDevice;

	return true;
}

//____ SetCanvas() ________________________________________________________

bool WgRootPanel::SetCanvas(wg::Surface* pSurface)
{
	if (!pSurface)
	{
		return false;
	}

	if( pSurface == m_canvas.pSurface )
        return true;


	m_canvas.ref = wg::CanvasRef::None;
    m_canvas.pSurface = pSurface;
    m_canvas.size = pSurface->pixelSize()*64;

    if( !m_bHasGeo && m_hook.Widget() )
        m_hook.Widget()->_onNewSize(m_canvas.size/64);

    m_dirtyPatches.add(PixelGeo());
    return true;
}

bool WgRootPanel::SetCanvas(wg::CanvasRef ref)
{
	m_canvas.ref = ref;
    m_canvas.pSurface = nullptr;
    m_canvas.size = m_pGfxDevice->canvas(ref).size;

    if( !m_bHasGeo && m_hook.Widget() )
        m_hook.Widget()->_onNewSize(m_canvas.size/64);

    m_dirtyPatches.add(PixelGeo());
    return true;
}

//_____ SetPixelGeo() _____________________________________________________________

bool WgRootPanel::SetPixelGeo( const WgRect& geo )
{
	if( m_bGeoFrozen )
	{
		m_frozenGeo = geo;
		return true;
	}

	if( geo.x == 0 && geo.y == 0 && geo.w == 0 && geo.h == 0 )
		m_bHasGeo = false;
	else
		m_bHasGeo = true;

	m_geo = geo;

	if( m_hook.Widget() )
	{
		if( m_bHasGeo )
			m_hook.Widget()->_onNewSize(geo);
		else
			m_hook.Widget()->_onNewSize(m_canvas.size/64);
	}
	return true;
}

//____ PixelGeo() __________________________________________________________________

WgRect WgRootPanel::PixelGeo() const
{
	if( m_bHasGeo )
		return m_geo;
	else
        return m_canvas.size/64;
}

//____ SetFreezeGeo() _________________________________________________________

void WgRootPanel::SetFreezeGeo(bool bFreeze)
{
	if( bFreeze == m_bGeoFrozen )
		return;

	m_bGeoFrozen = bFreeze;
	if( bFreeze )
	{
		m_frozenGeo = m_geo;
		m_frozenScale = m_scale;
	}
	else
	{
		if( m_geo != m_frozenGeo )
			SetPixelGeo( m_frozenGeo );

		if( m_scale != m_frozenScale )
			SetScale(m_frozenScale);
	}
}

//____ SetChild() _____________________________________________________________

bool WgRootPanel::SetChild( WgWidget * pWidget )
{
	if( !pWidget )
		return false;

	m_hook._attachWidget(pWidget);
	m_hook.Widget()->_onNewSize(m_geo.size());

	m_hook.Widget()->_onCollectPatches( m_dirtyPatches, PixelGeo(), PixelGeo() );

	return true;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgRootPanel::ReleaseChild()
{
	return m_hook._releaseWidget();
}

WgWidget * WgRootPanel::ReleaseChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return ReleaseChild();

	return 0;
}


//____ DeleteChild() __________________________________________________________

bool WgRootPanel::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return SetChild(0);

	return false;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgRootPanel::DeleteAllChildren()
{
	DeleteChild();
	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgRootPanel::ReleaseAllChildren()
{
	return ReleaseChild()==0?false:true;
}

//____ SetVisible() ___________________________________________________________

bool WgRootPanel::SetVisible( bool bVisible )
{
	if( bVisible != m_bVisible )
	{
		m_bVisible = bVisible;
		AddDirtyPatch( PixelGeo() );
	}
	return true;
}

//____ SetUpdatedRectOverlay() _________________________________________________

void WgRootPanel::SetUpdatedRectOverlay( wg::Skin * pUpdatedRectOverlay, int afterglowFrames )
{
	m_pUpdatedRectOverlay = pUpdatedRectOverlay;
	m_afterglowFrames = afterglowFrames;
}

//____ AddDirtyPatch() __________________________________________________________

void WgRootPanel::AddDirtyPatch( const WgRect& rect )
{
	if( m_dirtyRectAlignment > 1 )
	{
		WgRect aligned;

		int adder = m_dirtyRectAlignment-1;
		int mask = ~adder;

		aligned.x = rect.x & mask;
		aligned.y = rect.y;
		aligned.w = ((rect.x + rect.w + adder) & mask) - aligned.x;
		aligned.h = rect.h;

		m_dirtyPatches.add( aligned );
	}
	else
		m_dirtyPatches.add( rect );
}


//____ Render() _______________________________________________________________

bool WgRootPanel::Render()
{
	return Render( PixelGeo() );
}

bool WgRootPanel::Render( const WgRect& clip )
{
	// This early out hopefully solves the problem with Console1 taking GPU - time on some machines when not visible.
	// It should prevent us from doing anything with the GPU
	if ( !m_bVisible || (m_dirtyPatches.isEmpty() && m_preRenderCalls.empty()) )
	{
		m_updatedPatches.clear(); // Probably not necessary, but wanna make sure they don't fill up...
		m_dirtyPatches.clear();   // Probably not necessary, but wanna make sure they don't fill up...
		m_preRenderCalls.clear();
		return true;              // Not an error, just hidden.
	}

	if( !BeginRender() )
		return false;

	if( !RenderSection(clip) )
	{
		EndRender();
		return false;
	}

	if( !EndRender() )
		return false;

	return true;
}

//____ BeginRender() __________________________________________________________

bool WgRootPanel::BeginRender()
{
	if( !m_pGfxDevice || (!m_canvas.pSurface && m_canvas.ref == wg::CanvasRef::None) || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.

	// Attend to pre-render callbacks

	for( auto& pWidget : m_preRenderCalls )
		pWidget->_preRender();

	m_preRenderCalls.clear();

	// Limit number of dirty rects

	if( m_maxDirtyRects > 0 && m_dirtyPatches.size() > m_maxDirtyRects )
	{
		WgRect u = m_dirtyPatches.bounds();
		m_dirtyPatches.clear();
		m_dirtyPatches.add(u);
	}

	// GfxDevice barfs on cliplist with rectangles (partly) outside canvas

    m_dirtyPatches.clip(PixelGeo());

	// Apply any limits to pixel-size of dirty rects.

	if( m_maxDirtyRectPixels > 0 )
	{
		int orgSize = m_dirtyPatches.size();

		for( int i = 0 ; i < orgSize ; i++ )
		{
			auto p = m_dirtyPatches.begin() + i;

			if( p->w * p->h > m_maxDirtyRectPixels )
			{
				int maxLines = m_maxDirtyRectPixels / p->w;

				int linesLeft = p->h - maxLines;
				int yPos = p->y + maxLines;

				const_cast<WgRect*>(p)->h = maxLines;

				while( linesLeft > maxLines )
				{
					m_dirtyPatches.add( WgRect(p->x,yPos,p->w,maxLines) );
					linesLeft -= maxLines;
					yPos += maxLines;
				}

				if( linesLeft > 0 )
					m_dirtyPatches.add( WgRect(p->x,yPos,p->w,linesLeft) );
			}
		}
	}


    //

	if( m_pUpdatedRectOverlay )
	{
		// Remove from afterglow queue patches that are overlapped by our new dirty patches.

		for( std::deque<WgPatches>::iterator it = m_afterglowRects.begin() ; it != m_afterglowRects.end() ; ++it )
			it->sub(&m_dirtyPatches);

		// Add our new dirty patches to the top of the afterglow queue.


		m_afterglowRects.push_front(WgPatches());
		m_afterglowRects.front().add(&m_dirtyPatches);

		// Possibly remove overlays from the back, put them into dirty rects for re-render

		while( (int) m_afterglowRects.size() > m_afterglowFrames+1 )
		{
			m_dirtyPatches.add( &m_afterglowRects.back() );
			m_afterglowRects.pop_back();
		}

		// Re-render graphics behind overlays that go from state FOCUSED to NORMAL

		if( m_afterglowRects.size() > 1 )
		{
			m_dirtyPatches.add( &m_afterglowRects[1] );
		}
	}

	// Initialize GfxDevice

	bool m_bWasAlreadyRendering = m_pGfxDevice->isRendering();

	if( m_bWasAlreadyRendering )
		return true;
	else
		return m_pGfxDevice->beginRender();
}


//____ RenderSection() __________________________________________________________

bool WgRootPanel::RenderSection( const WgRect& _clip )
{
    if( !m_pGfxDevice || (!m_canvas.pSurface && m_canvas.ref == wg::CanvasRef::None) || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.

	// Make sure we have a vaild clip rectangle (doesn't go outside our geometry and has an area)

    WgRect canvas = m_bHasGeo ? WgRect::overlap(m_geo, WgRect(m_canvas.size/64)) : WgRect(m_canvas.size/64);
	WgRect clip = WgRect::overlap( _clip, canvas );
	if( clip.w == 0 || clip.h == 0 )
		return false;						// Invalid rect area.

	// Nothing to render if our only child is hidden

	if( !m_bVisible )
		return true;						// Not an error, just hidden.

	// Copy and clip our dirty patches
	// TODO: Optimize when clip rectangle equals canvas

	WgPatches dirtyPatches( m_dirtyPatches.size() );

	for( const WgRect * pRect = m_dirtyPatches.begin() ; pRect != m_dirtyPatches.end() ; pRect++ )
	{
		if( clip.isOverlapping(*pRect) )
			dirtyPatches.push( WgRect::overlap(*pRect, clip) );
	}

	// Render the dirty patches recursively

	if( dirtyPatches.size() > 0 )
    {
        int nRects = dirtyPatches.size();
        const WgRect* pRects = dirtyPatches.begin();

		wg::RectSPX * pClipRects = (wg::RectSPX*) wg::GfxBase::memStackAlloc(nRects*sizeof(wg::RectSPX));

		for(int i = 0 ; i < nRects ; i++)
			pClipRects[i] = pRects[i]*64;

        if (m_canvas.pSurface)
            m_pGfxDevice->beginCanvasUpdate(m_canvas.pSurface, nRects, pClipRects);
        else
            m_pGfxDevice->beginCanvasUpdate(m_canvas.ref, nRects, pClipRects);

        m_hook.Widget()->_renderPatches( m_pGfxDevice, canvas, canvas, &dirtyPatches );

        m_pGfxDevice->endCanvasUpdate();

		wg::GfxBase::memStackFree(nRects*sizeof(wg::RectSPX));

    }
	// Handle updated rect overlays

	if( m_pUpdatedRectOverlay )
	{
        // Set clipping rectangle.

        wg::RectI myClip = clip*64;

        if (m_canvas.pSurface)
            m_pGfxDevice->beginCanvasUpdate(m_canvas.pSurface, 1, &myClip, 0);
        else
            m_pGfxDevice->beginCanvasUpdate(m_canvas.ref, 1, &myClip, 0);

		// Render our new overlays

		for( const WgRect * pRect = m_afterglowRects[0].begin() ; pRect != m_afterglowRects[0].end() ; pRect++ )
		{
			m_pUpdatedRectOverlay->_render( m_pGfxDevice,(*pRect)*64, m_scale >> 6, WgStateEnum::Focused );
		}

		// Render overlays that have turned into afterglow

		if( m_afterglowRects.size() > 1 )
		{
			for( const WgRect * pRect = m_afterglowRects[1].begin() ; pRect != m_afterglowRects[1].end() ; pRect++ )
			{
				m_pUpdatedRectOverlay->_render( m_pGfxDevice, (*pRect)*64, m_scale >> 6, WgStateEnum::Default );
			}
		}

        m_pGfxDevice->endCanvasUpdate();
	}

	return true;
}

//____ EndRender() ____________________________________________________________

bool WgRootPanel::EndRender( void )
{
	if( !m_pGfxDevice || (!m_canvas.pSurface && m_canvas.size.isEmpty()) || !m_hook.Widget() )
		return false;						// No GFX-device or no widgets to render.


	// Turn dirty patches into updated patches
	//TODO: Optimize by just making a swap.

	m_updatedPatches.clear();
	m_updatedPatches.add(&m_dirtyPatches);
	m_dirtyPatches.clear();

	if( !m_bWasAlreadyRendering )
		return m_pGfxDevice->endRender();
	else
		return true;
}


//____ FindWidget() _____________________________________________________________

WgWidget * WgRootPanel::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	if( !PixelGeo().contains(ofs) || !m_hook.Widget() )
		return 0;

	if( m_hook.Widget() && m_hook.Widget()->IsContainer() )
		return m_hook.Widget()->CastToContainer()->FindWidget( ofs, mode );

	return m_hook.Widget();
}

//____ SetScale() ______________________________________________________________

void WgRootPanel::SetScale( int scale )
{
	if( m_scale != scale )
	{
		if( m_bGeoFrozen )
		{
			m_frozenScale = scale;
			return;
		}

		m_scale = scale;
		if( m_hook.Widget() )
			m_hook.Widget()->_setScale(m_scale);
	}
}


//____ _focusRequested() _______________________________________________________

bool WgRootPanel::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	if( m_pEventHandler )
		return m_pEventHandler->SetKeyboardFocus(pWidgetRequesting);
	else
		return false;
}

//____ _focusReleased() ________________________________________________________

bool WgRootPanel::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	if( m_pEventHandler )
		return m_pEventHandler->SetKeyboardFocus(0);
	else
		return false;
}

//____ _inViewRequested() ________________________________________________________

void WgRootPanel::_inViewRequested( WgHook * pChild )
{
	// Do nothing.
}

void WgRootPanel::_inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	// Do nothing.
}


///////////////////////////////////////////////////////////////////////////////

WgRootPanel::Hook::~Hook()
{
}

const char * WgRootPanel::Hook::Type( void ) const
{
	return ClassType();
}

const char * WgRootPanel::Hook::ClassType()
{
	return c_hookType;
}

WgCoord WgRootPanel::Hook::PixelPos() const
{
	return m_pRoot->PixelGeo();
}

WgSize WgRootPanel::Hook::PixelSize() const
{
	return m_pRoot->PixelGeo();
}

WgRect WgRootPanel::Hook::PixelGeo() const
{
	return m_pRoot->PixelGeo();
}

WgCoord WgRootPanel::Hook::ScreenPixelPos() const
{
	return m_pRoot->PixelGeo();
}

WgRect WgRootPanel::Hook::ScreenPixelGeo() const
{
	return m_pRoot->PixelGeo();
}

WgRootPanel* WgRootPanel::Hook::Root() const
{
	return m_pRoot;
}

void WgRootPanel::Hook::_requestRender()
{
	if( m_pRoot->m_bVisible )
		m_pRoot->AddDirtyPatch( PixelGeo() );
}

void WgRootPanel::Hook::_requestRender( const WgRect& rect )
{
	if( m_pRoot->m_bVisible )
		m_pRoot->AddDirtyPatch( WgRect( PixelPos() + rect.pos(), rect.size() ) );
}

void WgRootPanel::Hook::_requestResize()
{
	// Do nothing, root ignores size requests.
}

WgHook * WgRootPanel::Hook::_prevHook() const
{
	return 0;
}

WgHook * WgRootPanel::Hook::_nextHook() const
{
	return 0;
}

WgWidgetHolder * WgRootPanel::Hook::_holder() const
{
	return m_pRoot;
}


WgContainer * WgRootPanel::Hook::_parent() const
{
	return 0;
}

