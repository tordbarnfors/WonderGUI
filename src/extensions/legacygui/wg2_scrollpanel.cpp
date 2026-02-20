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
#include <wg2_scrollpanel.h>
#include <wg2_gfxdevice.h>
#include <wg2_util.h>
#include <wg2_patches.h>
#include <wg2_event.h>
#include <wg2_eventhandler.h>

#include <algorithm>



static const char	c_widgetType[] = {"ScrollPanel"};
static const char	c_hookType[] = {"ScrollHook"};


//____ Constructor ____________________________________________________________

WgScrollPanel::WgScrollPanel()
{
	m_sliderTargets[0].m_bHorizontal = false;
	m_sliderTargets[0].m_pParent = this;

	m_sliderTargets[1].m_bHorizontal = true;
	m_sliderTargets[1].m_pParent = this;

	m_bgColor = WgColor::White;
	m_contentOrigo = WgOrigo::NorthWest;
	m_widthPolicy = wg::SizeConstraint::None;
	m_heightPolicy = wg::SizeConstraint::None;


	m_stepSizeX		= 1;
	m_stepSizeY		= 1;
	m_jumpSizeX		= 0.75f;
	m_jumpSizeY		= 0.75f;

	m_viewPixOfs.x	= 0;
	m_viewPixOfs.y	= 0;

	m_bAutoHideSliderX = false;
	m_bAutoHideSliderY = false;

	m_bSliderBottom	= true;
	m_bSliderRight	= true;

	m_bAutoScrollX		= false;
	m_bAutoScrollY		= false;

    m_mouseWheelAxis[0] = wg::Axis::Undefined;
    m_mouseWheelAxis[1] = wg::Axis::Y;
    m_mouseWheelAxis[2] = wg::Axis::X;
    m_mouseWheelAxis[3] = wg::Axis::Undefined;
    m_mouseWheelAxis[4] = wg::Axis::Undefined;

	m_bOverlayScrollbars = true;

	m_elements[WINDOW]._setParent(this);
	m_elements[XDRAG]._setParent(this);
	m_elements[YDRAG]._setParent(this);

	_updateElementGeo( WgSize(256,256) );
}

//____ ~WgScrollPanel() __________________________________________________

WgScrollPanel::~WgScrollPanel()
{
}

//____ Type() _________________________________________________________________

const char * WgScrollPanel::Type() const
{
	return GetClass();
}

const char * WgScrollPanel::GetClass( void )
{
	return c_widgetType;
}

//____ SetOverlayScrollbars() ____________________________________________________

void WgScrollPanel::SetOverlayScrollbars( bool bOverlay )
{
	if( bOverlay != m_bOverlayScrollbars )
	{
		m_bOverlayScrollbars = bOverlay;
		_updateElementGeo( PixelSize() );
	}
}

//____ StepUp() _______________________________________________________________

bool WgScrollPanel::StepUp()
{
	int ofs = m_viewPixOfs.y - ((m_stepSizeY*m_scale) >> WG_SCALE_BINALS);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( ofs );
}

//____ StepDown() _____________________________________________________________
bool WgScrollPanel::StepDown()
{
	return SetViewPixelOfsY( m_viewPixOfs.y + ((m_stepSizeY*m_scale) >> WG_SCALE_BINALS));
}

//____ StepLeft() _____________________________________________________________
bool WgScrollPanel::StepLeft()
{
	int ofs = m_viewPixOfs.x - ((m_stepSizeX*m_scale) >> WG_SCALE_BINALS);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( ofs );
}

//____ StepRight() ____________________________________________________________

bool WgScrollPanel::StepRight()
{
	return SetViewPixelOfsX( m_viewPixOfs.x + ((m_stepSizeX*m_scale) >> WG_SCALE_BINALS) );
}


//____ JumpUp() _______________________________________________________________

bool WgScrollPanel::JumpUp()
{
	int ofs = m_viewPixOfs.y - (Uint32)(ViewPixelLenY() * m_jumpSizeY);
	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( (Uint32) ofs );
}

//____ JumpDown() _____________________________________________________________

bool WgScrollPanel::JumpDown()
{
	return SetViewPixelOfsY( (Uint32)(m_viewPixOfs.y + ViewPixelLenY() * m_jumpSizeY) );
}

//____ JumpLeft() _____________________________________________________________

bool WgScrollPanel::JumpLeft()
{
	int ofs = m_viewPixOfs.x - (Uint32)(ViewPixelLenX() * m_jumpSizeX);
	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( (Uint32) ofs );
}

//____ JumpRight() ____________________________________________________________

bool WgScrollPanel::JumpRight()
{
	return SetViewPixelOfsX( (Uint32)(m_viewPixOfs.x + ViewPixelLenX() * m_jumpSizeX) );
}

//____ _wheelRollX() ___________________________________________________________

bool WgScrollPanel::_wheelRollX(int distance)
{
	int ofs = m_viewPixOfs.x - (((m_stepSizeX*m_scale) >> WG_SCALE_BINALS) * distance * 3);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsX( ofs );
}

//____ _wheelRollY() ___________________________________________________________

bool WgScrollPanel::_wheelRollY(int distance)
{
	int ofs = m_viewPixOfs.y - (((m_stepSizeY*m_scale) >> WG_SCALE_BINALS) * distance * 3);

	if( ofs < 0 )
		ofs = 0;

	return SetViewPixelOfsY( ofs );
}

//____ SetJumpSizeX() _________________________________________________________

void WgScrollPanel::SetJumpSizeX( float viewFraction )
{
	if( viewFraction < 0.f )
		viewFraction = 0.f;
	if( viewFraction > 1.f )
		viewFraction = 1.f;

	m_jumpSizeX = viewFraction;
}

//____ SetJumpSizeY() _________________________________________________________

void WgScrollPanel::SetJumpSizeY( float viewFraction )
{
	if( viewFraction < 0.f )
		viewFraction = 0.f;
	if( viewFraction > 1.f )
		viewFraction = 1.f;

	m_jumpSizeY = viewFraction;
}

//____ IsHSliderVisible() ____________________________________________________

bool WgScrollPanel::IsHSliderVisible()
{
	return m_elements[XDRAG].m_bVisible;
}

//____ IsVSliderVisible() ____________________________________________________

bool WgScrollPanel::IsVSliderVisible()
{
	return m_elements[YDRAG].m_bVisible;
}


//____ ViewPixelLenX() ________________________________________________________

Uint32 WgScrollPanel::ViewPixelLenX()
{
	return m_elements[WINDOW].m_windowGeo.w;
}

//____ ViewPixelLenY() ________________________________________________________

Uint32 WgScrollPanel::ViewPixelLenY()
{
	return m_elements[WINDOW].m_windowGeo.h;
}

//____ ViewPointLenX() ________________________________________________________

Uint32 WgScrollPanel::ViewPointLenX()
{
	return (m_elements[WINDOW].m_windowGeo.w << WG_SCALE_BINALS) / m_scale;
}

//____ ViewPointLenY() ________________________________________________________

Uint32 WgScrollPanel::ViewPointLenY()
{
	return (m_elements[WINDOW].m_windowGeo.h << WG_SCALE_BINALS) / m_scale;
}


//____ ViewOfsX() _____________________________________________________________

float WgScrollPanel::ViewOfsX()
{
	Uint32 size = m_contentSize.w - ViewPixelLenX();
	if( size <= 0 )
		return 0.f;
	else
		return m_viewPixOfs.x / (float) size;

}

//____ ViewOfsY() _____________________________________________________________

float WgScrollPanel::ViewOfsY()
{
	Uint32 size = m_contentSize.h - ViewPixelLenY();
	if( size <= 0 )
		return 0.f;
	else
		return m_viewPixOfs.y / (float) size;
}

//____ ViewLenX() _____________________________________________________________

float WgScrollPanel::ViewLenX()
{
	if( m_contentSize.w == 0 )
		return 1.0f;

	float len = ViewPixelLenX()/(float)m_contentSize.w;

	if( len > 1.f )
		len = 1.f;

	return len;
}

//____ ViewLenY() _____________________________________________________________

float WgScrollPanel::ViewLenY()
{
	if( m_contentSize.h == 0 )
		return 1.0f;

	float len = ViewPixelLenY()/(float)m_contentSize.h;

	if( len > 1.f )
		len = 1.f;

	return len;
}

