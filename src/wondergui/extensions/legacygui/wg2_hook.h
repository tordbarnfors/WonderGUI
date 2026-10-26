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
#ifndef WG2_HOOK_DOT_H
#define WG2_HOOK_DOT_H

#ifndef	WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif



class WgWidgetHolder;
class WgContainer;
class WgWidget;
class WgRoot;
class WgRectLink;
class WgEventHandler;

class WgHook
{
	friend class WgWidget;
	friend class WgLayer;
	friend class WgPanel;
	friend class WgVectorPanel;

public:

	virtual WgCoord		PointPos() const;
	virtual WgSize		PointSize() const;
	virtual WgRect		PointGeo() const;
	virtual WgCoord		ScreenPointPos() const;
	virtual WgRect		ScreenPointGeo() const;


	virtual WgCoord	PixelPos() const = 0;
	virtual WgSize	PixelSize() const = 0;
	virtual WgRect	PixelGeo() const = 0;
	virtual WgCoord	ScreenPixelPos() const = 0;
	virtual WgRect	ScreenPixelGeo() const = 0;

	WgHook *		Prev() const { return _prevHook(); }
	WgHook *		Next() const { return _nextHook(); }

	WgWidget *				Widget() const { return m_pWidget; }
	WgWidgetHolder * 		Holder() const { return _holder(); }
	WgContainer * 			Parent() const { return _parent(); }

	virtual WgRoot *	Root() const;
	WgEventHandler *	EventHandler() const;

	virtual const char *Type( void ) const = 0;

protected:

	WgHook() : m_pWidget(0) {}
	virtual ~WgHook();

	virtual void	_attachWidget( WgWidget * pWidget );				// Make sure Widget links us. Call when hook has been relocated.
	void			_relinkWidget();
	WgWidget*		_releaseWidget();								//

	// To be called by Widget

	virtual void	_requestRender() = 0;
	virtual void	_requestRender( const WgRect& rect ) = 0;
	virtual void	_requestResize() = 0;

	virtual bool	_requestFocus();
	virtual bool	_releaseFocus();

	virtual void    _requestInView();
	virtual void    _requestInView( const WgRect& mustHaveArea, const WgRect& niceToHaveArea );

	virtual bool	_isDisplayed() const;
	
	//

	virtual WgHook *	_prevHook() const = 0;
	virtual WgHook *	_nextHook() const = 0;
	virtual WgWidgetHolder * _holder() const = 0;
	virtual WgContainer * _parent() const = 0;

	WgWidget *		m_pWidget;
};

template<typename T> T* WgCast(WgHook * pHook)
{
	if(pHook)
	{
		if(T::ClassType() == pHook->Type())
			return static_cast<T*>(pHook);
	}
	return 0;
}

#endif //WG2_HOOK_DOT_H
