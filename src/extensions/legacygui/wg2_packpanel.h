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
#ifndef WG2_PACKPANEL_DOT_H
#define WG2_PACKPANEL_DOT_H

#ifndef	WG2_SIZEBROKER_DOT_H
#	include <wg2_sizebroker.h>
#endif

#ifndef WG2_VECTORPANEL_DOT_H
#	include <wg2_vectorpanel.h>
#endif

class WgPackPanel;

class WgPackHook : public WgVectorHook
{
	friend class WgPackPanel;

public:
	const char *Type( void ) const;
	static const char * ClassType();

	bool	SetWeight( float weight );
	float	Weight() { return m_weight; }

	bool    SetBaseline( float baseline );
	float   Baseline() { return m_baseline; }


	WgPackHook *	Prev() const { return _prev(); }
	WgPackHook *	Next() const { return _next(); }
	WgPackPanel*		Parent() const;

protected:
	PROTECTED_LINK_METHODS( WgPackHook );

	WgPackHook( WgPackPanel * pParent );

	WgContainer * _parent() const;

	float			m_weight;			// Weight for space allocation.
	float           m_baseline;         // Baseline position (0.0-1.0) for widget.
	WgRect			m_geo;				// Real geo of child (no padding included).
	WgSize			m_preferredSize;	// Cached padded preferred size from the child.

		int             m_preferredWidthForHeight;    // Cached padded preferred
	int             m_preferredHeightForWidth;    // Cached padded preferred length for current breadth of the child.
	WgPackPanel *	m_pParent;
};




class WgPackPanel : public WgVectorPanel
{
	friend class WgPackHook;

public:
	WgPackPanel();
	virtual ~WgPackPanel();

	virtual const char *Type( void ) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override { return new WgPackPanel(); };

    virtual inline WgPackHook * AddChild( WgWidget * pWidget ) { return static_cast<WgPackHook*>(WgVectorPanel::AddChild(pWidget)); }
	inline WgPackHook * AddChildren( WgWidget * pWidgets[], int nb ) { return static_cast<WgPackHook*>(WgVectorPanel::AddChildren(pWidgets, nb)); }
	inline WgPackHook * InsertChild( WgWidget * pWidget, WgWidget * pSibling ) { return static_cast<WgPackHook*>(WgVectorPanel::InsertChild(pWidget,pSibling)); }
	inline WgPackHook * InsertChildSorted( WgWidget * pWidget ) { return static_cast<WgPackHook*>(WgVectorPanel::InsertChildSorted(pWidget)); }

	void			SetOrientation( wg::Axis orientaiton );
	wg::Axis		Orientation() const { return m_bHorizontal?wg::Axis::X:wg::Axis::Y; }

	WgPackHook *	FirstHook() const { return static_cast<WgPackHook*>(_firstHook()); }
	WgPackHook *	LastHook() const { return static_cast<WgPackHook*>(_lastHook()); }

	void			SetSizeBroker( WgSizeBroker* pBroker );
	WgSizeBroker *	SizeBroker() const { return m_pSizeBroker; }

	void            SetBaselineMode( bool bBaseline );
	bool            BaselineMode( bool bBaseline ) const { return m_bBaselineMode; }
	float           Baseline() const { return m_baseline; }

	int				MatchingPixelHeight( int width ) const override;
	int				MatchingPixelWidth( int height ) const override;

	WgSize			PreferredPixelSize() const override;

	void            FreezeGeo();
	void            UnfreezeGeo();

	void			SetSkin(wg::Skin* pSkin) override;


protected:

	// Overloaded from Widget

	void			_onNewSize( const WgSize& size ) override;
	void			_setScale(int scale) override;


	// Overloaded from Container

	WgHook*			_firstHookWithGeo( WgRect& geo ) const override;
	WgHook*			_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const override;

	WgHook*			_lastHookWithGeo( WgRect& geo ) const override;
	WgHook*			_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const override;


	// Overloaded from VectorPanel

	WgRect			_hookGeo( const WgVectorHook * pHook ) override;
	void			_onResizeRequested( WgVectorHook * pHook ) override;
	void			_onRenderRequested( WgVectorHook * pHook ) override;
	void			_onRenderRequested( WgVectorHook * pHook, const WgRect& rect ) override;
	void			_onWidgetAppeared( WgVectorHook * pInserted ) override;				// so parent can update geometry and possibly request render.
	void            _onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast ) override;
	void			_onWidgetDisappeared( WgVectorHook * pToBeRemoved ) override;		// so parent can update geometry and possibly request render.
    void            _onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast ) override;        // so parent can update geometry and possibly request render.
	void			_onWidgetsReordered() override;
	void			_refreshAllWidgets() override;
	WgVectorHook *	_newHook() override;

	//

	void            _refreshBaseline();
	void			_refreshChildGeo();
	void			_updatePreferredPixelSize();
	int				_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const;
	int				_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const;

	bool			m_bHorizontal;
	bool            m_bBaselineMode;
	float           m_baseline;

	WgSizeBroker * 	m_pSizeBroker;
	WgSize			m_preferredSize;                                    // Internal, excluding PackPanel's own skin padding.

	bool            m_bFreezeGeoExt = false;

	bool			m_bFreezeGeo = false;
	bool			m_bWantsGeoUpdate = false;

	bool            m_bChildGeoNeedsRefresh = false;

};


#endif //WG2_PACKPANEL_DOT_H