//____ SetViewPixelOfs() ______________________________________________________

bool WgScrollPanel::SetViewPixelOfs( int x, int y )
{
	if( x == m_viewPixOfs.x && y == m_viewPixOfs.y )
		return true;

	int		pixLenX = ViewPixelLenX();
	int		pixLenY = ViewPixelLenY();
	bool	bChangedX = false;
	bool	bChangedY = false;

	bool retVal = true;
	if( y < 0 )
	{
		y = 0;
		retVal = false;
	}
	if( y > m_contentSize.h - pixLenY )
	{
		y = std::max(0, m_contentSize.h - pixLenY);
		retVal = false;
	}

	if( x < 0 )
	{
		x = 0;
		retVal = false;
	}
	if( x > m_contentSize.w - pixLenX )
	{
		x = std::max(0, m_contentSize.w - pixLenX);
		retVal = false;
	}

	if( x != m_viewPixOfs.x )
	{
		m_viewPixOfs.x = x;
		bChangedX = true;
	}

	if( y != m_viewPixOfs.y )
	{
		m_viewPixOfs.y = y;
		bChangedY = true;
	}

	float	ofsX = ViewOfsX();
	float	ofsY = ViewOfsY();

	if( bChangedX )
	{
		m_sliderTargets[1]._updateSlider( ViewOfsX(), ViewLenX() );
	}

	if( bChangedY )
	{
		m_sliderTargets[0]._updateSlider( ViewOfsY(), ViewLenY() );
	}

	if( bChangedX || bChangedY )
	{
		m_elements[WINDOW].m_canvasGeo = _genContentCanvasGeo( m_elements[WINDOW].m_windowGeo, m_contentSize, m_contentOrigo, m_viewPixOfs );
		_requestRender( m_elements[WINDOW].m_windowGeo );
	}

	return retVal;
}

//____ SetViewPixelOfsX() _____________________________________________________

bool WgScrollPanel::SetViewPixelOfsX( int x )
{
	return SetViewPixelOfs(x, m_viewPixOfs.y);
}

//____ SetViewPixelOfsY() _____________________________________________________

bool WgScrollPanel::SetViewPixelOfsY( int y )
{
	return SetViewPixelOfs(m_viewPixOfs.x, y);
}


//____ SetViewPointOfs() _____________________________________________________

bool WgScrollPanel::SetViewPointOfs( int x, int y )
{
	return SetViewPixelOfs((x*m_scale)>>WG_SCALE_BINALS, (y*m_scale)>>WG_SCALE_BINALS);
}


//____ SetViewPointOfsX() _____________________________________________________

bool WgScrollPanel::SetViewPointOfsX( int x )
{
	return SetViewPixelOfs((x*m_scale)>>WG_SCALE_BINALS, m_viewPixOfs.y);
}

//____ SetViewPointOfsY() _____________________________________________________

bool WgScrollPanel::SetViewPointOfsY( int y )
{
	return SetViewPixelOfs(m_viewPixOfs.x,(y*m_scale)>>WG_SCALE_BINALS);
}


//____ SetViewOfs() ___________________________________________________________

bool WgScrollPanel::SetViewOfs( float x, float y )
{
	if( x < 0.f )
		x = 0;
	if( x > 1.f )
		x = 1.f;

	if( y < 0.f )
		y = 0;
	if( y > 1.f )
		y = 1.f;

	int	width = m_contentSize.w - ViewPixelLenX();
	int	height = m_contentSize.h - ViewPixelLenY();

	if( width < 0 )
		width = 0;

	if( height < 0 )
		height = 0;


	return SetViewPixelOfs( (Uint32)((width*x)+0.5f), (Uint32)((height*y)+0.5f) );
}

//____ SetViewOfsX() __________________________________________________________

bool WgScrollPanel::SetViewOfsX( float x )
{
	if( x < 0.f )
		x = 0;
	if( x > 1.f )
		x = 1.f;

	int	width = m_contentSize.w - ViewPixelLenX();
	if( width < 0 )
		width = 0;

	return SetViewPixelOfsX( (Uint32)((width*x)+0.5f) );
}

//____ SetViewOfsY() __________________________________________________________

bool WgScrollPanel::SetViewOfsY( float y )
{
	if( y < 0.f )
		y = 0;
	if( y > 1.f )
		y = 1.f;

	int	height = m_contentSize.h - ViewPixelLenY();
	if( height < 0 )
		height = 0;

	return SetViewPixelOfsY( (Uint32)((height*y)+0.5f) );
}

//____ SetContent() ___________________________________________________________

