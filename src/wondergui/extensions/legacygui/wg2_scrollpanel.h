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
#ifndef	WG2_SCROLLPANEL_DOT_H
#define	WG2_SCROLLPANEL_DOT_H


#ifndef WG2_PANEL_DOT_H
#	include <wg2_panel.h>
#endif

#ifndef	WG2_WIDGET_SLIDER_DOT_H
#	include <wg2_slider.h>
#endif

#ifndef WG2_SLIDERTARGET_DOT_H
#	include <wg2_slidertarget.h>
#endif

#include <wg2_color.h>

#include <vector>

class WgScrollPanel;

//____ WgScrollHook _____________________________________________________________

class WgScrollHook : public WgPanelHook
{
	friend class WgScrollPanel;
public:
	const char *Type( void ) const;
	static const char * ClassType();

	WgCoord			PixelPos() const;
	WgSize			PixelSize() const;
	WgRect			PixelGeo() const;
	WgCoord			ScreenPixelPos() const;
	WgRect			ScreenPixelGeo() const;

	WgHook * 		Prev() const { return _prevHook(); }
	WgHook * 		Next() const { return _nextHook(); }


	bool			SetVisible( bool bVisible ) { return false; }		// This widget handles hide/show of children according to its own rules.
	WgScrollPanel * 	Parent() const { return m_pView; }

protected:
	WgScrollHook() : m_pView(0) {};				// So we can make them members and then make placement new...
	~WgScrollHook();
	void			_setParent( WgScrollPanel * pParent ) { m_pView = pParent; }

	void			_requestRender();
	void			_requestRender( const WgRect& rect );
	void			_requestResize();

	WgHook *		_prevHook() const;
	WgHook *		_nextHook() const;
	WgContainer *	_parent() const;

	WgScrollPanel *	m_pView;
	WgRect			m_windowGeo;		// Geometry of Widgets window inside parent.
	WgRect			m_canvasGeo;		// Geometry of Widgets canvas. Same as m_windowGeo for sliders.
};



//____ WgScrollPanel ________________________________________________________

class WgScrollPanel : public WgPanel
{
	friend class WgScrollHook;
public:
	WgScrollPanel();
	virtual ~WgScrollPanel();
	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgScrollPanel(); };


	//____ Methods _______________________________________________________________

	bool		StepUp();
	bool		StepDown();
	bool		StepLeft();
	bool		StepRight();

	bool		JumpUp();
	bool		JumpDown();
	bool		JumpLeft();
	bool		JumpRight();

	void		SetStepSize( int pixX, int pixY ) { m_stepSizeX = pixX; m_stepSizeY = pixY; };
	void		SetStepSizeX( int pixels ) { m_stepSizeX = pixels; };
	void		SetStepSizeY( int pixels ) { m_stepSizeY = pixels; };

	Uint32 		StepSizeX() { return m_stepSizeX; };
	Uint32 		StepSizeY() { return m_stepSizeY; };

	void		SetJumpSize( float viewFractionX, float viewFractionY ) { SetJumpSizeX(viewFractionX); SetJumpSizeY(viewFractionY); };
	void		SetJumpSizeX( float viewFraction );
	void		SetJumpSizeY( float viewFraction );
	float		JumpSizeX( ) const { return m_jumpSizeX; }
	float		JumpSizeY( ) const { return m_jumpSizeY; }

	int			ContentPixelWidth() { return m_contentSize.w; };
	int			ContentPixelHeight() { return m_contentSize.h; };
	WgSize		ContentPixelSize() { return m_contentSize; };

	int			ContentPointWidth() { return (m_contentSize.w << WG_SCALE_BINALS) / m_scale; };
	int			ContentPointHeight() { return (m_contentSize.h << WG_SCALE_BINALS) / m_scale; };
	WgSize		ContentPointSize() { return (m_contentSize*WG_SCALE_BASE)/ m_scale; };

	void		SetBgColor( WgColor color );
	WgColor		BgColor() const { return m_bgColor; }

	void		SetContentOrigo( WgOrigo origo );
	WgOrigo ContentOrigo() const { return m_contentOrigo; }

	void		SetContentSizePolicy( WgSizePolicy widthPolicy, WgSizePolicy heightPolicy );
	WgSizePolicy	ContentWidthPolicy() const { return m_widthPolicy; }
	WgSizePolicy	ContentHeightPolicy() const { return m_heightPolicy; }


	Uint32		ViewPixelOfsX() { return m_viewPixOfs.x; };
	Uint32		ViewPixelOfsY() { return m_viewPixOfs.y; };
	WgCoord		ViewPixelOfs() { return m_viewPixOfs; };

	Uint32		ViewPointOfsX() { return (m_viewPixOfs.x << WG_SCALE_BINALS) / m_scale; };
	Uint32		ViewPointOfsY() { return (m_viewPixOfs.y << WG_SCALE_BINALS) / m_scale; };
	WgCoord		ViewPointOfs() { return (m_viewPixOfs * WG_SCALE_BASE) / m_scale; };

	Uint32		ViewPixelLenX();
	Uint32		ViewPixelLenY();

	Uint32		ViewPointLenX();
	Uint32		ViewPointLenY();

	float		ViewOfsX();
	float		ViewOfsY();
	float		ViewLenX();
	float		ViewLenY();

	bool		SetViewPixelOfs( int x, int y );
	bool		SetViewPixelOfsX( int x );
	bool		SetViewPixelOfsY( int y );

	bool		SetViewPointOfs( int x, int y );
	bool		SetViewPointOfsX( int x );
	bool		SetViewPointOfsY( int y );


	bool		SetViewOfs( float x, float y );
	bool		SetViewOfsX( float x );
	bool		SetViewOfsY( float y );

