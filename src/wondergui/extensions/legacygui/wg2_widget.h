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
#ifndef WG2_WIDGET_DOT_H
#define WG2_WIDGET_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#ifndef WG2_HOOK_DOT_H
#	include <wg2_hook.h>
#endif

#ifndef WG_EVENT_DOT_H
#	include <wg2_event.h>
#endif

#include <wg_string.h>
#include <wg_skin.h>
#include <wg2_patches.h>

class Wg_Interface_TextHolder;
class WgContainer;
class WgPanel;
class WgCapsule;
class WgLayer;
class WgEventHandler;

namespace wg
{
	class LegacyRootCapsule;
}

class WgWidget : public WgWeakPtrTarget
{
    friend class WgInput;
    friend class WgEventHandler;

    friend class WgHook;
    friend class WgFlexHook;
    friend class WgModalHook;
    friend class WgListHook;
    friend class WgStackHook;
    friend class WgDragNDropHook;

    friend class WgRoot;
	friend class WgRootPanel;
	friend class WgPluginRoot;
    friend class WgFlexPanel;
    friend class WgModalLayer;
    friend class WgTablePanel;
    friend class WgScrollPanel;
    friend class WgListPanel;
    friend class WgCapsule;
    friend class WgLayer;
    friend class WgStackPanel;
    friend class WgContainer;
    friend class WgPanel;
    friend class WgPackPanel;
    friend class WgShaderCapsule;
    friend class WgCanvasCapsule;
    friend class WgRenderLayerCapsule;
    friend class WgFixCapsule;
    friend class WgPopupLayer;
    friend class WgFlowPanel;
    friend class WgDragNDropLayer;
    friend class WgShadowLayer;
    friend class WgTooltipLayer;
        
    friend class WgTableRow;

    friend class WgZoomOutCapsule;
	
	friend class wg::LegacyRootCapsule;

public:
	WgWidget();
	virtual ~WgWidget();

	virtual const char *Type( void ) const = 0;

	inline int			Id() const { return m_id; }
	inline void			SetId( int id ) { m_id = id; }

	virtual void		SetSkin(wg::Skin * pSkin);
	wg::Skin_p			Skin() const { return m_pSkin; }

    wg::String          GetTooltipString() const { return m_bShowTooltipString ? m_tooltip : ""; }
    void                SetTooltipString( const wg::String& str ) { m_tooltip = str; }
    void                SetShowTooltipString(const bool bShowTooltipString) { m_bShowTooltipString = bShowTooltipString; }
    bool                GetShowTooltipString() const { return m_bShowTooltipString; }

	inline void			Refresh() { _onRefresh(); }
	void				SetEnabled(bool bEnabled);
	inline bool			IsEnabled() const { return m_bEnabled; }

	bool				CloneContent( const WgWidget * _pOrg );

	void			SetPointerStyle( WgPointerStyle style )	{ m_pointerStyle = style; }
	virtual WgPointerStyle	GetPointerStyle() const;

	void			SetTabLock( bool bLock ) { m_bTabLock = bLock; }
	bool			IsTabLocked() const { return m_bTabLock; }

	virtual void	SetMarkOpacity( int opacity ) { m_markOpacity = opacity; }
	int				GetMarkOpacity() const { return m_markOpacity; }
	virtual bool	MarkTest( const WgCoord& ofs );

	WgHook*			Hook() const { return m_pHook; }

	virtual WgWidget * NewOfMyType() const = 0;

	// Convenient calls to hook

	WgCoord			PointPos() const { if (m_pHook) return m_pHook->PointPos(); return WgCoord(0, 0); }
	WgSize			PointSize() const { if (m_pHook) return m_pHook->PointSize(); return WgSize(256, 256); }
	WgRect			PointGeo() const { if (m_pHook) return m_pHook->PointGeo(); return WgRect(0, 0, 256, 256); }
	WgCoord			ScreenPointPos() const { if (m_pHook) return m_pHook->ScreenPointPos(); return WgCoord(0, 0); }
	WgRect			ScreenPointGeo() const { if (m_pHook) return m_pHook->ScreenPointGeo(); return WgRect(0, 0, 256, 256); }

	WgSize          PreferredPointSize() const;
	WgSize          MinPointSize() const;
	WgSize          MaxPointSize() const;

	WgCoord			PixelPos() const { if( m_pHook ) return m_pHook->PixelPos(); return WgCoord(0,0); }
	WgSize			PixelSize() const { if( m_pHook ) return m_pHook->PixelSize(); return WgSize(256,256); }
	WgRect			PixelGeo() const { if( m_pHook ) return m_pHook->PixelGeo(); return WgRect(0,0,256,256); }
	WgCoord			ScreenPixelPos() const { if( m_pHook ) return m_pHook->ScreenPixelPos(); return WgCoord(0,0); }
	WgRect			ScreenPixelGeo() const { if( m_pHook ) return m_pHook->ScreenPixelGeo(); return WgRect(0,0,256,256); }

