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
#ifndef WG2_STACKPANEL_DOT_H
#define WG2_STACKPANEL_DOT_H

#ifndef WG2_VECTORPANEL_DOT_H
#	include <wg2_vectorpanel.h>
#endif

class WgStackPanel;

class WgStackHook : public WgVectorHook
{
	friend class WgStackPanel;
public:
	const char *Type( void ) const;
	static const char * ClassType();

	enum SizePolicy
	{
		DEFAULT,
		STRETCH,
		SCALE
	};

	void			SetSizePolicy( SizePolicy policy );
	SizePolicy		GetSizePolicy() const { return m_sizePolicy; }

	void			SetOrigo( WgOrigo origo );
	WgOrigo	Origo() const { return m_origo; }

	WgStackHook * 	Prev() const { return _prev(); }
	WgStackHook * 	Next() const { return _next(); }

	WgStackPanel * 	Parent() const { return m_pParent; }

protected:
	PROTECTED_LINK_METHODS( WgStackHook );

	WgStackHook( WgStackPanel * pParent );

	WgContainer * _parent() const;

	WgRect			_getGeo( const WgRect& parentGeo ) const;

	WgStackPanel *	m_pParent;

	WgOrigo	m_origo;
	SizePolicy		m_sizePolicy;

};

class WgStackPanel : public WgVectorPanel
{
friend class WgStackHook;

public:
	WgStackPanel();
	virtual ~WgStackPanel();

	const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgStackPanel(); };


	inline WgStackHook * AddChild( WgWidget * pWidget ) { return static_cast<WgStackHook*>(WgVectorPanel::AddChild(pWidget)); }
	inline WgStackHook * AddChildren( WgWidget * pWidgets[], int nb ) { return static_cast<WgStackHook*>(WgVectorPanel::AddChildren(pWidgets, nb)); }
	inline WgStackHook * InsertChild( WgWidget * pWidget, WgWidget * pSibling ) { return static_cast<WgStackHook*>(WgVectorPanel::InsertChild(pWidget,pSibling)); }
	inline WgStackHook * InsertChildSorted( WgWidget * pWidget ) { return static_cast<WgStackHook*>(WgVectorPanel::InsertChildSorted(pWidget)); }

	inline WgStackHook* FirstHook() const { return static_cast<WgStackHook*>(m_hooks.first()); }
	inline WgStackHook* LastHook() const { return static_cast<WgStackHook*>(m_hooks.last()); }

	// Overloaded from WgWidget

	int		MatchingPixelHeight( int width ) const;
	int		MatchingPixelWidth( int height ) const;

	WgSize	PreferredPixelSize() const;

protected:

	// Overloaded from Widget

//	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onNewSize( const WgSize& size );

	// Overloaded from WgPanel

	WgHook *	_firstHookWithGeo( WgRect& writeGeo ) const;
	WgHook *	_nextHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const;

	WgHook *	_lastHookWithGeo( WgRect& writeGeo ) const;
	WgHook *	_prevHookWithGeo( WgRect& writeGeo, WgHook * pHook ) const;

	// Overloaded from WgVectorPanel

	WgRect	_hookGeo( const WgVectorHook * pHook );
	void	_onResizeRequested( WgVectorHook * pHook );
	void	_onRenderRequested( WgVectorHook * pHook );
	void	_onRenderRequested( WgVectorHook * pHook, const WgRect& rect );
	void	_onWidgetAppeared( WgVectorHook * pInserted );				// so parent can update geometry and possibly request render.
	void    _onWidgetsAppeared( WgVectorHook * pFirst, WgVectorHook * pLast );
	void	_onWidgetDisappeared( WgVectorHook * pToBeRemoved );		// so parent can update geometry and possibly request render.
    void    _onWidgetsDisappeared( WgVectorHook * pFirst, WgVectorHook * pLast );
	void	_onWidgetsReordered();
	void	_refreshAllWidgets();
	WgVectorHook * _newHook();

	// Internal to WgStackPanel

	void 	_refreshPreferredPixelSize();
	void	_adaptChildrenToSize();
	void	_renderFromChildOnward( WgVectorHook * pHook );


	WgSize	m_size;
	WgSize	m_preferredSize;

};



#endif //WG2_STACKPANEL_DOT_H