/*
	bool	ViewCenterOnPixel( int x, int y );
	bool	ViewCenterOnPixelX( int x );
	bool	ViewCenterOnPixelY( int y );

	bool	ViewInclude( WgRect& rect );
	bool	ViewIncludeX( WgRect& rect );
	bool	ViewIncludeY( WgRect& rect );
*/
    void SetDisableScroll(bool bDisable) { m_bDisableScroll = bDisable;  }

    bool        SetMouseWheelAxis(int wheelNb, wg::Axis axis);
    wg::Axis    MouseWheelAxis(int wheelNb);
    
	void        SetOverlayScrollbars(bool bOverlay);
	bool        OverlayScrollbars() const { return m_bOverlayScrollbars; }

	bool		SetAutoscroll( bool bAutoX, bool bAutoY	);
	bool		AutoScrollX() const { return m_bAutoScrollX; }
	bool		AutoScrollY() const { return m_bAutoScrollY; }

	bool				SetHSlider( WgHSlider * pSlider );
	void				DeleteHSlider() {SetVSlider(0);}
	WgHSlider *	HSlider() const { return (WgHSlider*) m_elements[XDRAG].Widget(); }
	WgHSlider* 	ReleaseHSlider();

	bool				SetVSlider( WgVSlider * pSlider );
	void				DeleteVSlider() {SetVSlider(0);}
	WgVSlider *	VSlider() const { return (WgVSlider*) m_elements[YDRAG].Widget(); }
	WgVSlider* 	ReleaseVSlider();

	bool				SetContent( WgWidget * pContent );
	void				DeleteContent() {SetContent(0); }
	WgWidget*			Content() const { return m_elements[WINDOW].Widget(); }
	WgWidget*			ReleaseContent();

	bool				DeleteChild( WgWidget * pWidget );
	WgWidget *			ReleaseChild( WgWidget * pWidget );

	bool				DeleteAllChildren();
	bool				ReleaseAllChildren();

	void				SetSliderAutoHide( bool bHideVSlider, bool bHideHSlider );
	bool				GetHSliderAutoHide() const { return m_bAutoHideSliderX; }
	bool				GetVSliderAutoHide() const { return m_bAutoHideSliderY; }

	void				SetSliderPositions( bool bBottom, bool bRight );
	bool				IsSliderAtBottom() const { return m_bSliderBottom; }
	bool				IsSliderAtRight() const { return m_bSliderRight; }

	bool				IsVSliderVisible();
	bool				IsHSliderVisible();

	void				SetShowOverlaySlidersOnlyOnHover( bool bShowOnlyOnHover );
	
	void				SetFillerBlocks( wg::Skin * pFillerSkin );
	wg::Skin_p          FillerBlocks() const { return m_pFillerBlocks; }


	WgRect              ViewGeoPixels() const;                // Geometry of View window within WgScrollPanel widget.
	WgRect              ContentInViewPixels() const;          // Section of content displayed in the view.
	bool                PositionContentInViewPixels( WgCoord posInContent, WgOrigo viewPosOrigo = WgOrigo::Center, WgCoord viewPosOffset = {0,0} );

	void				SetInstantIntoView(bool bInstant);	// Set to make scrollIntoView() jump directly to distination (no scrolling).
	void                ScrollIntoView( WgWidget * pWidget, const WgBorders& margin = 0, const WgRect& viewSection = WgRect() );

	void                SetRubberBorder( const WgBorders& border );
	void                EnableRubberBorder( bool bEnabled );
	void                RubberBorderSnapToPos();

	bool                SetHoverScrollBorder( const WgBorders& border );
	bool                SetHoverScrollSpeed( int maxPointsPerSecond );

	// Overloaded from WgPanel

	WgWidget * FindWidget( const WgCoord& pos, WgSearchMode mode );

	// Overloaded from Widget

	WgSize				PreferredPixelSize() const;

	int					MatchingPixelHeight(int pixelWidth) const;
	int					MatchingPixelWidth(int pixelHeight) const;


/*
	NEED TO BE IMPLEMENTED!!!

	int		MatchingPixelHeight( int width ) const;	//
	int		MatchingPixelWidth( int height ) const;

*/

