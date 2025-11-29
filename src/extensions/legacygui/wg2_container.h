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
#ifndef	WG2_CONTAINER_DOT_H
#define	WG2_CONTAINER_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_WIDGETHOLDER_DOT_H
#	include <wg2_widgetholder.h>
#endif

class WgContainer : public WgWidgetHolder, public WgWidget
{
	friend class WgEventHandler;

	friend class WgHook;
	friend class WgFlexHook;
	friend class WgModalHook;

	friend class WgMenu;
	friend class WgMenubar;
	friend class WgCombobox;

	friend class WgRootPanel;
	friend class WgFlexPanel;
	friend class WgModalLayer;
	friend class WgDragNDropLayer;
	friend class WgTablePanel;
	friend class WgScrollPanel;
	friend class WgStackPanel;
	friend class WgVectorPanel;
	friend class WgListPanel;
	friend class WgCapsule;
	friend class WgPopupOpener;

	public:


		virtual WgWidget * 	FindWidget( const WgCoord& ofs, WgSearchMode mode );

		bool		IsWidget() const;
		bool		IsContainer() const;

		WgWidget *				CastToWidget();
		const WgWidget *		CastToWidget() const;
		WgContainer *			CastToContainer();
		const WgContainer *		CastToContainer() const;

		// The following need to be here to tackle ambigious access to identical methods in WgWidget and WgWidgetHolder.

		virtual bool			IsPanel() const;
		virtual WgPanel *		CastToPanel();
		virtual const WgPanel *	CastToPanel() const;

		virtual bool			IsCapsule() const;
		virtual WgCapsule *		CastToCapsule();
		virtual const WgCapsule *	CastToCapsule() const;

		virtual bool			IsLayer() const;
		virtual WgLayer *		CastToLayer();
		virtual const WgLayer *	CastToLayer() const;

		void					SetScale( int scale );
		int						Scale() const { return m_scale; }

		void                    SetEavesdropping( bool bEavesdropping );
		bool                    IsEavesdropping() const { return m_bEavesdropping; }

	protected:
		WgContainer();
		virtual ~WgContainer() {};

		virtual void	_onEnable();
		virtual void	_onDisable();

		bool			_descendantPos(WgWidget * pDescendant, WgCoord& pos);

		virtual void	_onNewRoot( WgRoot * pRoot );

		virtual void	_setScale( int scale );

		virtual void	_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );
		virtual WgHook* _firstHookWithGeo( WgRect& geo ) const = 0;
		virtual WgHook* _nextHookWithGeo( WgRect& geo, WgHook * pHook ) const = 0;

		virtual WgHook* _lastHookWithGeo( WgRect& geo ) const = 0;
		virtual WgHook* _prevHookWithGeo( WgRect& geo, WgHook * pHook ) const = 0;

		bool 			_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting );	// Needed until WgPanel inherits from WgWidget
		bool 			_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing );		// Needed until WgPanel inherits from WgWidget

		void            _inViewRequested( WgHook * pChild );
		void            _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea );

		virtual WgModalLayer *	_getModalLayer() const;
		virtual WgPopupLayer*	_getPopupLayer() const;

		virtual void	_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );
		virtual void	_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );

		virtual void	_onCloneContent( const WgContainer * _pOrg );

		bool		m_bSiblingsOverlap;	// Set if children (might be) overlapping each other (special considerations to be taken during rendering).
		bool        m_bEavesdropping;
};

#endif //WG2_CONTAINER_DOT_H
