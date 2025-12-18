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
#ifndef	WG2_WIDGETHOLDER_DOT_H
#define	WG2_WIDGETHOLDER_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#include <wg2_geo.h>

class WgWidget;
class WgRoot;
class WgHook;
class WgContainer;
class WgPanel;
class WgCapsule;
class WgLayer;
class WgModalLayer;
class WgPopupLayer;

class WgWidgetHolder
{
	friend class WgPanel;
	friend class WgHook;
	friend class WgContainer;
	public:
		WgWidget *	FirstWidget() const;
		WgWidget *	LastWidget() const;

		inline WgHook *	FirstHook() const { return _firstHook(); }
		inline WgHook *	LastHook() const { return _lastHook(); }

		virtual WgWidget *	FindWidget( const WgCoord& ofs, WgSearchMode mode ) = 0;

		virtual bool		DeleteChild( WgWidget * pWidget ) = 0;
		virtual WgWidget *	ReleaseChild( WgWidget * pWidget ) = 0;

		virtual bool		DeleteAllChildren() = 0;
		virtual bool		ReleaseAllChildren() = 0;

		virtual WgWidget *					CastToWidget();
		virtual const WgWidget *			CastToWidget() const;
		virtual WgContainer *				CastToContainer();
		virtual const WgContainer *			CastToContainer() const;
		virtual WgPanel *					CastToPanel();
		virtual const WgPanel *				CastToPanel() const;
		virtual WgCapsule *					CastToCapsule();
		virtual const WgCapsule *			CastToCapsule() const;
		virtual WgLayer *					CastToLayer();
		virtual const WgLayer *				CastToLayer() const;
		virtual WgRoot *					CastToRoot();
		virtual const WgRoot *				CastToRoot() const;

		virtual bool		IsRoot() const;
		virtual bool		IsWidget() const;
		virtual bool		IsContainer() const;
		virtual bool		IsPanel() const;
		virtual bool		IsCapsule() const;
		virtual bool		IsLayer() const;

	protected:

		virtual bool 		_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting ) = 0;
		virtual bool 		_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing ) = 0;

		virtual void        _inViewRequested( WgHook * pChild ) = 0;
		virtual void        _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea ) = 0;

		virtual WgHook*		_firstHook() const = 0;
		virtual WgHook*		_lastHook() const = 0;
};


#endif //WG2_WIDGETHOLDER_DOT_H
