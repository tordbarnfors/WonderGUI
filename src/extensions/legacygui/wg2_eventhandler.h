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
#ifndef WG_EVENTHANDLER_DOT_H
#define WG_EVENTHANDLER_DOT_H

#include <deque>
#include <map>
#include <vector>

#ifndef WG_EVENT_DOT_H
#	include <wg2_event.h>
#endif

#ifndef WG_EVENTFILTER_DOT_H
#	include <wg2_eventfilter.h>
#endif


#ifndef WG2_KEY_DOT_H
#	include <wg2_key.h>
#endif

#include <wg_chain.h>

class WgRoot;
class WgPanel;

class WgEventListener
{
public:
	virtual ~WgEventListener() {};

	virtual void OnEvent( const WgEvent::Event * pEvent ) = 0;
};


struct WgModKeyMap
{
	WgModifierKeys       stepWord        = WG_MODKEY_STD_CTRL;
	WgModifierKeys       beginEndLine    = WG_MODKEY_NONE;
	WgModifierKeys       beginEndText    = WG_MODKEY_NONE;
//    WgModifierKeys       pageUpDown      = WG_MODKEY_NONE;
};


class WgEventHandler
{
friend class WgWidget;

public:
	WgEventHandler( WgRoot * pRoot );
	~WgEventHandler();

	bool	QueueEvent( WgEvent::Event * pEvent );

	void	ProcessEvents();

	void	IgnoreNextCharacterEvent();			// Hack to allow us to forget a /n Character event following a WG_KEY_RETURN that switched focus for us.
												// In WG3 we should have something similar in InputHandler instead (ignoreNextText?) or some generic intercept msg callback.

	//----

	bool	SetFocusGroup( WgPanel * pFocusGroup );
	bool	SetKeyboardFocus( WgWidget * pFocus );

	WgPanel *	FocusGroup() const;
	WgWidget *			KeyboardFocus() const { return m_keyFocusWidget.GetRealPtr(); }

	//----

	bool	IsMouseButtonPressed( int button );
	bool	IsKeyPressed( int native_keycode );
	WgModifierKeys ModKeys() const { return m_modKeys; }

	void                SetModKeyMap( const WgModKeyMap& key );
	const WgModKeyMap&  GetModKeyMap() const { return m_modKeyMap; }

	bool	IsWindowFocused() const { return m_bWindowFocus; }

	WgCoord PointerPixelPos() const { return m_pointerPos; }

	//----

	void	AddCallback( void(*fp)( const WgEvent::Event * pEvent) );
	void	AddCallback( void(*fp)( const WgEvent::Event * pEvent, void * pParam), void * pParam );
	void	AddCallback( void(*fp)( const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest );
	void	AddCallback( WgEventListener * pListener );

	void	AddCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent) );
	void	AddCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent, void * pParam), void * pParam );
	void	AddCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest );
	void	AddCallback( const WgEventFilter& filter, WgEventListener * pListener );

	int		DeleteCallbacksTo( const WgWidget * pWidget );
	int		DeleteCallbacksTo( const WgEventListener * pListener );
	int		DeleteCallbacksTo( void(*fp)( const WgEvent::Event * pEvent) );
	int		DeleteCallbacksTo( void(*fp)( const WgEvent::Event * pEvent, void * pParam) );

	int		DeleteCallbacksOn( const WgWidget * pWidget );
	int		DeleteCallbacksOn( const WgEventType type );
	int		DeleteCallbacksOn( const WgWidget * pWidget, WgEventType type );

	int		DeleteCallback( const WgEventFilter& filter, const WgWidget * pWidget );
	int		DeleteCallback( const WgEventFilter& filter, const WgEventListener * pListener );
	int		DeleteCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent) );
	int		DeleteCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent, void * pParam) );


	int		DeleteAllCallbacks();
	int		DeleteDeadCallbacks();

	//----

	bool	ForwardEvent( const WgEvent::Event * pEvent );
	bool	ForwardEvent( const WgEvent::Event * pEvent, WgWidget * pRecipient );

    void    HidePointer();
    void    ShowPointer();
    
	WgPointerStyle	PointerStyle() const { return m_pointerStyle; }

	
    bool    isPointerHidden() const { return m_pointerHideCount > 0; }
    
    bool    isPointerLocked() const { return m_pWidgetLockedPointer; }
    
    WgWidget * LockedPointerWidget() const { return m_pWidgetLockedPointer.GetRealPtr(); }
    