    bool            LockPointer();
    void            ReleasePointer();
    
	bool			GrabFocus() { if( m_pHook ) return m_pHook->_requestFocus(); return false; }
	bool			ReleaseFocus() { if( m_pHook && m_bFocused ) return m_pHook->_releaseFocus(); return false; }
	bool			IsFocused() { return m_bFocused; }
	WgContainer *	Parent() const { if( m_pHook ) return m_pHook->_parent(); return 0; }
	WgWidgetHolder* Holder() const { if( m_pHook ) return m_pHook->_holder(); return 0; }

	WgWidget *		NextSibling() const { if( m_pHook ) {WgHook * p = m_pHook->Next(); if( p ) return p->Widget(); } return 0; }
	WgWidget *		PrevSibling() const { if( m_pHook ) {WgHook * p = m_pHook->Prev(); if( p ) return p->Widget(); } return 0; }

	WgCoord			Local2absPixel( const WgCoord& cord ) const;		// Cordinate from local cordsys to global (in pixels)
	WgCoord			Abs2localPixel( const WgCoord& cord ) const; 		// Cordinate from global to local cordsys (in pixels)

	WgCoord			Local2absPoint( const WgCoord& cord ) const;		// Cordinate from local cordsys to global (in pixels)
	WgCoord			Abs2localPoint( const WgCoord& cord ) const; 		// Cordinate from global to local cordsys (in pixels)

	wg::Surface_p	Screenshot( const wg::Surface::Blueprint& bp );
	wg::Surface_p	Screenshot(const WgRect& _rect, const wg::Surface::Blueprint& bp);

    WgWidget *      CommonAncestor(WgWidget* pOtherWidget);

	int Scale() const { return m_scale; }

	// To be overloaded by Widget

	virtual int		MatchingPixelHeight( int pixelWidth ) const;
	virtual int		MatchingPixelWidth( int pixelHeight ) const;
    
    int             MatchingPointHeight( int pixelWidth ) const;

	virtual WgSize	PreferredPixelSize() const = 0;
	virtual WgSize	MinPixelSize() const;
	virtual WgSize	MaxPixelSize() const;

	virtual bool	IsContainer() const { return false; }
	virtual bool	IsPanel() const { return false; }
	virtual bool	IsCapsule() const { return false; }
	virtual bool	IsLayer() const { return false; }
	virtual WgContainer * CastToContainer() { return 0; }
	virtual const WgContainer * CastToContainer() const { return 0; }
	virtual WgPanel * CastToPanel() { return 0; }
	virtual const WgPanel * CastToPanel() const { return 0; }
	virtual WgCapsule * CastToCapsule() { return 0; }
	virtual const WgCapsule * CastToCapsule() const { return 0; }
	virtual WgLayer * CastToLayer() { return 0; }
	virtual const WgLayer * CastToLayer() const { return 0; }

    void            SetFullStateSupport( bool bFullStateSupport ) { m_bFullStateSupport = bFullStateSupport; }

	virtual bool	SetMarked();					// Switch to WG_MODE_MARKED unless we are disabled or widget controls mode itself.
	virtual bool	SetSelected();					// Switch to WG_MODE_SELECTED unless we are disabled or widget controls mode itself.
	virtual bool	SetNormal();					// Switch to WG_MODE_NORMAL unless we are disabled or widget controls mode itself.

	inline bool		IsSelected() { return m_bSelected; }

	void			SetSelectable(bool bSelectable);
	inline bool		IsSelectable() const { return m_bSelectable; };

	bool			IsDisplayed() const;
	
	virtual WgMode	Mode() const;

	virtual void        setPickable( bool bPickable, int category = 0 );
	bool                isPickable() const { return m_bPickable; }
	int                 pickCategory() const { return m_pickCategory; }

	void                setDropTarget( bool bDropTarget );
	bool                isDropTarget() const { return m_bDropTarget; }
    WgState             State() { return m_state; }

protected:

	void			_onNewHook( WgHook * pHook );
	virtual void	_onNewRoot( WgRoot * pRoot );

	void			_startReceiveTicks();
	void			_stopReceiveTicks();
	virtual WgBlendMode	_getBlendMode() const;

	WgEventHandler* _eventHandler() const;
	void			_queueEvent( WgEvent::Event * pEvent );

	bool            _requestPreRenderCall();

    int             _listAncestors(WgWidget* array[], int max);

	
	// Convenient calls to hook