protected:

	enum ElementType
	{
		WINDOW = 0,
		XDRAG,
		YDRAG
	};


	class SliderTarget : public WgSliderTarget
	{
	public:
		float		_stepFwd();
		float		_stepBwd();
		float		_jumpFwd();
		float		_jumpBwd();
		float		_wheelRolled( int distance );

		float		_setPosition( float fraction );

		WgWidget*	_getWidget();
		float		_getSliderPosition();
		float		_getSliderSize();
		void		_sliderDragStart();
		void		_sliderDragEnd();

		
		void		_updateSlider( float pos, float size ) { WgSliderTarget::_updateSlider(pos,size); }

		bool		m_bHorizontal;
		WgScrollPanel *	m_pParent;
	};


	static const int	MAX_ELEMENTS = 3;

	virtual void _onNewSize( const WgSize& size );

	//

	WgCoord     _calcDesiredWindowPos( const WgRect& keepInView );

	void		_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void		_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );
	void		_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
	void		_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );

	bool		_onAlphaTest( const WgCoord& ofs );
	void		_onCloneContent( const WgWidget * _pOrg );

	WgHook *	_firstHookWithGeo( WgRect& geo ) const;
	WgHook *	_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const;

	WgHook *	_lastHookWithGeo( WgRect& geo ) const;
	WgHook *	_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const;


	WgSize		_calcContentSize( WgSize mySize );
	void		_updateElementGeo( WgSize mySize );

	WgRect		_genContentCanvasGeo( const WgRect& window, WgSize contentSize, WgOrigo origo, WgCoord viewOfs );

	void        _inViewRequested( WgHook * pChild );
	void        _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea );

	bool		_wheelRollX(int distance);
	bool		_wheelRollY(int distance);

	WgSize		m_contentSize;

	int			m_stepSizeX;
	int			m_stepSizeY;
	float		m_jumpSizeX;
	float		m_jumpSizeY;

	WgCoord		m_viewPixOfs;

	bool		m_bAutoHideSliderX;		// Should X-slider autohide when not needed?
	bool		m_bAutoHideSliderY;		// Should Y-slider autohide when not needed?

	bool		m_bSliderBottom;		// True if X-slider should be at the bottom, otherwise at the top.
	bool		m_bSliderRight;			// True if Y-slider should be at the right, otherwise at the left.

	bool		m_bAutoScrollX;
	bool		m_bAutoScrollY;

    wg::Axis    m_mouseWheelAxis[5];
    
    bool        m_bOverlayScrollbars;
	bool		m_bShowOverlaysOnlyOnHover = false;
	bool		m_bHovered = false;			// HACK! Only used by ShowOverlaysOnlyOnHover.
	bool		m_bSliderDragInProgress = false;

	WgScrollHook		m_elements[MAX_ELEMENTS];	// Content, xDrag and yDrag widgets in that order.

	WgColor			m_bgColor;
	WgSizePolicy	m_widthPolicy;
	WgSizePolicy	m_heightPolicy;
	WgOrigo	m_contentOrigo;		// Origo when content is smaller than window

	SliderTarget	m_sliderTargets[2];			// Order: Vertical, horizontal

private:

	WgHook*	_firstHook() const;
	WgHook*	_lastHook() const;


	wg::Skin_p		m_pFillerBlocks;
	WgRect			m_geoFiller;

	WgWidgetWeakPtr m_pScrollIntoViewChild;                 // Child that should be scrolled into viewSection of view.
	WgRect          m_scrollIntoViewSection;                // Section of view child should be scrolled into (default is whole view).
	WgBorders       m_scrollIntoViewMargin;                 // Margin around child that should be visible.
	bool			m_bInstantIntoView = false;				// Set to make ScrollIntoView() jump directly to destination.
	WgBorders       m_rubberBorder;
	bool            m_bRubberBorder = false;
	bool            m_bRubberBorderHold = false;            // When set, rubber border is temporary disabled.
	int             m_rubberBorderPause = 0;                // Millisec left for the rubber border to pause

	WgBorders       m_hoverScrollBorder;
	int             m_hoverScrollSpeed = 500;               // Max hover scroll speed in points/sec.
	int             m_hoverScrollX = 0;                     // X-scroll (positive or negative) in pixels/sec.
	int             m_hoverScrollY = 0;                     // Y-scroll (positive or negative) in pixels/sec.
	int             m_hoverScrollRemainsX = 0;              // Saved remains from hover scroll calculations. 1/1000th of a pixel.
	int             m_hoverScrollRemainsY = 0;              // Saved remains from hover scroll calculations. 1/1000th of a pixel.

	std::vector<uint8_t>    m_hoverScrollCategories;        // DropPick-categories for which we activate scrollHover (none-set = all).

    bool m_bDisableScroll = false;
};

#endif //WG2_SCROLLPANEL_DOT_H