bool WgScrollPanel::SetContent( WgWidget * pContent )
{
	m_elements[WINDOW]._attachWidget(pContent);

	_updateElementGeo( PixelSize() );
	_requestRender( m_elements[WINDOW].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	_requestResize();
	return true;

}

//____ SetHSlider() ________________________________________________________

bool WgScrollPanel::SetHSlider( WgHSlider* pSlider )
{
	// Remove us as target target from current Slider (if we have any)

	if( m_elements[XDRAG].Widget() )
		((WgWidgetSlider*)m_elements[XDRAG].Widget())->SetSliderTarget(0);

	//

	m_elements[XDRAG]._attachWidget(pSlider);

	if( pSlider )
		pSlider->SetSliderTarget( &m_sliderTargets[1] );

	_updateElementGeo( PixelSize() );
	if( pSlider )
		pSlider->SetSlider( ViewOfsX(), ViewLenX() );
	_requestRender( m_elements[XDRAG].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	return true;
}

//____ SetVSlider() ________________________________________________________

bool WgScrollPanel::SetVSlider( WgVSlider* pSlider )
{
	// Remove us as target target from current Slider (if we have any)

	if( m_elements[YDRAG].Widget() )
		((WgWidgetSlider*)m_elements[YDRAG].Widget())->SetSliderTarget(0);

	//

	m_elements[YDRAG]._attachWidget(pSlider);

	//

	if( pSlider )
		pSlider->SetSliderTarget( &m_sliderTargets[0] );

	_updateElementGeo( PixelSize() );
	if(pSlider)
		pSlider->SetSlider( ViewOfsY(), ViewLenY() );
	_requestRender( m_elements[YDRAG].m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	return true;
}

//____ SetShowOverlaySlidersOnlyOnHover() ___________________________________________

void WgScrollPanel::SetShowOverlaySlidersOnlyOnHover( bool bShowOnlyOnHover )
{
	if( bShowOnlyOnHover != m_bShowOverlaysOnlyOnHover )
	{
		m_bShowOverlaysOnlyOnHover = bShowOnlyOnHover;
		_requestRender();
	}
}


//____ ReleaseContent() _______________________________________________________

WgWidget* WgScrollPanel::ReleaseContent()
{
	WgWidget * p = m_elements[WINDOW]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ ReleaseHSlider() ____________________________________________________

WgHSlider* WgScrollPanel::ReleaseHSlider()
{
	WgHSlider * p = (WgHSlider*) m_elements[XDRAG]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ ReleaseVSlider() ____________________________________________________

WgVSlider* WgScrollPanel::ReleaseVSlider()
{
	WgVSlider * p = (WgVSlider*) m_elements[YDRAG]._releaseWidget();
	_updateElementGeo( PixelSize() );
	return p;
}

//____ DeleteChild() __________________________________________________________

bool WgScrollPanel::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_elements[XDRAG].Widget() )
		return SetHSlider(0);
	else if( pWidget == m_elements[YDRAG].Widget() )
		return SetVSlider(0);
	else if( pWidget == m_elements[WINDOW].Widget() )
		return SetContent(0);

	return false;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgScrollPanel::ReleaseChild( WgWidget * pWidget )
{
	if( pWidget == m_elements[XDRAG].Widget() )
		return ReleaseHSlider();
	else if( pWidget == m_elements[YDRAG].Widget() )
		return ReleaseVSlider();
	else if( pWidget == m_elements[WINDOW].Widget() )
		return ReleaseContent();

	return 0;
}

//____ DeleteAllChildren() ______________________________________________________

bool WgScrollPanel::DeleteAllChildren()
{
	SetHSlider(0);
	SetVSlider(0);
	SetContent(0);
	return true;
}

//____ ReleaseAllChildren() _____________________________________________________

bool WgScrollPanel::ReleaseAllChildren()
{
	ReleaseHSlider();
	ReleaseVSlider();
	ReleaseContent();
	return true;
}



//____ SetSliderAutoHide() _________________________________________________

void WgScrollPanel::SetSliderAutoHide( bool bHideHSlider, bool bHideVSlider )
{
	if( bHideHSlider == m_bAutoHideSliderX && bHideVSlider == m_bAutoHideSliderY )
		return;

	bool	bWasVisibleX = IsHSliderVisible();
	bool	bWasVisibleY = IsVSliderVisible();

	m_bAutoHideSliderX = bHideHSlider;
	m_bAutoHideSliderY = bHideVSlider;

	// Force a refresh of our subclass if its geometry has been affected.

	if( IsHSliderVisible() != bWasVisibleX || IsVSliderVisible() != bWasVisibleY )
		_updateElementGeo( PixelSize() );
}

//____ SetSliderPositions() ________________________________________________

void WgScrollPanel::SetSliderPositions( bool bBottom, bool bRight )
{
	if( bBottom == m_bSliderBottom && bRight == m_bSliderRight )
		return;

	m_bSliderBottom	= bBottom;
	m_bSliderRight	= bRight;

	_updateElementGeo( PixelSize() );
}

//____ SetFillerBlocks() ______________________________________________________

void WgScrollPanel::SetFillerBlocks( wg::Skin * pBlocks )
{
	m_pFillerBlocks = pBlocks;
	_requestRender( m_geoFiller );
}

//____ FindWidget() ____________________________________________________________

WgWidget * WgScrollPanel::FindWidget( const WgCoord& pos, WgSearchMode mode )
{
	// Check XDRAG

	WgScrollHook * p = &m_elements[XDRAG];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.contains( pos ) )
	{
		if( mode != WgSearchMode::MarkPolicy || p->m_pWidget->MarkTest( pos - p->m_windowGeo.pos() ) )
			return p->m_pWidget;
	}

	// Check YDRAG

	p = &m_elements[YDRAG];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.contains( pos ) )
	{
		if( mode != WgSearchMode::MarkPolicy || p->m_pWidget->MarkTest( pos - p->m_windowGeo.pos() ) )
			return p->m_pWidget;
	}

	// Check WINDOW

	p = &m_elements[WINDOW];
	if( p->IsVisible() && p->m_pWidget && p->m_windowGeo.contains( pos ) && p->m_canvasGeo.contains( pos ) )
	{
		if( p->m_pWidget->IsContainer() )
		{
			WgWidget * pFound = p->m_pWidget->CastToContainer()->FindWidget( pos - p->m_canvasGeo.pos(), mode );
			if( pFound )
				return pFound;
		}
		else if( mode != WgSearchMode::MarkPolicy || p->m_pWidget->MarkTest( pos - p->m_canvasGeo.pos() ) )
			return p->m_pWidget;
	}

	// Check window background color

	if( m_bgColor.a != 0 && p->m_windowGeo.contains( pos ) )
		return this;


	// Check our little corner square and geometry

	if( mode == WgSearchMode::Geometry || MarkTest( pos ) )
		return this;

	//

	return 0;
}

//____ PreferredPixelSize() ___________________________________________________________

WgSize WgScrollPanel::PreferredPixelSize() const
{
	WgSize sz;

	if (m_elements[WINDOW].m_pWidget)
		sz = m_elements[WINDOW]._paddedPreferredPixelSize(m_scale);

	if (m_elements[XDRAG].m_pWidget && !m_bAutoHideSliderX)
	{
		WgSize scrollbar = m_elements[XDRAG]._paddedPreferredPixelSize(m_scale);
		sz.h += scrollbar.h;
		if (scrollbar.w > sz.w)
			sz.w = scrollbar.w;
	}

	if (m_elements[YDRAG].m_pWidget && !m_bAutoHideSliderY)
	{
		WgSize scrollbar = m_elements[YDRAG]._paddedPreferredPixelSize(m_scale);
		sz.w += scrollbar.w;
		if (scrollbar.h > sz.h)
			sz.h = scrollbar.h;
	}

	return sz;
}

//____ MatchingPixelHeight() __________________________________________________

int WgScrollPanel::MatchingPixelHeight(int pixelWidth) const
{
	int height = 0;

	// If YDRAG needs to be present, we need to remove its width and consider its height.

	if (m_elements[YDRAG].m_pWidget && !m_bAutoHideSliderY)
	{
		WgSize sz = m_elements[YDRAG]._paddedPreferredPixelSize(m_scale);
		height = sz.h;
		pixelWidth -= sz.w;
	}

	int contentWidth = 0;
	if (m_elements[WINDOW].m_pWidget)
	{
		height = std::max(m_elements[WINDOW]._paddedMatchingPixelHeight(pixelWidth,m_scale), height);
		contentWidth = m_elements[WINDOW]._paddedPreferredPixelSize(m_scale).w;
	}

	// If XDRAG needs to be displayed at this width, we need to include its height.

	if (m_elements[XDRAG].m_pWidget && (!m_bAutoHideSliderX || contentWidth > pixelWidth) )
		height += m_elements[XDRAG]._paddedPreferredPixelSize(m_scale).h;


	return height;
}

//____ MatchingPixelWidth() ___________________________________________________

int WgScrollPanel::MatchingPixelWidth(int pixelHeight) const
{
	//TODO: Implement!!!

	return PreferredPixelSize().w;
}



//____ SetBgColor() ____________________________________________________________

void  WgScrollPanel::SetBgColor( WgColor color )
{
	if( color != m_bgColor )
	{
		m_bgColor = color;
		_requestRender( m_elements[WINDOW].m_windowGeo );
	}
}

//____ SetContentOrigo() _________________________________________________

void  WgScrollPanel::SetContentOrigo( WgOrigo origo )
{
	m_contentOrigo = origo;

	WgScrollHook * p = &m_elements[WINDOW];
	WgRect geo = _genContentCanvasGeo( p->m_windowGeo, m_contentSize, m_contentOrigo, m_viewPixOfs );

	if( geo != p->m_canvasGeo )
	{
		p->m_canvasGeo = geo;
		_requestRender( p->m_windowGeo );
	}
}

//____ SetContentSizePolicy() __________________________________________________

void  WgScrollPanel::SetContentSizePolicy( WgSizePolicy widthPolicy, WgSizePolicy heightPolicy )
{
	if( widthPolicy != m_widthPolicy || heightPolicy != m_heightPolicy )
	{
		m_widthPolicy = widthPolicy;
		m_heightPolicy = heightPolicy;

		_updateElementGeo( PixelSize() );
	}
}

//____ ViewGeoPixels() __________________________________________________________

WgRect WgScrollPanel::ViewGeoPixels() const
{
	return m_elements[WINDOW].m_windowGeo;
};

//____ ContentInViewPixels() _____________________________________________________

WgRect WgScrollPanel::ContentInViewPixels() const
{
	return { m_viewPixOfs, m_elements[WINDOW].m_windowGeo.size() };
};

//____ PositionContentInViewPixels() ___________________________________________

bool WgScrollPanel::PositionContentInViewPixels( WgCoord posInContent, WgOrigo viewPosOrigo, WgCoord viewPosOffset )
{
	WgCoord pos = posInContent;

	pos -= WgUtil::OrigoToOfs( viewPosOrigo, m_elements[WINDOW].m_windowGeo.size() );
	pos -= viewPosOffset;

	return SetViewPixelOfs(pos.x,pos.y);
}

//____ SetInstantIntoView() ___________________________________________________

void WgScrollPanel::SetInstantIntoView(bool bInstant)
{
	m_bInstantIntoView = bInstant;
}

//____ ScrollIntoView() ________________________________________________________

void WgScrollPanel::ScrollIntoView( WgWidget * pWidget, const WgBorders& margin, const WgRect& viewSection )
{
	if( !m_bRubberBorder && !m_pScrollIntoViewChild && m_hoverScrollRemainsX == 0 && m_hoverScrollRemainsY == 0 )
		_startReceiveTicks();

	m_pScrollIntoViewChild = pWidget;
	m_scrollIntoViewMargin = margin;
	m_scrollIntoViewSection = viewSection;
	
	m_hoverScrollRemainsX = 0;
	m_hoverScrollRemainsY = 0;
}


//____ SetRubberBorder() ________________________________________________________

void WgScrollPanel::SetRubberBorder( const WgBorders& border )
{
	m_rubberBorder = border;
}

//____ EnableRubberBorder() _____________________________________________________

void WgScrollPanel::EnableRubberBorder( bool bEnabled )
{
	if( bEnabled == m_bRubberBorder )
		return;

	if( bEnabled )
	{
		m_rubberBorderPause = 0;
		_startReceiveTicks();
	}
	else
		_stopReceiveTicks();

	m_bRubberBorder = bEnabled;
}

//____ RubberBorderSnapToPos() ________________________________________________________

void WgScrollPanel::RubberBorderSnapToPos()
{
	WgCoord pos = _calcDesiredWindowPos( WgRect(m_contentSize) - m_rubberBorder.scale(m_scale) );
	SetViewPixelOfs( pos.x, pos.y );
}


//____ SetHoverScrollBorder() ________________________________________________________

bool WgScrollPanel::SetHoverScrollBorder( const WgBorders& border )
{
	if( border.left != border.right || border.top != border.bottom || border.left <0 || border.top <= 0 )
		return false;

	m_hoverScrollBorder = border;
	return true;
}

//____ SetHoverScrollSpeed() ___________________________________________________

bool WgScrollPanel::SetHoverScrollSpeed( int maxPointsPerSecond )
{
	if( m_hoverScrollSpeed < 0 )
		return false;

	m_hoverScrollSpeed = maxPointsPerSecond;
	return true;
}


//____ _calcContentSize() ______________________________________________________

WgSize WgScrollPanel::_calcContentSize( WgSize mySize )
{
	WgWidget * pContent = m_elements[WINDOW].Widget();

	if( !pContent )
		return WgSize(0,0);

	//

	if(!m_bOverlayScrollbars)
	{
		if( !m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
			mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;

		if( !m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
			mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;
	}

	//

	WgSize contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

	if(!m_bOverlayScrollbars)
	{
		if( contentSize.h > mySize.h && m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
		{
			mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;
			contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

			if( contentSize.w > mySize.w && m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
			{
				mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;
				contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );
			}
		}
		else if( contentSize.w > mySize.w && m_bAutoHideSliderX && m_elements[XDRAG].Widget() )
		{
			mySize.h -= m_elements[XDRAG].Widget()->PreferredPixelSize().h;
			contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );

			if( contentSize.h > mySize.h && m_bAutoHideSliderY && m_elements[YDRAG].Widget() )
			{
				mySize.w -= m_elements[YDRAG].Widget()->PreferredPixelSize().w;
				contentSize = m_elements[WINDOW]._sizeFromPolicy( mySize, m_widthPolicy, m_heightPolicy, m_scale );
			}
		}
	}

	return contentSize;
}

//____ _updateElementGeo() _____________________________________________________

void WgScrollPanel::_updateElementGeo( WgSize _mySize )
{
	WgRect myRect = m_pSkin ? _skinContentRect( m_pSkin, _mySize, m_state, m_scale ) : WgRect(_mySize);
	WgSize mySize = myRect.size();

	WgSize newContentSize = _calcContentSize( mySize );

	WgRect	newDragX, newDragY, newWindow, newFiller;
	bool	bShowDragX = false, bShowDragY = false;

	// First get "default geometry" for them all, ignoring overlaps.

	newWindow = myRect;

	if( m_elements[XDRAG].Widget() )
	{
		newDragX.h = m_elements[XDRAG].Widget()->PreferredPixelSize().h;
		newDragX.w = newWindow.w;
		newDragX.x = newWindow.x;

		if( m_bSliderBottom )
			newDragX.y = newWindow.y + newWindow.h - newDragX.h;
		else
			newDragX.y = newWindow.y;
	}

	if( m_elements[YDRAG].Widget() )
	{
		newDragY.w = m_elements[YDRAG].Widget()->PreferredPixelSize().w;
		newDragY.h = newWindow.h;
		newDragY.y = newWindow.y;

		if( m_bSliderRight )
			newDragY.x = newWindow.x + newWindow.w - newDragY.w;
		else
			newDragY.x = newWindow.x;
	}

	// Determine which dragbars we need to show, using basic rules

	if( m_bHovered || !m_bOverlayScrollbars || !m_bShowOverlaysOnlyOnHover || m_bSliderDragInProgress )
	{
		if( m_elements[XDRAG].Widget() && (newContentSize.w > newWindow.w || !m_bAutoHideSliderX) )
			bShowDragX = true;

		if( m_elements[YDRAG].Widget() && (newContentSize.h > newWindow.h || !m_bAutoHideSliderY) )
			bShowDragY = true;

		// See if showing one forces us to show the other

		if( bShowDragY && m_elements[XDRAG].Widget() && newContentSize.w > (newWindow.w - newDragY.w) )
			bShowDragX = true;

		if( bShowDragX && m_elements[YDRAG].Widget() && newContentSize.h > (newWindow.h - newDragY.h) )
			bShowDragY = true;
	}
	


	// Adjust view layout to accomodate visible dragbars

	if( bShowDragY )
	{
		newWindow.w -= newDragY.w;
		if( !m_bSliderRight )
			newWindow.x += newDragY.w;
	}

	if( bShowDragX )
	{
		newWindow.h -= newDragX.h;
		if( !m_bSliderBottom )
			newWindow.y += newDragX.h;
	}

	// If both dragbars are visible we need to avoid overlap
	// and include a filler for the empty square.

	if( bShowDragX && bShowDragY && !m_bOverlayScrollbars )
	{
		newDragX.x = newWindow.x;
		newDragX.w = newWindow.w;

		newDragY.y = newWindow.y;
		newDragY.h = newWindow.h;

		newFiller.x = newDragY.x;
		newFiller.y = newDragX.y;
		newFiller.w = newDragY.w;
		newFiller.h = newDragX.h;
	}

	// Expand view area again if scrollbars are in overlay mode
	// The previous calculations were still needed for correct
	// scrollbar placement.

	if( m_bOverlayScrollbars )
		newWindow = myRect;

	// Determine changes to views position and size over content before
	// we overwrite old values, so we can emit right signals once all is updated.
	// Update m_viewPixOfs at the same time.

	bool	bNewOfsX = false;
	bool	bNewOfsY = false;
	bool	bNewWidth = false;
	bool	bNewHeight = false;

	if( newWindow.h != m_elements[WINDOW].m_windowGeo.h )
		bNewHeight = true;

	if( newWindow.w != m_elements[WINDOW].m_windowGeo.w )
		bNewWidth = true;


	if( m_bAutoScrollX )
	{
		if( m_viewPixOfs.x + m_elements[WINDOW].m_windowGeo.w >= m_contentSize.w && newWindow.w < newContentSize.w )
		{
			bNewOfsX = true;
			m_viewPixOfs.x = newContentSize.w - newWindow.w;
		}
	}

	if( m_bAutoScrollY )
	{
		if( m_viewPixOfs.y + m_elements[WINDOW].m_windowGeo.h >= m_contentSize.h && newWindow.h < newContentSize.h )
		{
			bNewOfsY = true;
			m_viewPixOfs.y = newContentSize.h - newWindow.h;
		}
	}

	if( m_viewPixOfs.x + newWindow.w > newContentSize.w )
	{
		int x = newContentSize.w - newWindow.w;
		if( x < 0 )
			x = 0;
		if( x != m_viewPixOfs.x )
		{
			bNewOfsX = true;
			m_viewPixOfs.x = x;
		}
	}

	if( m_viewPixOfs.y + newWindow.h > newContentSize.h )
	{
		int y = newContentSize.h - newWindow.h;
		if( y < 0 )
			y = 0;
		if( y != m_viewPixOfs.y )
		{
			bNewOfsY = true;
			m_viewPixOfs.y = y;
		}
	}

	// If something visible has changed we need to update element geometry and request render.
	// This is more optimized than it looks like...

	if( newWindow.w < 0 )
		newWindow.w = 0;
	
	if( newWindow.h < 0 )
		newWindow.h = 0;
	
	if( newWindow != m_elements[WINDOW].m_windowGeo || bShowDragX != m_elements[XDRAG].m_bVisible || bShowDragY != m_elements[YDRAG].m_bVisible || newContentSize != m_contentSize )
	{
		m_elements[WINDOW].m_windowGeo = newWindow;
		m_elements[XDRAG].m_windowGeo = newDragX;
		m_elements[YDRAG].m_windowGeo = newDragY;
		m_elements[WINDOW].m_canvasGeo = _genContentCanvasGeo( newWindow, newContentSize, m_contentOrigo, m_viewPixOfs );
		m_elements[XDRAG].m_canvasGeo = newDragX;
		m_elements[YDRAG].m_canvasGeo = newDragY;
		m_elements[XDRAG].m_bVisible = bShowDragX;
		m_elements[YDRAG].m_bVisible = bShowDragY;

		_requestRender();

		// Notify sliders of their new size.

		if( bShowDragX )
			m_elements[XDRAG].Widget()->_onNewSize(newDragX.size());
		if( bShowDragY )
			m_elements[YDRAG].Widget()->_onNewSize(newDragY.size());
	}

	// If contentSize has changed we save changes and set flags

	bool bNewContentWidth = false;
	bool bNewContentHeight = false;

	if( newContentSize.w != m_contentSize.w )
		bNewContentWidth = true;

	if( newContentSize.h != m_contentSize.h )
		bNewContentHeight = true;

	if( bNewContentWidth || bNewContentHeight )
	{
		m_contentSize = newContentSize;

		// Notify content of its new size.

		if( m_elements[WINDOW].Widget() )
			m_elements[WINDOW].Widget()->_onNewSize(newContentSize);
	}

	// Notify sliders of any change to content size, view size or view offset.

	if( bNewOfsX || bNewWidth || bNewContentWidth )
		m_sliderTargets[1]._updateSlider( ViewOfsX(), ViewLenX() );

	if( bNewOfsY || bNewHeight || bNewContentHeight )
		m_sliderTargets[0]._updateSlider( ViewOfsY(), ViewLenY() );

/*
	// Send signals if views size or position over content has changed
	// or contents size has changed.

	if( bNewOfsX || bNewContentWidth )
		Emit( ViewPosX(), ViewOfsX() );
	if( bNewOfsX )
		Emit( ViewPosPixelX(), m_viewPixOfs.x );

	if( bNewOfsY || bNewContentHeight )
		Emit( ViewPosY(), ViewOfsY() );
	if( bNewOfsY )
		Emit( ViewPosPixelY(), m_viewPixOfs.y );

	if( bNewWidth || bNewContentWidth )
		Emit( ViewSizeX(), ViewLenX() );
	if( bNewWidth )
		Emit( ViewSizePixelX(), ViewPixelLenX() );

	if( bNewHeight || bNewContentHeight )
		Emit( ViewSizeY(), ViewLenY() );
	if( bNewHeight )
		Emit( ViewSizePixelY(), ViewPixelLenY() );

	if( bNewOfsX || bNewWidth || bNewContentWidth )
		Emit( ViewPosSizeX(), ViewOfsX(), ViewLenX() );
	if( bNewOfsX || bNewWidth )
		Emit( ViewPosSizePixelX(), m_viewPixOfs.x, ViewPixelLenX() );

	if( bNewOfsY || bNewHeight || bNewContentHeight )
		Emit( ViewPosSizeY(), ViewOfsY(), ViewLenY() );
	if( bNewOfsY || bNewHeight )
		Emit( ViewPosSizePixelY(), m_viewPixOfs.y, ViewPixelLenY() );

	if( bNewOfsX || bNewOfsY || bNewContentHeight || bNewContentWidth )
		Emit( ViewPos(), ViewOfsX(), ViewOfsY() );
	if( bNewOfsX || bNewOfsY )
		Emit( ViewPosPixel(), m_viewPixOfs.x, m_viewPixOfs.y );

	if( bNewWidth || bNewHeight || bNewContentHeight || bNewContentWidth )
		Emit( ViewSize(), ViewLenX(), ViewLenY() );
	if( bNewWidth || bNewHeight )
		Emit( ViewSizePixel(), ViewPixelLenX(), ViewPixelLenY() );
*/
}

//____ _genContentCanvasGeo() __________________________________________________

WgRect WgScrollPanel::_genContentCanvasGeo( const WgRect& window, WgSize contentSize, WgOrigo origo, WgCoord viewOfs )
{
	WgRect	out( window.pos() - viewOfs, contentSize);

	if( window.w > contentSize.w )
	{
		WgRect r = WgUtil::OrigoToRect( origo, WgSize(window.w,1), WgSize(contentSize.w,1) );
		out.x = window.x + r.x;
	}

	if( window.h > contentSize.h )
	{
		WgRect r = WgUtil::OrigoToRect( origo, WgSize(1,window.h), WgSize(1,contentSize.h) );
		out.y = window.y + r.y;
	}

	return out;
}

//____ _onNewSize() ____________________________________________________________

void WgScrollPanel::_onNewSize( const WgSize& size )
{
	_updateElementGeo( size );
}

//____ _calcDesiredWindowPos() __________________________________________________

WgCoord WgScrollPanel::_calcDesiredWindowPos( const WgRect& content)
{
	// Get current window and content (canvas minus rubber borders) geometries.

	WgRect  window = { m_viewPixOfs, m_elements[WINDOW].m_windowGeo.size() };

	// Calculate desiredWindowPos

	WgCoord desiredWindowPos = window.pos();

	if( content.w < window.w )
	{
		WgRect r = WgUtil::OrigoToRect( m_contentOrigo, WgSize(window.w,1), WgSize(content.w,1) );
		desiredWindowPos.x = content.x - r.x;
	}
	else
	{
		if( window.x < content.x )
			desiredWindowPos.x = content.x;

		if( window.x + window.w > content.x + content.w  )
			desiredWindowPos.x = content.x + content.w - window.w;
	}

	if( content.h < window.h )
	{
		WgRect r = WgUtil::OrigoToRect( m_contentOrigo, WgSize(1,window.h), WgSize(1,content.h) );
		desiredWindowPos.y = content.y - r.y;
	}
	else
	{
		if( window.y < content.y )
			desiredWindowPos.y = content.y;

		if( window.y + window.h > content.y + content.h  )
			desiredWindowPos.y = content.y + content.h - window.h;
	}

	return desiredWindowPos;
}

//____ _onEvent() ______________________________________________________________

void WgScrollPanel::_onEvent( const WgEvent::Event * _pEvent, WgEventHandler * pHandler )
{
	bool    bCallSuper = false;

	switch( _pEvent->Type() )
	{
		case WG_EVENT_MOUSE_ENTER:
			bCallSuper = true;
			m_bHovered = true;
			if( m_bShowOverlaysOnlyOnHover && m_bOverlayScrollbars )
			{
				_updateElementGeo(PixelSize());
			}
			break;

		case WG_EVENT_MOUSE_LEAVE:
			bCallSuper = false;
			m_bHovered = false;
			if( m_bShowOverlaysOnlyOnHover && m_bOverlayScrollbars )
			{
				_updateElementGeo(PixelSize());
			}
			break;

			
		case WG_EVENT_TICK:
		{
			int ms = static_cast<const WgEvent::Tick*>(_pEvent)->Millisec();

			if( m_hoverScrollX != 0 || m_hoverScrollY != 0 )
			{
				int x = m_hoverScrollX * ms + m_hoverScrollRemainsX;
				int y = m_hoverScrollY * ms + m_hoverScrollRemainsY;

				m_hoverScrollRemainsX = x % 1000;
				m_hoverScrollRemainsY = y % 1000;

				SetViewPixelOfs( m_viewPixOfs.x + x/1000, m_viewPixOfs.y + y/1000 );
			}
			else
			{
				// Calculate distance to scroll for rubber border.

				WgCoord rubberBorderScroll;

				if( m_bRubberBorder && !m_bRubberBorderHold )
				{
					if( m_rubberBorderPause > 0 )
						m_rubberBorderPause = std::max(0,m_rubberBorderPause-ms);
					else
						rubberBorderScroll = _calcDesiredWindowPos( WgRect(m_contentSize) - m_rubberBorder.scale(m_scale) ) - m_viewPixOfs;
				}

				// Calculate distance to scroll for scroll into view.

				WgCoord intoViewScroll;

				if( m_pScrollIntoViewChild )
				{
					WgWidget * p = m_pScrollIntoViewChild.GetRealPtr();

					WgRect inside = p->PixelGeo() + (m_scrollIntoViewMargin*m_scale/WG_SCALE_BASE);

					p = p->Parent();
					while( p && p != this )
					{
						if( p->Parent() != this )
							inside += p->PixelPos();
						p = p->Parent();
					}

					if( p == this )
					{
						WgRect window = { m_viewPixOfs, m_elements[WINDOW].m_windowGeo.size() };

						if( !m_scrollIntoViewSection.isEmpty() )
							window = m_scrollIntoViewSection*m_scale/WG_SCALE_BASE + m_viewPixOfs;

						WgCoord currPos = window.pos();

						int diffLeft = inside.x - window.x;
						int diffRight = inside.right() - window.right();
						int diffTop = inside.y - window.y;
						int diffBottom = inside.bottom() - window.bottom();

						if( diffLeft > 0 && diffRight > 0  )
							window.x += std::min(diffLeft, diffRight);
						else if( diffLeft < 0 && diffRight < 0 )
							window.x += std::max(diffLeft, diffRight);

						if( diffTop > 0 && diffBottom > 0 )
							window.y += std::min(diffTop, diffBottom);
						else if( diffTop < 0 && diffBottom < 0 )
							window.y += std::max(diffTop, diffBottom);

						intoViewScroll = window.pos() - currPos;
					}

					if( intoViewScroll == WgCoord(0,0) )
					{
						m_pScrollIntoViewChild = nullptr;

						if( !m_bRubberBorder && m_hoverScrollRemainsX == 0 && m_hoverScrollRemainsY == 0 )
							_stopReceiveTicks();
					}
				}

				// Combine distances (scroll the longest distance)

				WgCoord distance = {    abs(rubberBorderScroll.x) > abs(intoViewScroll.x ) ? rubberBorderScroll.x : intoViewScroll.x,
										abs(rubberBorderScroll.y) > abs(intoViewScroll.y ) ? rubberBorderScroll.y : intoViewScroll.y };

				// Scale the distance and apply to current pos.

				if( !(m_bInstantIntoView && abs(intoViewScroll.x ) > abs(rubberBorderScroll.x)) )
				{
					if( distance.x < 0 )
						distance.x = distance.x / 4 -1;

					if( distance.x > 0 )
						distance.x = distance.x / 4 + 1;
				}

			   if( !(m_bInstantIntoView && abs(intoViewScroll.y ) > abs(rubberBorderScroll.y)) )
				  {
					if( distance.y < 0 )
						distance.y = distance.y / 4 -1;

					if( distance.y > 0 )
						distance.y = distance.y / 4 + 1;
				}

				if( distance.x != 0 || distance.y != 0 )
					SetViewPixelOfs( m_viewPixOfs.x + distance.x, m_viewPixOfs.y + distance.y );
			}

			break;
		}

		case WG_EVENT_MOUSEBUTTON_PRESS:
		{
			m_bRubberBorderHold = true;
			WgWidget::_onEvent(_pEvent,pHandler);

			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			m_bRubberBorderHold = false;
			WgWidget::_onEvent(_pEvent,pHandler);

			break;
		}


		case WG_EVENT_MOUSEWHEEL_ROLL:
		{
            if(m_bDisableScroll == true)
                return;
            const WgEvent::MouseWheelRoll *pEvent = static_cast<const WgEvent::MouseWheelRoll *>(_pEvent);

            if( m_elements[WINDOW].m_windowGeo.contains(pEvent->PointerPixelPos()) )
			{
				m_rubberBorderPause = 300;
				int wheel = pEvent->Wheel();

                auto axis = wheel >= 0 && wheel < 5 ? m_mouseWheelAxis[wheel] : wg::Axis::Undefined;
                int inverter = pEvent->InvertScroll() ? -1 : 1;

                if(axis == wg::Axis::X)
                    _wheelRollX( pEvent->Distance()*inverter );
                else if(axis == wg::Axis::Y)
                    _wheelRollY( pEvent->Distance()*inverter );
				else
					bCallSuper = true;
			}
			else
				bCallSuper = true;
			break;
		}

		case WG_EVENT_DROPHOVER_ENTER:
		case WG_EVENT_DROPHOVER_MOVE:
		{
			if( !m_hoverScrollBorder.isEmpty() )
			{
				WgCoord pos = _pEvent->PointerPixelPos();

//				if( !m_elements[WINDOW].m_windowGeo.contains(pos) )
//					break;

				WgBorders border = m_hoverScrollBorder.scale(m_scale);
				WgRect center = m_elements[WINDOW].m_windowGeo - border;

				int dirX = 0, dirY = 0;

				if( pos.x < center.x )
					dirX = pos.x - center.x;
				else if( pos.x > center.x + center.w )
					dirX = pos.x - (center.x + center.w);

				if( pos.y < center.y )
					dirY = pos.y - center.y;
				else if( pos.y > center.y + center.h )
					dirY = pos.y - (center.y + center.h);

                int maxScrollSpeed = m_hoverScrollSpeed * m_scale / WG_SCALE_BASE;

				dirX = dirX * maxScrollSpeed / border.left;
				dirY = dirY * maxScrollSpeed / border.top;

                wg::limit( dirX, -maxScrollSpeed, maxScrollSpeed );
                wg::limit( dirY, -maxScrollSpeed, maxScrollSpeed );

				if( dirX == 0 && dirY == 0 )
				{
					if( !m_bRubberBorder && !m_pScrollIntoViewChild )
					{
						_stopReceiveTicks();
						m_hoverScrollRemainsX = 0;
						m_hoverScrollRemainsY = 0;
					}
				}
				else if( m_hoverScrollX == 0 && m_hoverScrollY == 0 )
				{
						if( !m_bRubberBorder && !m_pScrollIntoViewChild )
							_startReceiveTicks();
				}

				m_hoverScrollX = dirX;
				m_hoverScrollY = dirY;
			}

			break;
		}

		case WG_EVENT_DROPHOVER_LEAVE:
		{
			m_hoverScrollX = 0;
			m_hoverScrollY = 0;
			m_hoverScrollRemainsX = 0;
			m_hoverScrollRemainsY = 0;

			if( !m_bRubberBorder && !m_pScrollIntoViewChild )
				_stopReceiveTicks();
			break;
		}

		default:
			bCallSuper = true;
		break;
	}

	if( bCallSuper )
		WgWidget::_onEvent(_pEvent,pHandler);

}

//____ _renderPatches() ________________________________________________________

void WgScrollPanel::_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches )
{

	// We start by eliminating dirt outside our geometry

	WgPatches 	patches( _pPatches->size() );								// TODO: Optimize by pre-allocating?

	for( const WgRect * pRect = _pPatches->begin() ; pRect != _pPatches->end() ; pRect++ )
	{
		if( _window.isOverlapping( *pRect ) )
			patches.push( WgRect::overlap(*pRect,_window) );
	}

	//

	if( patches.isEmpty() )
		return;
	
	WgRect	dirtBounds = patches.bounds();

	int bytesToRelease = _convertAndPushClipList( pDevice, patches.size(), patches.begin() );

	// Render skin

	if( m_pSkin )
		_renderSkin( m_pSkin, pDevice, m_state, _canvas, m_scale);                // TODO: Optimize. Clip away window geometry if window background color is transparent.

	// Render Window background color

	if( m_bgColor.a != 0 )
	{
		WgRect window = m_elements[WINDOW].m_windowGeo + _canvas.pos();
		pDevice->fill( window*64, m_bgColor );
	}

	//

	if( m_elements[WINDOW].Widget() )
	{
		WgRect canvas = m_elements[WINDOW].m_canvasGeo + _canvas.pos();
		WgRect window = WgRect::overlap( canvas, m_elements[WINDOW].m_windowGeo + _canvas.pos() );

		// Use intersection in case canvas is smaller than window.

		if( window.isOverlapping(dirtBounds) )
		{
			// We need to eliminate dirt outside our view window

			WgPatches     winPatches( patches.size() );

			for( const WgRect * pRect = patches.begin() ; pRect != patches.end() ; pRect++ )
			{
				if( window.isOverlapping( *pRect ) )
					winPatches.push( WgRect::overlap(*pRect,window) );
			}

			if( !winPatches.isEmpty() )
				m_elements[WINDOW].Widget()->_renderPatches( pDevice, canvas, window, &winPatches );
		}
	}

	if( m_elements[XDRAG].m_bVisible )
	{
		WgRect canvas = m_elements[XDRAG].m_windowGeo + _canvas.pos();
		if( canvas.isOverlapping(dirtBounds) )
			m_elements[XDRAG].Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
	}

	if( m_elements[YDRAG].m_bVisible )
	{
		WgRect canvas = m_elements[YDRAG].m_windowGeo + _canvas.pos();
		if( canvas.isOverlapping(dirtBounds) )
			m_elements[YDRAG].Widget()->_renderPatches( pDevice, canvas, canvas, &patches );
	}

	//

	if( m_pFillerBlocks && m_geoFiller.w != 0 && m_geoFiller.h != 0 )
	{
		WgMode mode = m_bEnabled?WG_MODE_NORMAL:WG_MODE_DISABLED;
		WgRect canvas = m_geoFiller + _canvas.pos();

		_renderSkin(m_pFillerBlocks, pDevice, WgUtil::ModeToState(mode), canvas, m_scale);
	}

	//

	_popAndReleaseClipList( pDevice, bytesToRelease );
}

//____ _onCollectPatches() _______________________________________________________

void WgScrollPanel::_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip )
{
	container.add( WgRect::overlap(geo,clip) );
}

//____ _onMaskPatches() __________________________________________________________

void WgScrollPanel::_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode )
{
	switch( m_maskOp )
	{
		case WgMaskOp::Recurse:
		{
			// Mask against view

			WgScrollHook * p = &m_elements[WINDOW];

			if( m_bgColor.a == 255 )
				patches.sub( WgRect::overlap( p->m_windowGeo + geo.pos(), clip) );
			else if( p->Widget() )
				p->Widget()->_onMaskPatches( patches, p->m_canvasGeo + geo.pos(), WgRect::overlap(p->m_windowGeo + geo.pos(), clip), blendMode );

			// Mask against dragbars

			p = &m_elements[XDRAG];
			if( p->IsVisible() )
				p->Widget()->_onMaskPatches( patches, p->m_windowGeo + geo.pos(), clip, blendMode );

			p = &m_elements[YDRAG];
			if( p->IsVisible() )
				p->Widget()->_onMaskPatches( patches, p->m_windowGeo + geo.pos(), clip, blendMode );

			// Maska against corner piece

			if( !m_geoFiller.isEmpty() && m_pFillerBlocks && m_pFillerBlocks->isOpaque() )
				patches.sub( WgRect::overlap(m_geoFiller + geo.pos(), clip) );

			break;
		}
		case WgMaskOp::Skip:
			break;
		case WgMaskOp::Mask:
			patches.sub( WgRect::overlap(geo,clip) );
			break;
	}
}

//____ _onAlphaTest() ___________________________________________________________

bool WgScrollPanel::_onAlphaTest( const WgCoord& ofs )
{
	if( m_pFillerBlocks && m_geoFiller.contains( ofs ) )
	{
		WgMode mode = m_bEnabled?WG_MODE_NORMAL:WG_MODE_DISABLED;

		return _markTestSkin(m_pFillerBlocks, ofs, m_geoFiller, WgUtil::ModeToState(mode), m_markOpacity, m_scale);
	}
	else
		return false;
}

//____ _onCloneContent() _______________________________________________________

void WgScrollPanel::_onCloneContent( const WgWidget * _pOrg )
{
	WgScrollPanel* pOrg = (WgScrollPanel*)_pOrg;

	WgPanel::_onCloneContent(pOrg);


	m_contentSize = pOrg->m_contentSize;

	m_stepSizeX = pOrg->m_stepSizeX;
	m_stepSizeY = pOrg->m_stepSizeY;
	m_jumpSizeX = pOrg->m_jumpSizeX;
	m_jumpSizeY = pOrg->m_jumpSizeY;

	m_viewPixOfs = pOrg->m_viewPixOfs;

	m_bAutoHideSliderX = pOrg->m_bAutoHideSliderX;
	m_bAutoHideSliderY = pOrg->m_bAutoHideSliderY;

	m_bSliderBottom = pOrg->m_bSliderBottom;
	m_bSliderRight = pOrg->m_bSliderRight;

	m_bAutoScrollX = pOrg->m_bAutoScrollX;
	m_bAutoScrollY = pOrg->m_bAutoScrollY;

	m_bgColor = pOrg->m_bgColor;
	m_widthPolicy = pOrg->m_widthPolicy;
	m_heightPolicy = pOrg->m_heightPolicy;
	m_contentOrigo = pOrg->m_contentOrigo;

	m_pFillerBlocks = pOrg->m_pFillerBlocks;
	m_geoFiller = pOrg->m_geoFiller;

	//

	m_elements[WINDOW].m_windowGeo = pOrg->m_elements[WINDOW].m_windowGeo;
	// Canvas geo for WINDOW should remain as it is since we don't have any content.

	//

	m_elements[XDRAG].m_windowGeo = pOrg->m_elements[XDRAG].m_windowGeo;
	m_elements[XDRAG].m_canvasGeo = pOrg->m_elements[XDRAG].m_canvasGeo;
	m_elements[XDRAG].m_bVisible = pOrg->m_elements[XDRAG].m_bVisible;
	if( pOrg->m_elements[XDRAG].m_pWidget )
	{
		WgHSlider * pSlider = (WgHSlider*) pOrg->m_elements[XDRAG].m_pWidget->NewOfMyType();
		pSlider->CloneContent( pOrg->m_elements[XDRAG].m_pWidget );
		pSlider->SetSliderTarget(&m_sliderTargets[1]);
		m_elements[XDRAG]._attachWidget(pSlider);
	}

	//

	m_elements[YDRAG].m_windowGeo = pOrg->m_elements[YDRAG].m_windowGeo;
	m_elements[YDRAG].m_canvasGeo = pOrg->m_elements[YDRAG].m_canvasGeo;
	m_elements[YDRAG].m_bVisible = pOrg->m_elements[YDRAG].m_bVisible;
	if( pOrg->m_elements[YDRAG].m_pWidget )
	{
		WgVSlider * pSlider = (WgVSlider*) pOrg->m_elements[YDRAG].m_pWidget->NewOfMyType();
		pSlider->CloneContent( pOrg->m_elements[YDRAG].m_pWidget );
		pSlider->SetSliderTarget(&m_sliderTargets[0]);
		m_elements[YDRAG]._attachWidget(pSlider);
	}

	//
}


//______________________________________________________________________________
bool WgScrollPanel::SetMouseWheelAxis(int wheelNb, wg::Axis axis)
{
    if( wheelNb < 0 || wheelNb > 4 )
        return false;

    m_mouseWheelAxis[wheelNb] = axis;
    return true;
}

//_____________________________________________________________________________
bool WgScrollPanel::SetAutoscroll( bool bAutoX, bool bAutoY )
{
	m_bAutoScrollX = bAutoX;
	m_bAutoScrollY = bAutoY;
	return true;
}

//_____________________________________________________________________________
void WgScrollPanel::_inViewRequested( WgHook * pChild )
{
	// Our only child is always in view as much as possible, so just pass it on like any
	// normal container.

	WgRect geo = pChild->PixelGeo();
	_requestInView( geo, geo );
}

void WgScrollPanel::_inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	WgRect window = { m_viewPixOfs, m_elements[WINDOW].m_windowGeo.size() };

	for (int i = 0; i < 2; i++)
	{
		WgRect inside = i == 0 ? niceToHaveArea : mustHaveArea;

		int diffLeft = inside.x - window.x;
		int diffRight = inside.right() - window.right();
		int diffTop = inside.y - window.y;
		int diffBottom = inside.bottom() - window.bottom();

		if( diffLeft > 0 && diffRight > 0  )
			window.x += std::min(diffLeft, diffRight);
		else if( diffLeft < 0 && diffRight < 0 )
			window.x += std::max(diffLeft, diffRight);

		if( diffTop > 0 && diffBottom > 0 )
			window.y += std::min(diffTop, diffBottom);
		else if( diffTop < 0 && diffBottom < 0 )
			window.y += std::max(diffTop, diffBottom);
	}

	if( window.pos() != m_viewPixOfs )
		SetViewPixelOfs( window.x, window.y );

	// Forward to any outer ScrollPanel

	WgRect newMustHaveArea = WgRect::overlap( mustHaveArea - m_viewPixOfs + m_elements[WINDOW].m_windowGeo.pos(), m_elements[WINDOW].m_windowGeo );
	WgRect newNiceToHaveArea = WgRect::overlap( niceToHaveArea - m_viewPixOfs + m_elements[WINDOW].m_windowGeo.pos(), m_elements[WINDOW].m_windowGeo );

	_requestInView( newMustHaveArea, newNiceToHaveArea );
}

