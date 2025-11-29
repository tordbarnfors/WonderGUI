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
#ifndef WG2_FLOWPANEL_DOT_H
#define WG2_FLOWPANEL_DOT_H

#ifndef	WG2_SIZEBROKER_DOT_H
#	include <wg2_sizebroker.h>
#endif

#ifndef WG2_VECTORPANEL_DOT_H
#	include <wg2_vectorpanel.h>
#endif

class WgFlowPanel;

class WgFlowHook : public WgVectorHook
{
	friend class WgFlowPanel;

public:
	const char *Type( void ) const;
	static const char * ClassType();

	bool	SetWeight( float weight );
	float	Weight() { return m_weight; }

	WgFlowHook *	Prev() const { return _prev(); }
	WgFlowHook *	Next() const { return _next(); }
	WgFlowPanel*	Parent() const;

protected:
	PROTECTED_LINK_METHODS( WgFlowHook );

	WgFlowHook( WgFlowPanel * pParent );

	WgContainer * _parent() const;

	float			m_weight;			// Weight for space allocation.
	WgRect			m_geo;				// Real geo of child (no padding included).
	WgSize			m_preferredSize;	// Cached padded preferred size from the child.
	WgFlowPanel *	m_pParent;
};




class WgFlowPanel : public WgVectorPanel
{
	friend class WgFlowHook;

public:
	WgFlowPanel();
	virtual ~WgFlowPanel();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgFlowPanel(); };

	inline WgFlowHook * AddChild( WgWidget * pWidget ) { return static_cast<WgFlowHook*>(WgVectorPanel::AddChild(pWidget)); }
	inline WgFlowHook * AddChildren( WgWidget * pWidgets[], int nb ) { return static_cast<WgFlowHook*>(WgVectorPanel::AddChildren(pWidgets, nb)); }
	inline WgFlowHook * InsertChild( WgWidget * pWidget, WgWidget * pSibling ) { return static_cast<WgFlowHook*>(WgVectorPanel::InsertChild(pWidget,pSibling)); }
	inline WgFlowHook * InsertChildSorted( WgWidget * pWidget ) { return static_cast<WgFlowHook*>(WgVectorPanel::InsertChildSorted(pWidget)); }

    void            PadAllChildren( WgBorders padding );

	void			SetOrientation( wg::Axis orientaiton );
	wg::Axis		Orientation() const { return m_bHorizontal?wg::Axis::X:wg::Axis::Y; }

	WgFlowHook *	FirstHook() const { return static_cast<WgFlowHook*>(_firstHook()); }
	WgFlowHook *	LastHook() const { return static_cast<WgFlowHook*>(_lastHook()); }

//	void			SetSizeBroker( WgSizeBroker* pBroker );
//	WgSizeBroker *	SizeBroker() const { return m_pSizeBroker; }

	int				MatchingPixelHeight( int width ) const;
	int				MatchingPixelWidth( int height ) const;

	WgSize			PreferredPixelSize() const;

protected:

	// Overloaded from Widget

	void			_onNewSize( const WgSize& size );


	// Overloaded from Container

	WgHook*			_firstHookWithGeo( WgRect& geo ) const;
	WgHook*			_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const;

	WgHook*			_lastHookWithGeo( WgRect& geo ) const;
	WgHook*			_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const;


	// Overloaded from VectorPanel

	WgRect			_hookGeo( const WgVectorHook * pHook );
	void			_onResizeRequested( WgVectorHook * pHook );
	void			_onRenderRequested( WgVectorHook * pHook );
	void			_onRenderRequested( WgVectorHook * pHook, const WgRect& rect );
	void			_onWidgetAppeared( WgVectorHook * pInserted );				// so parent can update geometry and possibly request render.
	void            _onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast );
	void			_onWidgetDisappeared( WgVectorHook * pToBeRemoved );		// so parent can update geometry and possibly request render.
    void            _onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast );
	void			_onWidgetsReordered();
	void			_refreshAllWidgets();
	WgVectorHook *	_newHook();

	//

	void			_refreshChildGeo();
	void			_updatePreferredPixelSize();
	int				_populateSizeBrokerArray( WgSizeBrokerItem * pArray ) const;
	int				_populateSizeBrokerArray( WgSizeBrokerItem * pArray, int forcedBreadth ) const;

	bool			m_bHorizontal;
	WgSizeBroker * 	m_pSizeBroker;
	WgSize			m_preferredSize;

};


#endif //WG2_FLOWPANEL_DOT_H