private:
	class	Callback;
    
	void 	_postTickEvents( int ticks );
	void 	_processEventQueue();

	bool    _forwardEvent( const WgEvent::Event * _pEvent );

	void	_finalizeEvent( WgEvent::Event * pEvent );
	void	_processGeneralEvent( WgEvent::Event * pEvent );
	void	_processEventCallbacks( WgEvent::Event * pEvent );

	void	_processTick( WgEvent::Tick * pEvent );

	void	_processFocusGained( WgEvent::FocusGained * pEvent );
	void	_processFocusLost( WgEvent::FocusLost * pEvent );

	void	_processMouseEnter( WgEvent::MouseEnter * pEvent );
	void	_processMouseMove( WgEvent::MouseMove * pEvent );
	void	_processMousePosition( WgEvent::MousePosition * pEvent );
	void	_processMouseLeave( WgEvent::MouseLeave * pEvent );

	void	_processMouseButtonPress( WgEvent::MouseButtonPress * pEvent );
	void	_processMouseButtonRepeat( WgEvent::MouseButtonRepeat * pEvent );
	void	_processMouseButtonDrag( WgEvent::MouseButtonDrag * pEvent );
	void	_processMouseButtonRelease( WgEvent::MouseButtonRelease * pEvent );
	void	_processMouseButtonClick( WgEvent::MouseButtonClick * pEvent );
	void	_processMouseButtonDoubleClick( WgEvent::MouseButtonDoubleClick * pEvent );

	void	_processMouseWheelRoll( WgEvent::MouseWheelRoll * pEvent );

	void	_processKeyPress( WgEvent::KeyPress * pEvent );
	void	_processKeyRepeat( WgEvent::KeyRepeat * pEvent );
	void	_processKeyRelease( WgEvent::KeyRelease * pEvent );

	void	_processCharacter( WgEvent::Character * pEvent );

	bool	_isWidgetInList( const WgWidget * pWidget, const std::vector<WgWidgetWeakPtr>& list );
	int		_widgetPosInList( const WgWidget * pWidget, const std::vector<WgWidgetWeakPtr>& list );

	void	_addCallback( const WgEventFilter& filter, Callback * pCallback );
	int		_deleteCallbacksTo( const void * pReceiver );
	int		_deleteCallbacksOnType( WgEventType type, wg::Chain<Callback> * pChain );
	int		_deleteCallback( const WgEventFilter& filter, const void * pReceiver );

	void 	_updateMarkedWidget(bool bMouseMoved);
	WgWidget * _updateEnteredWidgets( WgWidget * pMarkedWidget );

	void	_addTickReceiver( WgWidget * pWidget );
	bool    _handleEavesdropping( WgWidget * pReceiver, WgEvent::Event * pEvent );

    bool    _widgetLockPointer(WgWidget * pWidget);
    void    _widgetReleasePointer(WgWidget * pWidget);

	//

	WgRoot *		m_pRoot;

	std::deque<WgEvent::Event*>				m_eventQueue;
	bool									m_bIsProcessing;	// Set when we are inside ProcessEvents().
	std::deque<WgEvent::Event*>::iterator	m_insertPos;		// Position where we insert events being queued when processing.
	bool									m_bIgnoreNextCharacterEvent;

	int64_t			m_time;			// Time in microseconds
	WgCoord			m_pointerPos;
	WgPointerStyle	m_pointerStyle;
	WgModifierKeys	m_modKeys;

	WgModKeyMap     m_modKeyMap;

	std::vector<WgWidgetWeakPtr>	m_vTickWidgets;		// Widgets that have requested periodic tick-events (i.e. on every processEvents() ).

	// Current mouse state

	WgWidgetWeakPtr				m_pMarkedWidget;		// Widget the pointer currently is "inside". Empty if outside a modal widget.

	std::vector<WgWidgetWeakPtr>	m_vEnteredWidgets;	// All widgets that pointer is considered to be inside (= markedWidget + its ancestors).

	int                         m_markedLockCountdown = 0;      // Millisec left for which the mouse is locked to the marked widget.

    int                         m_pointerHideCount = 0;          // 0 = pointer showing, 1+ pointer is hidden.
    
	// Current button states

	bool						m_bButtonPressed[WG_MAX_BUTTONS+1];

	WgEvent::MouseButtonPress *	m_pLatestPressEvents[WG_MAX_BUTTONS+1];			// Saved info for the last time each button was pressed.
	WgEvent::MouseButtonRelease *	m_pLatestReleaseEvents[WG_MAX_BUTTONS+1];	// Saved info for the last time each button was released.

	WgWidgetWeakPtr				m_latestPressWidgets[WG_MAX_BUTTONS+1];		// Widget that received the latest press, for each button.
	WgWidgetWeakPtr				m_previousPressWidgets[WG_MAX_BUTTONS+1];	// Widget that received the second latest press, for each button,
																			// used for double-click handling.
    WgWidgetWeakPtr             m_pWidgetLockedPointer;
    
	// Current keyboard state

	struct KeyDownInfo
	{
		WgEvent::KeyPress * 	pEvent;
		WgWidgetWeakPtr			pWidget;					// Widget that received the press event.
	};

	std::vector<KeyDownInfo*>	m_keysDown;				// One entry for each currently depressed key, in order of being pressed.

	bool						m_bWindowFocus;			// Set if we have window focus.
	WgWidgetWeakPtr				m_keyFocusGroup;		// Current focus group (if any).
	WgWidgetWeakPtr				m_keyFocusWidget;		// Widget currently having the keyboard focus.

	std::map<WgWidgetWeakPtr,WgWidgetWeakPtr>	m_focusGroupMap;	// Mapping focus groups (key) with their currently focused Widget (value).


	// Callbacks

	class Callback : public wg::Link
	{
	public:
		virtual ~Callback() {};

		LINK_METHODS(Callback);

		virtual void 			ProcessEvent( const WgEvent::Event * pEvent ) = 0;
		virtual bool 			IsAlive() const = 0;
		virtual void * 			Receiver() const = 0;
		inline const WgEventFilter& 	Filter() const { return m_filter; }

	protected:
		WgEventFilter		m_filter;
	};


	class WidgetCallback : public Callback
	{
	public:
		WidgetCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest );

		void 	ProcessEvent( const WgEvent::Event * pEvent );
		bool 	IsAlive() const;
		void *	Receiver() const;

	private:
		void(*m_pFunction)(const WgEvent::Event * pEvent, WgWidget * pDest);
		WgWidgetWeakPtr	m_pWidget;		// Destination Widget, not source as in the event.
	};

	class FunctionCallbackParam : public Callback
	{
	public:
		FunctionCallbackParam( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, void * pParam), void * pParam );

		void 	ProcessEvent( const WgEvent::Event * pEvent );
		bool 	IsAlive() const;
		void *	Receiver() const;

	private:
		void(*m_pFunction)( const WgEvent::Event * pEvent, void * pParam);
		void *			m_pParam;
	};

	class FunctionCallback : public Callback
	{
	public:
		FunctionCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent) );

		void 	ProcessEvent( const WgEvent::Event * pEvent );
		bool 	IsAlive() const;
		void *	Receiver() const;

	private:
		void(*m_pFunction)( const WgEvent::Event * pEvent);
	};

	class ListenerCallback : public Callback
	{
	public:
		ListenerCallback( const WgEventFilter& filter, WgEventListener * pListener );

		void 	ProcessEvent( const WgEvent::Event * pEvent );
		bool 	IsAlive() const;
		void *	Receiver() const;

	private:
		WgEventListener *	m_pListener;
	};


	wg::Chain<Callback>						m_globalCallbacks;	// Callbacks called for every event.

	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >	m_widgetCallbacks;	// Callbacks for Widget-specific events.


};




#endif //WG_EVENTHANDLER_DOT_H