//_____________________________________________________________________________
WgHook*	WgScrollPanel::_firstHook() const
{
	for( int i = 0 ; i < 3 ; i++ )
		if( m_elements[i].m_pWidget )
			return const_cast<WgScrollHook*>(&m_elements[i]);

	return 0;
}

//_____________________________________________________________________________
WgHook*	WgScrollPanel::_lastHook() const
{
	for( int i = 2 ; i >= 0 ; i++ )
		if( m_elements[i].m_pWidget )
			return const_cast<WgScrollHook*>(&m_elements[i]);

	return 0;
}


//_____________________________________________________________________________
WgHook * WgScrollPanel::_firstHookWithGeo( WgRect& geo ) const
{
	for( int i = 0 ; i < 3 ; i++ )
		if( m_elements[i].m_pWidget )
		{
			geo = m_elements[i].m_canvasGeo;
			return const_cast<WgScrollHook*>(&m_elements[i]);
		}

	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	const WgScrollHook * pLast = &m_elements[2];
	WgScrollHook * p = (WgScrollHook*) pHook;


	while( p != pLast )
	{
		p++;
		if( p->m_pWidget )
		{
			geo = p->m_canvasGeo;
			return const_cast<WgScrollHook*>(p);
		}
	}
	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_lastHookWithGeo( WgRect& geo ) const
{
	for( int i = 2 ; i >= 0 ; i++ )
		if( m_elements[i].m_pWidget )
		{
			geo = m_elements[i].m_canvasGeo;
			return const_cast<WgScrollHook*>(&m_elements[i]);
		}

	return 0;
}

//_____________________________________________________________________________
WgHook * WgScrollPanel::_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const
{
	const WgScrollHook * pFirst = &m_elements[0];
	WgScrollHook * p = (WgScrollHook*) pHook;


	while( p != pFirst )
	{
		p--;
		if( p->m_pWidget )
		{
			geo = p->m_canvasGeo;
			return const_cast<WgScrollHook*>(p);
		}
	}
	return 0;
}

//____ WgScrollHook::Destructor ___________________________________________________

WgScrollHook::~WgScrollHook()
{
}

//____ WgScrollHook::Type _______________________________________________________

const char * WgScrollHook::Type( void ) const
{
	return ClassType();
}

//____ WgScrollHook::ClassType __________________________________________________

const char * WgScrollHook::ClassType()
{
	return c_hookType;
}


//____ WgScrollHook::PixelPos() ________________________________________________________

WgCoord WgScrollHook::PixelPos() const
{
	return m_canvasGeo.pos();
}

//____ WgScrollHook::PixelSize() _______________________________________________________

WgSize WgScrollHook::PixelSize() const
{
	return m_canvasGeo.size();
}

//____ WgScrollHook::PixelGeo() ________________________________________________________

WgRect WgScrollHook::PixelGeo() const
{
	return m_canvasGeo;
}

//____ WgScrollHook::ScreenPixelPos() __________________________________________________

WgCoord WgScrollHook::ScreenPixelPos() const
{
	return m_pView->ScreenPixelPos() + m_canvasGeo.pos();
}

//____ WgScrollHook::ScreenPixelGeo() __________________________________________________

WgRect WgScrollHook::ScreenPixelGeo() const
{
	return m_canvasGeo + m_pView->ScreenPixelPos();
}


//____ WgScrollHook::_requestRender() ______________________________________________

void WgScrollHook::_requestRender()
{
	if( m_bVisible )
		m_pView->_requestRender( m_windowGeo );
}

void WgScrollHook::_requestRender( const WgRect& rect )
{
	if( m_bVisible )
	{
		WgRect r = WgRect::overlap( m_windowGeo, rect + m_canvasGeo.pos() );

		if( !r.isEmpty() )
			m_pView->_requestRender( r );
	}
}

//____ WgScrollHook::_requestResize() ______________________________________________

void WgScrollHook::_requestResize()
{
	m_pView->_updateElementGeo( m_pView->PixelSize() );
	m_pView->_requestRender( m_windowGeo );		// If geometry is same as the old one, we need to request render ourselves.
	m_pView->_requestResize();
}

//____ WgScrollHook::_prevHook() ___________________________________________________

WgHook * WgScrollHook::_prevHook() const
{
	const WgScrollHook * pFirst = &m_pView->m_elements[0];
	WgScrollHook * p 		= const_cast<WgScrollHook*>(this);

	while( p != pFirst )
	{
		p--;
		if( p->m_pWidget )
			return p;
	}
	return 0;
}

//____ WgScrollHook::_nextHook() ___________________________________________________

WgHook * WgScrollHook::_nextHook() const
{
	WgScrollHook * pLast = &m_pView->m_elements[2];
	WgScrollHook * p 		= const_cast<WgScrollHook*>(this);

	while( p != pLast )
	{
		p++;
		if( p->m_pWidget )
			return p;
	}
	return 0;
}

//____ WgScrollHook::_parent() ___________________________________________________

WgContainer * WgScrollHook::_parent() const
{
	return m_pView;
}

//____ SliderTarget methods ____________________________________________________

float WgScrollPanel::SliderTarget::_stepFwd()
{
	if( m_bHorizontal )
	{
		m_pParent->StepRight();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->StepDown();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_stepBwd()
{
	if( m_bHorizontal )
	{
		m_pParent->StepLeft();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->StepUp();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_jumpFwd()
{
	if( m_bHorizontal )
	{
		m_pParent->JumpRight();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->JumpDown();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_jumpBwd()
{
	if( m_bHorizontal )
	{
		m_pParent->JumpLeft();
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->JumpUp();
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_wheelRolled( int distance )
{
	if( m_bHorizontal )
	{
		m_pParent->_wheelRollX(distance);
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->_wheelRollY(distance);
		return m_pParent->ViewOfsY();
	}
}

float WgScrollPanel::SliderTarget::_setPosition( float fraction )
{
	if( m_bHorizontal )
	{
		m_pParent->SetViewOfsX(fraction);
		return m_pParent->ViewOfsX();
	}
	else
	{
		m_pParent->SetViewOfsY(fraction);
		return m_pParent->ViewOfsY();
	}
}

WgWidget* WgScrollPanel::SliderTarget::_getWidget()
{
	return m_pParent;
}

float WgScrollPanel::SliderTarget::_getSliderPosition()
{
	if( m_bHorizontal )
		return m_pParent->ViewOfsX();
	else
		return m_pParent->ViewOfsY();
}

float WgScrollPanel::SliderTarget::_getSliderSize()
{
	if( m_bHorizontal )
		return m_pParent->ViewLenX();
	else
		return m_pParent->ViewLenY();
}

void WgScrollPanel::SliderTarget::_sliderDragStart()
{
	m_pParent->m_bSliderDragInProgress = true;
//	m_pParent->_updateElementGeo(m_pParent->PixelSize());
//	m_pParent->_requestRender();
}

void WgScrollPanel::SliderTarget::_sliderDragEnd()
{
	m_pParent->m_bSliderDragInProgress = false;
	
	if( m_pParent->m_bShowOverlaysOnlyOnHover && m_pParent->m_bOverlayScrollbars )
		m_pParent->_updateElementGeo(m_pParent->PixelSize());
}


