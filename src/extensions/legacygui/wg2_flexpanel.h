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
#ifndef WG2_FLEXPANEL_DOT_H
#define WG2_FLEXPANEL_DOT_H

#include <vector>

#ifndef WG2_PANEL_DOT_H
#	include <wg2_panel.h>
#endif

#include <wg_chain.h>

class WgFlexPanel;

//____ WgFlexHook _____________________________________________________________

class WgFlexHook : public WgPanelHook, protected wg::Link
{
	friend class WgWidget;
	friend class WgPanel;
	friend class WgFlexPanel;
	friend class wg::Chain<WgFlexHook>;

public:

	const char *Type( void ) const;
	static const char * ClassType();

	// Flex-specific methods

	bool	SetAnchored();
	bool	SetAnchored( int anchorTopLeft, int anchorBottomRight, WgBorders padding = 0 );

	bool	SetFloating();
	bool	SetFloating( const WgCoord& pointPos, WgOrigo origo = WgOrigo::NorthWest );
	bool	SetFloating( const WgCoord& pointPos, int anchor, WgOrigo hotspot );
	bool	SetFloating( const WgRect& pointGeometry, WgOrigo origo = WgOrigo::NorthWest );
	bool	SetFloating( const WgRect& pointGeometry, int anchor, WgOrigo hotspot );

	bool    SetPadding( WgBorders padding );


	void	Top();								// Put us ontop all our silbings.
	void	Bottom();							// Put us below all our siblings.
	bool	Up();								// Move us one step up among siblings.
	bool	Down();								// Move us one step down among siblings.

	bool	MoveOver( WgFlexHook * pSibling );
	bool	MoveUnder( WgFlexHook * pSibling );

	bool	IsFloating() const { return m_bFloating; }
	bool	IsAnchored() const { return !m_bFloating; }

	// Methods for floating hooks

	bool	SetAnchor( int anchor );
	bool	SetHotspot( WgOrigo hotspot );

	bool	SetSizePolicy( WgSizePolicy width, WgSizePolicy height );

	bool	SetPointGeo( const WgRect& geometry );

	bool	SetPointOfs( const WgCoord& ofs );
	bool	SetPointOfsX( int x );
	bool	SetPointOfsY( int y );

	bool	SetPointSize( const WgSize& size );
	bool	SetPointWidth( int width );
	bool	SetPointHeight( int height );

	bool	MovePoints( const WgCoord& ofs );
	bool	MovePointsX( int x );
	bool	MovePointsY( int y );

	WgSizePolicy	WidthPolicy() const { return m_widthPolicy; }
	WgSizePolicy	HeightPolicy() const { return m_heightPolicy; }
	int				Origo() const { return m_anchor; }
	WgOrigo	Hotspot() const { return m_hotspot; }
	WgRect			FloatPointGeo() const { return m_placementGeo; }
	WgCoord			FloatPointOfs() const { return m_placementGeo.pos(); }
	WgRect			FloatPointSize() const { return m_placementGeo.size(); }

	// Methods for anchored hooks

	int				AnchorTopLeft() const { return m_anchorTopLeft; }
	int				AnchorBottomRight() const { return m_anchorBottomRight; }

	// Standard Hook methods

	WgCoord			PixelPos() const { return m_realGeo.pos(); }
	WgSize			PixelSize() const { 	return m_realGeo.size(); }
	WgRect			PixelGeo() const { return m_realGeo; }


	WgCoord			ScreenPixelPos() const;
	WgRect			ScreenPixelGeo() const;

	WgFlexHook *	Prev() const { return _prev(); }
	WgFlexHook *	Next() const { return _next(); }

	WgFlexPanel* Parent() const { return m_pParent; }

protected:
	// TODO: Constructor should in the future call SetHook() on Widget, once we are totally rid of widgets...

	PROTECTED_LINK_METHODS( WgFlexHook );

	WgFlexHook( WgFlexPanel * pParent, const WgRect& placementGeo, WgBorders padding );

	void		_refreshRealGeo();
	WgSize		_sizeNeededForGeo();

	void		_requestRender();
	void		_requestRender( const WgRect& rect );
	void		_requestResize();

	WgHook *		_prevHook() const;
	WgHook *		_nextHook() const;
	WgContainer * _parent() const;

	WgFlexPanel* m_pParent;