	void			_requestRender() { if( m_pHook ) m_pHook->_requestRender(); }
	void			_requestRender( const WgRect& rect ) { if( m_pHook ) m_pHook->_requestRender( rect ); }
	void			_requestResize() { if( m_pHook ) m_pHook->_requestResize(); }

	inline void        _requestInView() { if( m_pHook ) m_pHook->_requestInView(); }
	inline void        _requestInView( const WgRect& mustHaveArea, const WgRect& niceToHaveArea ) { if( m_pHook ) m_pHook->_requestInView( mustHaveArea, niceToHaveArea ); }


	// To be overloaded by Widget

	virtual void    _preRender();

	int				_convertAndPushClipList( wg::GfxDevice * pDevice, int nRects, const WgRect * pRects );
	void			_popAndReleaseClipList( wg::GfxDevice * pDevice, int bytesToRelease );

	virtual void	_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );
	virtual void	_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
	virtual void	_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );
	virtual void	_onCloneContent( const WgWidget * _pOrg ) = 0;
	virtual void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	virtual void	_onNewSize( const WgSize& size );
	virtual void	_setScale( int scale );
	virtual void	_onRefresh();

	virtual void	_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	virtual	bool	_onAlphaTest( const WgCoord& ofs );
	virtual void	_onEnable();
	virtual void	_onDisable();
	virtual void	_onGotInputFocus();
	virtual void	_onLostInputFocus();

    virtual void    _setState( WgState state );

    
	// rename when widgets are done
	virtual bool	TempIsInputField() const;
	virtual Wg_Interface_TextHolder*	TempGetText();

	// Skin wrapping methods

	void            _renderSkin( wg::Skin * pSkin, wg::GfxDevice * pDevice, wg::State state, const wg::RectI& rect, int scale, float value = 1.f, float value2 = -1.f );
	bool            _markTestSkin( wg::Skin * pSkin, const wg::CoordI& ofs, const wg::RectI& canvas, wg::State state, int opacityTreshold, int scale, float value = 1.f, float value2 = -1.f) const;
	wg::SizeI       _skinMinSize( wg::Skin * pSkin, int scale ) const;
	wg::SizeI       _skinPreferredSize( wg::Skin * pSkin, int scale ) const;
	wg::SizeI       _skinSizeForContent( wg::Skin * pSkin, const wg::SizeI contentSize, int scale) const;
	wg::SizeI       _skinContentPadding(wg::Skin * pSkin, int scale) const;
	wg::CoordI      _skinContentOfs(wg::Skin * pSkin, wg::State state, int scale) const;
	wg::RectI       _skinContentRect(wg::Skin * pSkin, const wg::RectI& canvas, wg::State state, int scale) const;
    wg::RectI       _skinDirtyRect(wg::Skin * pSkin, const wg::RectI& canvas, int scale, wg::State newState, wg::State oldState,
                                   float newValue = 1, float oldValue =1, float newValue2 = -1, float oldValue2 = -1, int newAnimPos = 0, int oldAnimPos = 0 ) const;

	//

	Uint32			m_id;
	WgHook *		m_pHook;

	wg::Skin_p		m_pSkin;

	int				m_scale;
	WgPointerStyle	m_pointerStyle;

	wg::String		m_tooltip;
	int				m_markOpacity;

	bool			m_bOpaque;
	bool			m_bTabLock;		// If set, the widget prevents focus shifting away from it with tab.
	bool			m_bReceiveTick;	// Set if Widget should reveive periodic Tick() events.

	bool			m_bEnabled;		// Set when object is not disabled
	bool			m_bFocused;		// Set when object is enabled and has keyboard focus
	bool			m_bSelected;

	bool			m_bSelectable = true;	// Set if widget is allowed to be selected.
    bool            m_bFullStateSupport = false;
    
	WgState         m_state;

	bool            m_bPickable;        // Set if this widget accepts to be the source of drag-n-drop operations.
	uint8_t         m_pickCategory;     // Category of drag-n-drop operations. User defined.

	bool            m_bDropTarget;      // Set if this widget accepts to be the target of drag-n-drop operations.
    
    bool            m_bPointerLocked = false;

    bool            m_bShowTooltipString = true;
};

typedef class WgWeakPtr<WgWidget> WgWidgetWeakPtr;

typedef	int(*WgWidgetSortFunc)(const WgWidget *,const WgWidget *);

template<typename T> T* WgCast(WgWidget * pWidget)
{
	if(pWidget)
	{
		if(T::GetClass() == pWidget->Type())
			return static_cast<T*>(pWidget);
	}
	return 0;
}

template<typename T> const T* WgCast(const WgWidget * pWidget)
{
	if(pWidget)
	{
		if(T::GetClass() == pWidget->Type())
			return static_cast<const T*>(pWidget);
	}
	return 0;
}


#endif
