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
#ifndef WG2_POPUPLAYER_DOT_H
#define WG2_POPUPLAYER_DOT_H
#pragma once

#ifndef WG2_LAYER_DOT_H
	#include <wg2_layer.h>
#endif

#include <wg_chain.h>

class WgPopupLayer;

typedef class WgWeakPtr<WgWidget> Widget_wp;


//____ WgPopupHook ___________________________________________________________

class WgPopupHook : public WgLayerHook, protected wg::Link		/** @private */
{
	friend class WgPopupLayer;
	friend class wg::Chain<WgPopupHook>;

public:
	const char *Type(void) const;
	static const char * ClassType();

	// Standard Hook methods

	WgPopupHook *	Prev() const { return _prev(); }
	WgPopupHook *	Next() const { return _next(); }

	WgPopupLayer *	Parent() const;


protected:
	PROTECTED_LINK_METHODS(WgPopupHook);

	WgPopupHook(WgPopupLayer * pParent) : m_pParent(pParent), attachPoint(WgOrigo::NorthWest), maxSize(INT_MAX,INT_MAX) {}

	void		_requestRender();
	void		_requestRender(const WgRect& rect);
	void		_requestResize();

	WgHook *	_prevHook() const;
	WgHook *	_nextHook() const;
	WgContainer* _parent() const;


	WgPopupLayer * m_pParent;


	enum class State
	{
		OpeningDelay,			// Popup is in "delayed opening" mode. Some ms before it starts to open.
		Opening,				// Popup is opening (fading in).
		PeekOpen,				// Popup is open until pointer leaves launcherGeo (mode only allowed if bAutoClose is set).
		WeakOpen,				// Popup is open, but closed if other entry of ancestors is peeked (mode only allowed if bAutoClose is set).
		FixedOpen,				// Popup is open until it is closed by a pop() call.
		ClosingDelay,			// Popup is in countdown to closing mode (mode only allowed if bAutoClose is set).
		Closing,				// Popup is closing (fading out).
	};

	WgRect		launcherGeo;		// Launcher geo relative sibling or parent, measured in pixels.
	WgOrigo		attachPoint;
	WgCoord		attachOfs;			// Offset in points from attachPoint.
	bool		bAutoClose;			// Has been opened in auto-close mode.
	bool        bOpened;            // Set once the popup has passed state OpeningDelay.
	State		state;
	int			stateCounter;		// Counts millisec the slot has been in a transitative state (Delay, Opening, Coundown and Closing).
	WgSize		maxSize;
	Widget_wp	pOpener;			// Widget that opened this popup.
	Widget_wp	pKeyFocus;			// Pointer at widget that held focus when this popup was ontop.
};



//____ WgPopupLayer ____________________________________________________________

class WgPopupLayer : public WgLayer
{
	friend class WgPopupHook;
public:
	WgPopupLayer();
	virtual ~WgPopupLayer();



	virtual const char *Type(void) const override;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const override { return new WgPopupLayer(); };


	int				NbPopups() const;
	void			Push(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint = WgOrigo::NorthEast, WgCoord attachOffset = { 0,0 }, bool bAutoClose = false, bool bDelay = false, WgSize maxSize = WgSize(INT_MAX, INT_MAX));
	void			Pop(int nb = 1);
	void			Pop(WgWidget * pPopup);
	void			Clear();

	WgWidget * 		FindWidget(const WgCoord& ofs, WgSearchMode mode) override;

	bool			DeleteChild(WgWidget * pWidget) override { return 0; }
	WgWidget *		ReleaseChild(WgWidget * pWidget) override { Pop(pWidget); return pWidget; }	// Needs to be here since called by Hook when Child is deleted.

	bool			DeleteAllChildren() override { return 0; }
	bool			ReleaseAllChildren() override { return 0; }

	void			SetDelays(int popupOpenDelay, int popupFadeInDelay, int popupCloseDelay, int popupFadeOutDelay);

protected:

	WgPopupLayer *	_getPopupLayer() const override { return const_cast<WgPopupLayer*>(this); }

	void			_stealKeyboardFocus();
	void			_restoreKeyboardFocus();
	bool			_updateGeo(WgPopupHook * pSlot, bool bInitialUpdate = false );

	void			_removeSlots(int nb);
	void			_removeSlot(WgPopupHook * p);

	void			_addSlot(WgWidget * pPopup, WgWidget * pOpener, const WgRect& launcherGeo, WgOrigo attachPoint, WgCoord attachOfs, bool bAutoClose, bool bDelay, WgSize maxSize);

	void			_closeAutoOpenedUntil(WgWidget * pStayOpen, bool bCloseImmediately );
	void            _closeAllOpenUntil(WgWidget * pStayOpen, bool bCloseImmediately);
	void            _beginClosing( WgPopupHook * p, bool bCloseImmediately );

	// Overloaded from Panel


	void			_childRequestResize(WgPopupHook * pSlot);
	void			_onRequestRender(const WgRect& rect, const WgPopupHook * pHook);	// rect is in our coordinate system.


	// Overloaded from Layer

	WgLayerHook *	_firstLayerHook() const override;

	WgHook*			_firstHook() const override;		// Fist Hook returned is the normal child, then follows the modal ones.
	WgHook*			_lastHook() const override;		//

	WgHook *		_firstHookWithGeo(WgRect& geo) const override;
	WgHook *		_nextHookWithGeo(WgRect& geo, WgHook * pHook) const override;

	WgHook *		_lastHookWithGeo(WgRect& geo) const override;
	WgHook *		_prevHookWithGeo(WgRect& geo, WgHook * pHook) const override;



	// Overloaded from container

	void			_renderPatches(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches) override;
//		void			_maskPatches(Patches& patches, const Rect& geo, const Rect& clip, BlendMode blendMode);


	// Overloaded from Widget

	void			_onCloneContent(const WgWidget * _pOrg) override;
	void			_onNewSize(const WgSize& size) override;
	void			_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;


	wg::Chain<WgPopupHook>	m_popupHooks;		// First menu lies at the bottom.
	Widget_wp				m_pKeyFocus;	// Pointer at child that held focus before any menu was opened.


	int				m_openingDelayMs = 150;
	int				m_openingFadeMs = 100;
	int				m_closingDelayMs = 100;
	int				m_closingFadeMs = 100;
};


#endif //WG2_POPUPLAYER_DOT_H