	bool			m_bFloating;		// true = floating, false = anchored
	WgRect			m_realGeo;			// Widgets geo relative parent

	// Only used for floating hooks.

	WgSizePolicy	m_widthPolicy;
	WgSizePolicy	m_heightPolicy;

	int				m_anchor;
	WgOrigo         m_hotspot;
	WgRect			m_placementGeo;	// Widgets geo relative anchor and hotspot.

	//Only used for anchored hooks.

	int				m_anchorTopLeft;
	int				m_anchorBottomRight;

};


//____ WgFlexAnchor ___________________________________________________________

class WgFlexAnchor
{
	friend class WgFlexPanel;

public:
	float	RelativeX() const { return m_xRelative; }
	float	RelativeY() const { return m_yRelative; }
	int		OffsetX() const { return m_pixelOfs.x; }
	int		OffsetY() const { return m_pixelOfs.y; }
	WgCoord	Offset() const { return m_pixelOfs; }

	WgCoord	Position( const WgSize& parentSize, int scaleFactor ) const { return WgCoord( ((int)(m_xRelative*parentSize.w))+((m_pixelOfs.x*scaleFactor)>>WG_SCALE_BINALS),
																						  ((int)(m_yRelative*parentSize.h)) +((m_pixelOfs.y*scaleFactor)>>WG_SCALE_BINALS) ); }

private:
	WgFlexAnchor() : m_xRelative(0.f), m_yRelative(0.f), m_pixelOfs(0,0) {};
	WgFlexAnchor( float xRelative, float yRelative, const WgCoord& pixelOfs ) : m_xRelative(xRelative), m_yRelative(yRelative), m_pixelOfs(pixelOfs) {}

	float	m_xRelative;
	float	m_yRelative;
	WgCoord	m_pixelOfs;
};


//____ WgFlexPanel _________________________________________________________

