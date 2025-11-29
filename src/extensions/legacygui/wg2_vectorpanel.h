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
#ifndef WG2_VECTORPANEL_DOT_H
#define WG2_VECTORPANEL_DOT_H

#ifndef WG2_PANEL_DOT_H
#	include <wg2_panel.h>
#endif

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#include <wg_chain.h>

class WgVectorPanel;


class WgVectorHook : public WgPanelHook, protected wg::Link
{
	friend class WgVectorPanel;
	friend class wg::Chain<WgVectorHook>;


public:
	WgCoord	PixelPos() const;
	WgSize	PixelSize() const;
	WgRect	PixelGeo() const;
	WgCoord	ScreenPixelPos() const;
	WgRect	ScreenPixelGeo() const;

	WgVectorHook*	Prev() const { return _prev(); }
	WgVectorHook*	Next() const { return _next(); }
	WgVectorPanel * Parent() const;

	bool			MoveForward();
	bool			MoveBackward();
	bool			MoveBefore( WgVectorHook * pSibling );
	bool			MoveAfter( WgVectorHook * pSibling );
	bool			MoveFirst();
	bool			MoveLast();

	bool			SetVisible( bool bVisible );

protected:
	PROTECTED_LINK_METHODS( WgVectorHook );

	WgVectorHook();
	~WgVectorHook();

	void	_requestRender();
	void	_requestRender( const WgRect& rect );
	void	_requestResize();

	WgHook *	_prevHook() const;
	WgHook *	_nextHook() const;
};


class WgVectorPanel : public WgPanel
{
	friend class WgVectorHook;
public:
	WgVectorPanel();
	virtual ~WgVectorPanel();


	WgVectorHook * AddChild( WgWidget * pWidget );
	WgVectorHook * AddChildren( WgWidget * pWidgets[], int nb );
	WgVectorHook * InsertChild( WgWidget * pWidget, WgWidget * pSibling );
	WgVectorHook * InsertChildSorted( WgWidget * pWidget );

	bool			DeleteChild( WgWidget * pWidget );
	WgWidget *		ReleaseChild( WgWidget * pWidget );

	bool			DeleteAllChildren();
	bool			ReleaseAllChildren();

	void			SortChildren();
	void			SetSortOrder( WgSortOrder order );
	WgSortOrder		GetSortOrder() const { return m_sortOrder; }

	void			SetSortFunction( WgWidgetSortFunc pSortFunc );
	WgWidgetSortFunc	SortFunction() const { return m_pSortFunc; }

    void            HideChildren( int index, int amount );
    void            UnhideChildren( int index, int amount );

    
protected:

	void			_onCloneContent( const WgWidget * _pOrg );

	WgHook*			_firstHook() const { return m_hooks.first(); }
	WgHook*			_lastHook() const { return m_hooks.last(); }

	int				_compareWidgets(const WgWidget * p1, const WgWidget * p2) { return m_pSortFunc?m_pSortFunc(p1,p2):0; }

	// To be overloaded by subclasses

	virtual WgRect	_hookGeo( const WgVectorHook * pHook ) = 0;
	virtual void	_onResizeRequested( WgVectorHook * pHook ) = 0;
	virtual void	_onRenderRequested( WgVectorHook * pHook ) = 0;
	virtual void	_onRenderRequested( WgVectorHook * pHook, const WgRect& rect ) = 0;
	virtual void	_onWidgetAppeared( WgVectorHook * pInserted ) = 0;				// so parent can update geometry and possibly request render.
	virtual void    _onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast ) = 0;                // so parent can update geometry and possibly request
	virtual void	_onWidgetDisappeared( WgVectorHook * pToBeRemoved ) = 0;		// so parent can update geometry and possibly request render.
    virtual void    _onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast ) = 0;        // so parent can update geometry and possibly request render.
	virtual void	_onWidgetsReordered() = 0;
	virtual void	_refreshAllWidgets() = 0;
	virtual WgVectorHook * _newHook() = 0;

	//

	wg::Chain<WgVectorHook>	m_hooks;

	WgSortOrder			m_sortOrder;
	WgWidgetSortFunc	m_pSortFunc;
	int					m_nChildPanels;					// Number of VISIBLE children that are panels.
};



#endif //WG2_VECTORPANEL_DOT_H