class WgFlexPanel : public WgPanel
{
friend class WgFlexHook;

public:
	WgFlexPanel();
	virtual ~WgFlexPanel();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgFlexPanel(); };

	void			SetConfineChildren( bool bRestrictChildren );
	bool			IsConfiningChildren() const { return m_bConfineChildren; }


	WgFlexHook *	AddChild( WgWidget * pWidget );
	WgFlexHook *	AddChild( WgWidget * pWidget, int anchorTopLeft, int anchorBottomRight, WgBorders padding = 0 );

	WgFlexHook *    AddChild( WgWidget * pWidget, WgOrigo anchorTopLeft, WgOrigo anchorBottomRight, WgBorders padding = 0 )
	{
		return AddChild(pWidget,static_cast<int>(anchorTopLeft),static_cast<int>(anchorBottomRight),padding);
	}
	WgFlexHook *    AddChild( WgWidget * pWidget, int anchorTopLeft, WgOrigo anchorBottomRight, WgBorders padding = 0 )
	{
		return AddChild(pWidget,static_cast<int>(anchorTopLeft),anchorBottomRight,padding);
	}
	WgFlexHook *    AddChild( WgWidget * pWidget, WgOrigo anchorTopLeft, int anchorBottomRight, WgBorders padding = 0 )
	{
		return AddChild(pWidget,static_cast<int>(anchorTopLeft),anchorBottomRight,padding);
	}

	WgFlexHook *	AddChild( WgWidget * pWidget, const WgCoord& pos, WgOrigo origo = WgOrigo::NorthWest, WgBorders padding = 0 );
	WgFlexHook *	AddChild( WgWidget * pWidget, const WgCoord& pos, int anchor, WgOrigo hotspot, WgBorders padding = 0 );
	WgFlexHook *    AddChild( WgWidget * pWidget, const WgCoord& pos, WgOrigo anchor, WgOrigo hotspot, WgBorders padding = 0 )
	{
		return AddChild(pWidget,pos,static_cast<int>(anchor), hotspot, padding);
	}

	WgFlexHook *	AddChild( WgWidget * pWidget, const WgRect& geometry, WgOrigo origo = WgOrigo::NorthWest, WgBorders padding = 0 );
	WgFlexHook *	AddChild( WgWidget * pWidget, const WgRect& geometry, int anchor, WgOrigo hotspot, WgBorders padding = 0 );
	WgFlexHook *    AddChild( WgWidget * pWidget, const WgRect& geometry, WgOrigo anchor, WgOrigo hotspot, WgBorders padding = 0 )
	{
		return AddChild(pWidget,geometry,static_cast<int>(anchor), hotspot, padding);
	}

	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling );
	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling, int anchorTopLeft, int anchorBottomRight, WgBorders padding = 0 );
	WgFlexHook *    InsertChild( WgWidget * pWidget, WgWidget * pSibling, WgOrigo anchorTopLeft, WgOrigo anchorBottomRight, WgBorders padding = 0 )
	{
		return InsertChild(pWidget,pSibling,static_cast<int>(anchorTopLeft), static_cast<int>(anchorBottomRight),padding);
	}
	WgFlexHook *    InsertChild( WgWidget * pWidget, WgWidget * pSibling, int anchorTopLeft, WgOrigo anchorBottomRight, WgBorders padding = 0 )
	{
		return InsertChild(pWidget,pSibling,anchorTopLeft,static_cast<int>(anchorBottomRight),padding);
	}
	WgFlexHook *    InsertChild( WgWidget * pWidget, WgWidget * pSibling, WgOrigo anchorTopLeft, int anchorBottomRight, WgBorders padding = 0 )
	{
		return InsertChild(pWidget,pSibling,static_cast<int>(anchorTopLeft),anchorBottomRight,padding);
	}

	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgCoord& pos, WgOrigo origo = WgOrigo::NorthWest, WgBorders padding = 0  );
	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgCoord& pos, int anchor, WgOrigo hotspot, WgBorders padding = 0  );
	WgFlexHook *    InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgCoord& pos, WgOrigo anchor, WgOrigo hotspot, WgBorders padding = 0  )
	{
		return InsertChild( pWidget, pSibling, pos, static_cast<int>(anchor), hotspot, padding);
	}

	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgRect& geometry, WgOrigo origo = WgOrigo::NorthWest, WgBorders padding = 0  );
	WgFlexHook *	InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgRect& geometry, int anchor, WgOrigo hotspot, WgBorders padding = 0  );
	WgFlexHook *    InsertChild( WgWidget * pWidget, WgWidget * pSibling, const WgRect& geometry, WgOrigo anchor, WgOrigo hotspot, WgBorders padding = 0  )
	{
		return InsertChild(pWidget, pSibling, geometry, static_cast<int>(anchor), hotspot, padding);
	}

	bool			DeleteChild( WgWidget * pWidget );
	WgWidget *		ReleaseChild( WgWidget * pWidget );

	bool			DeleteAllChildren();
	bool			ReleaseAllChildren();

	int				AddAnchor( float relativeX, float relativeY, const WgCoord& pixelOfs = WgCoord(0,0) );
	bool			ReplaceAnchor( int index, float relativeX, float relativeY, const WgCoord& pixelOfs = WgCoord(0,0) );
	bool			DeleteAnchor( int index );
	void			DeleteAllAnchors();

	int				NbAnchors() const { return int(m_anchors.size()) + wg::Placement_size; }
	const WgFlexAnchor *	Anchor( int index );

	WgFlexHook*		FirstHook() const { return m_hooks.first(); }
	WgFlexHook*		LastHook() const { return m_hooks.last(); }

	// Overloaded from WgWidget

	WgSize			PreferredPixelSize() const;
	void            SetPreferredPointSize( WgSize size );

protected:
    void _onEvent(const WgEvent::Event *pEvent, WgEventHandler *pHandler);
    void            _onNewSize( const WgSize& size );
	void			_setScale( int scale );

private:
	void			_onCloneContent( const WgWidget * _pOrg );

	WgSize          _scaledPreferredPixelSize( WgWidget * pWidget );

	void			_onRequestRender( const WgRect& rect, const WgFlexHook * pHook );	// rect is in our coordinate system.

	WgHook*			_firstHook() const { return FirstHook(); }
	WgHook*			_lastHook() const { return LastHook(); }

	WgHook *		_firstHookWithGeo( WgRect& writeGeo ) const;
	WgHook *		_nextHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const;

	WgHook *		_lastHookWithGeo( WgRect& writeGeo ) const;
	WgHook *		_prevHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const;

	wg::Chain<WgFlexHook>			m_hooks;
	std::vector<WgFlexAnchor>	m_anchors;

	bool			m_bConfineChildren;
	WgSize          m_preferredSize;

	static WgFlexAnchor	g_baseAnchors[wg::Placement_size];
};


#endif //WG2_FLEXPANEL_DOT_H
