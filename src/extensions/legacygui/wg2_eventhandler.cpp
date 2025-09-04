/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <assert.h>
#include <wg2_event.h>
#include <wg2_eventfilter.h>
#include <wg2_eventhandler.h>
#include <wg2_base.h>
#include <wg2_rootpanel.h>
#include <wg2_panel.h>

//____ Constructor ____________________________________________________________

WgEventHandler::WgEventHandler( WgRoot * pRoot )
{
	m_pRoot					= pRoot;
	m_time					= 0;
	m_modKeys				= WG_MODKEY_NONE;
	m_pointerStyle			= WgPointerStyle::Arrow;

	m_bIsProcessing			= false;
	m_bWindowFocus			= true;
	m_bIgnoreNextCharacterEvent = false;

	for( int i = 0 ; i <= WG_MAX_BUTTONS ; i++ )
	{
		m_bButtonPressed[i] = 0;
		m_pLatestPressEvents[i] = 0;
		m_pLatestReleaseEvents[i] = 0;
	}
}

//____ Destructor _____________________________________________________________

WgEventHandler::~WgEventHandler()
{
	while( !m_eventQueue.empty() )
	{
		// Delete all events in queue as we have ownership of them
		WgEvent::Event * pEvent = m_eventQueue.front();
		delete pEvent;

		m_eventQueue.pop_front();		
	}
}

//____ SetModKeyMap() _________________________________________________________

void WgEventHandler::SetModKeyMap( const WgModKeyMap& map )
{
	m_modKeyMap = map;
}

//____ SetFocusGroup() ________________________________________________________

bool WgEventHandler::SetFocusGroup( WgPanel * pFocusGroup )
{
	// Sanity checks

	if( pFocusGroup )
	{
		if( pFocusGroup == m_keyFocusGroup.GetRealPtr() )
			return true;									// Not an error, but we don't need to do anything

		if( !pFocusGroup->IsFocusGroup() )
			return false;									// Panel is not a focus group

		if( !pFocusGroup->Hook() || pFocusGroup->Hook()->Root() != m_pRoot )
			return false;									// pFocusGroup is not a child of our root.
	}

	// Set new focus widget as specified by group

	WgWidgetWeakPtr pNewFocusWidget;

	if( pFocusGroup )
		if( m_focusGroupMap.find(pFocusGroup) != m_focusGroupMap.end() )
			pNewFocusWidget = m_focusGroupMap[pFocusGroup];

	if( m_keyFocusWidget )
		m_keyFocusWidget->_onLostInputFocus();

	if( pNewFocusWidget )
		pNewFocusWidget->_onGotInputFocus();

	// Set members and exit

	m_keyFocusWidget = pNewFocusWidget;
	m_keyFocusGroup = pFocusGroup;

	return true;
}

//____ SetKeyboardFocus() _____________________________________________________

bool WgEventHandler::SetKeyboardFocus( WgWidget * pFocus )
{
	// Return if Widget is not child of our root.

	if( pFocus && (!pFocus->Hook() || pFocus->Hook()->Root() != m_pRoot) )
		return false;

	// Handle old focus.

	WgWidget * pOldFocus = m_keyFocusWidget.GetRealPtr();

	if( pFocus == pOldFocus )
		return true;

	if( pOldFocus )
		pOldFocus->_onLostInputFocus();

	// Handle new focus, possibly switching focus group.

	if( pFocus )
	{
		// Check what focus group (if any) this Widget belongs to.

		m_keyFocusGroup = 0;
		WgContainer * p = pFocus->Parent();
		while( p )
		{
			if( p->IsPanel() && p->CastToPanel()->IsFocusGroup() )
			{
				m_keyFocusGroup = p;
				break;
			}

			p = p->Parent();
		}

		// Activate focus

		pFocus->_onGotInputFocus();
	}

	// Set members and exit.

	m_keyFocusWidget = pFocus;
	m_focusGroupMap[m_keyFocusGroup] = pFocus;

	return true;
}

//____ FocusGroup() ___________________________________________________________

WgPanel * WgEventHandler::FocusGroup() const
{
	WgWidget * pWidget = m_keyFocusGroup.GetRealPtr();
	if( pWidget )
		return pWidget->CastToPanel();

	return 0;
}

//____ IsMouseButtonPressed() _________________________________________________________

bool WgEventHandler::IsMouseButtonPressed( int button )
{
	if( button >= 1 && button <= WG_MAX_BUTTONS )
		return m_bButtonPressed[button];

	return false;
}

//____ IsKeyPressed() ____________________________________________________________

bool WgEventHandler::IsKeyPressed( int native_keycode )
{
	for( unsigned int i = 0 ; i < m_keysDown.size() ; i++ )
		if( native_keycode == m_keysDown[i]->pEvent->NativeKeyCode() )
			return true;

	return false;
}



//____ AddCallback() __________________________________________________________

void WgEventHandler::AddCallback( void(*fp)(const WgEvent::Event * pEvent) )
{
	Callback * p = new FunctionCallback( WgEventFilter(), fp );
	m_globalCallbacks.pushBack( p );
}

void WgEventHandler::AddCallback( void(*fp)(const WgEvent::Event * pEvent, void * pParam), void * pParam )
{
	Callback * p = new FunctionCallbackParam( WgEventFilter(), fp, pParam );
	m_globalCallbacks.pushBack( p );
}

void WgEventHandler::AddCallback( void(*fp)(const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest )
{
	Callback * p = new WidgetCallback( WgEventFilter(), fp, pDest );
	m_globalCallbacks.pushBack( p );
}

void WgEventHandler::AddCallback( WgEventListener * pListener )
{
	Callback * p = new ListenerCallback( WgEventFilter(), pListener );
	m_globalCallbacks.pushBack( p );
}

void WgEventHandler::AddCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent) )
{
	Callback * p = new FunctionCallback( filter, fp );
	_addCallback( filter, p );
}

void WgEventHandler::AddCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, void * pParam), void * pParam )
{
	Callback * p = new FunctionCallbackParam( filter, fp, pParam );
	_addCallback( filter, p );
}

void WgEventHandler::AddCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest )
{
	Callback * p = new WidgetCallback( filter, fp, pDest );
	_addCallback( filter, p );
}

void WgEventHandler::AddCallback( const WgEventFilter& filter, WgEventListener * pListener )
{
	Callback * p = new ListenerCallback( filter, pListener );
	_addCallback( filter, p );
}

//____ DeleteCallbacksTo() _______________________________________________________

int WgEventHandler::DeleteCallbacksTo( const WgWidget * pWidget )
{
	// Delete all callbacks with the specified Widget as recipient.

	return _deleteCallbacksTo(pWidget);
}

int WgEventHandler::DeleteCallbacksTo( void(*fp)( const WgEvent::Event * pEvent) )
{
	return _deleteCallbacksTo( reinterpret_cast<void*>(fp) );
}

int WgEventHandler::DeleteCallbacksTo( void(*fp)( const WgEvent::Event * pEvent, void * pParam) )
{
	return _deleteCallbacksTo( reinterpret_cast<void*>(fp) );
}

int WgEventHandler::DeleteCallbacksTo( const WgEventListener * pListener )
{
	return _deleteCallbacksTo(pListener);
}

//____ DeleteCallbacksOn() _______________________________________________________

int WgEventHandler::DeleteCallbacksOn( const WgWidget * pWidget )
{
	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.find(WgWidgetWeakPtr( const_cast<WgWidget*>(pWidget) ));

	if( it == m_widgetCallbacks.end() )
		return 0;

	int nDeleted = it->second.size();
	m_widgetCallbacks.erase(it);
	return nDeleted;
}

int WgEventHandler::DeleteCallbacksOn( WgEventType type )
{
	// Delete global callbacks on this event type

	int nDeleted = _deleteCallbacksOnType( type, &m_globalCallbacks );

	// Delete Widget-specific callbacks on this event type

	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.begin();
	while( it != m_widgetCallbacks.end() )
	{
		nDeleted += _deleteCallbacksOnType( type, &it->second );

		if( it->second.size() == 0 )
			it = m_widgetCallbacks.erase(it);
		else
			++it;
	}
	return nDeleted;
}


int WgEventHandler::DeleteCallbacksOn( const WgWidget * pWidget, WgEventType type )
{
	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.find( WgWidgetWeakPtr(const_cast<WgWidget*>(pWidget)));

	if( it == m_widgetCallbacks.end() )
		return 0;

	int nDeleted = 0;
	Callback * p = it->second.first();
	while( p )
	{
		if( p->Filter().EventType() == type )
		{
			Callback * pNext = p->next();
			delete p;
			nDeleted++;
			p = pNext;
		}
		else
			p = p->next();
	}

	if( it->second.size() == 0 )
		m_widgetCallbacks.erase(it);

	return nDeleted;
}



//____ DeleteCallbacks() ______________________________________________________

int WgEventHandler::DeleteCallback( const WgEventFilter& filter,const  WgWidget * pWidget )
{
	return _deleteCallback( filter, pWidget );
}

int WgEventHandler::DeleteCallback( const WgEventFilter& filter, const WgEventListener * pListener )
{
	return _deleteCallback( filter, pListener );
}

int WgEventHandler::DeleteCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent) )
{
	return _deleteCallback( filter, (const void *) fp );
}

int WgEventHandler::DeleteCallback( const WgEventFilter& filter, void(*fp)( const WgEvent::Event * pEvent, void * pParam) )
{
	return _deleteCallback( filter, (const void *) fp );
}



//____ DeleteAllCallbacks() ___________________________________________________

int WgEventHandler::DeleteAllCallbacks()
{
	m_globalCallbacks.clear();
	m_widgetCallbacks.clear();
	return false;
}

//____ DeleteDeadCallbacks() __________________________________________________

int WgEventHandler::DeleteDeadCallbacks()
{
	int nDeleted = 0;

	// Delete any dead global callbacks

	Callback * p = m_globalCallbacks.first();
	while( p )
	{
		if( p->IsAlive() )
			p = p->next();
		else
		{
			Callback * pNext = p->next();
			delete p;
			nDeleted++;
			p = pNext;
		}
	}

	// Delete any dead Widget-specific callbacks.
	// These can be dead by either sender or receiver having been deleted.

	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.begin();

	while( it != m_widgetCallbacks.end() )
	{
		if( !it->first )
		{
			nDeleted += it->second.size();
			it = m_widgetCallbacks.erase(it);		// Sender is dead, delete whole branch of callbacks.
		}
		else
		{
			Callback * p = it->second.first();
			while( p )
			{
				if( p->IsAlive() )
					p = p->next();
				else
				{
					Callback * pNext = p->next();
					delete p;					// Receiver is dead, delete callback.
					nDeleted++;
					p = pNext;
				}
			}
			++it;
		}
	}

	return nDeleted;
}

//____ HidePointer() _________________________________________________________

void WgEventHandler::HidePointer()
{
    if( m_pointerHideCount == 0 )
    {
        WgBase::hostBridge()->hidePointer();
    }
    
    m_pointerHideCount++;
}

//____ ShowPointer() __________________________________________________________

void WgEventHandler::ShowPointer()
{
    m_pointerHideCount--;
    
    if( m_pointerHideCount == 0 )
    {
        WgBase::hostBridge()->showPointer();
    }
}


//____ _addCallback() _________________________________________________________

void WgEventHandler::_addCallback( const WgEventFilter& filter, Callback * pCallback )
{
	if( filter.FiltersWidget() )
	{
		WgWidgetWeakPtr pWidget = filter.Widget();

		wg::Chain<Callback>& chain = m_widgetCallbacks[pWidget];

		chain.pushBack(pCallback);
	}
	else
		m_globalCallbacks.pushBack( pCallback );
}


//____ _deleteCallbacksTo() __________________________________________________

int WgEventHandler::_deleteCallbacksTo( const void * pReceiver )
{
	int nDeleted = 0;

	// Delete any global callbacks for this receiver.

	Callback * p = m_globalCallbacks.first();
	while( p )
	{
		if( p->Receiver() != pReceiver )
			p = p->next();
		else
		{
			Callback * pNext = p->next();
			delete p;
			nDeleted++;
			p = pNext;
		}
	}

	// Delete any Widget-specific callbacks for this receiver.

	std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.begin();

	while( it != m_widgetCallbacks.end() )
	{
		Callback * p = it->second.first();
		while( p )
		{
			if( p->Receiver() != pReceiver )
				p = p->next();
			else
			{
				Callback * pNext = p->next();
				delete p;					// Receiver is dead, delete callback.
				nDeleted++;
				p = pNext;
			}
		}
		++it;
	}

	return nDeleted;
}

//____ _deleteCallbacksOnType() _______________________________________________

int WgEventHandler::_deleteCallbacksOnType( WgEventType type, wg::Chain<Callback> * pChain )
{
	int nDeleted = 0;
	Callback * p = pChain->first();

	while( p )
	{

		if( p->Filter().EventType() == type )
		{
			Callback * pNext = p->next();
			delete p;
			nDeleted++;
			p = pNext;
		}
		else
			p = p->next();
	}
	return nDeleted;
}

//____ deleteCallback() ______________________________________________________

int WgEventHandler::_deleteCallback( const WgEventFilter& filter, const void * pReceiver )
{
	// Deletes all callbacks created with exactly the same filter settings and receiver.

	int nDeleted = 0;
	wg::Chain<Callback>*	pChain;

	if( filter.FiltersWidget() )
	{
		std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.find(WgWidgetWeakPtr(filter.Widget()));

		if( it == m_widgetCallbacks.end() )
			return 0;					// No callbacks for Widget

		pChain = &it->second;
	}
	else
	{
		pChain = &m_globalCallbacks;
	}

	// We now have the right chain and need to check all the callbacks.

	Callback * p = pChain->first();
	while( p )
	{
		if( filter.EventType() == p->Filter().EventType() && pReceiver == p->Receiver() )
		{
			Callback * pNext = p->next();
			delete p;
			nDeleted++;
			p = pNext;
		}
		else
			p = p->next();
	}

	if( pChain->size() == 0 && pChain != &m_globalCallbacks )
	{
		std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it = m_widgetCallbacks.find(WgWidgetWeakPtr(filter.Widget()));
		m_widgetCallbacks.erase(it);

	}

	return nDeleted;
}


//____ QueueEvent() ___________________________________________________________

bool WgEventHandler::QueueEvent( WgEvent::Event * pEvent )
{
	if (m_bIgnoreNextCharacterEvent && pEvent->Type() == WG_EVENT_CHARACTER)
	{
		m_bIgnoreNextCharacterEvent = false;
		return true;							// Not queued, but not an error so we pretend everything is ok.
	}

	if( m_bIsProcessing )
	{
		// Events that are posted as a direct result of another event being processed
		// are inserted before the next event in the queue.
		// If two or more events are posted by the same event being processed,
		// they need to be queued in the order of posting.

		m_insertPos = m_eventQueue.insert( m_insertPos, pEvent );
		m_insertPos++;
	}
	else
	{
		// Events being posted outside processing loop are simply added to the
		// queue.

		m_eventQueue.push_back( pEvent );
	}

	return true;
}

//____ ForwardEvent() __________________________________________________________

bool WgEventHandler::ForwardEvent( const WgEvent::Event * _pEvent )
{
	if( !_pEvent )
		return false;

	WgWidget * p = _pEvent->Widget();
	if( p && p->Parent() )
		return ForwardEvent( _pEvent, p->Parent() );
	else
		return false;
}

bool WgEventHandler::ForwardEvent( const WgEvent::Event * _pEvent, WgWidget * pRecipient )
{
	if( !_pEvent)
		return false;

	const_cast<WgEvent::Event *>(_pEvent)->m_pForwardTo = pRecipient;
	return true;
}

//____ _forwardEvent() __________________________________________________________

bool WgEventHandler::_forwardEvent( const WgEvent::Event * _pEvent )
{
	WgWidget * pRecipient = _pEvent->m_pForwardTo.GetRealPtr();


	WgEvent::Event * pEvent = 0;

	switch( _pEvent->Type() )
	{
		case WG_EVENT_MOUSE_ENTER:
			pEvent = new WgEvent::MouseEnter(WgCoord());
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseEnter*>(_pEvent) );
			break;
		case WG_EVENT_MOUSE_MOVE:
			pEvent = new WgEvent::MouseMove(WgCoord());
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseMove*>(_pEvent) );
			break;
		case WG_EVENT_MOUSE_LEAVE:
			pEvent = new WgEvent::MouseLeave();
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseLeave*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_PRESS:
			pEvent = new WgEvent::MouseButtonPress(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonPress*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_REPEAT:
			pEvent =  new WgEvent::MouseButtonRepeat(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonRepeat*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_DRAG:
			pEvent =  new WgEvent::MouseButtonDrag(0,WgCoord(),WgCoord(),WgCoord());
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonDrag*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_RELEASE:
			pEvent =  new WgEvent::MouseButtonRelease(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonRelease*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_CLICK:
			pEvent =  new WgEvent::MouseButtonClick(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonClick*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
			pEvent =  new WgEvent::MouseButtonDoubleClick(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseButtonDoubleClick*>(_pEvent) );
			break;
		case WG_EVENT_KEY_PRESS:
			pEvent =  new WgEvent::KeyPress(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::KeyPress*>(_pEvent) );
			break;
		case WG_EVENT_KEY_REPEAT:
			pEvent =  new WgEvent::KeyRepeat(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::KeyRepeat*>(_pEvent) );
			break;
		case WG_EVENT_KEY_RELEASE:
			pEvent =  new WgEvent::KeyRelease(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::KeyRelease*>(_pEvent) );
			break;
		case WG_EVENT_CHARACTER:
			pEvent =  new WgEvent::Character(0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::Character*>(_pEvent) );
			break;
		case WG_EVENT_MOUSEWHEEL_ROLL:
			pEvent =  new WgEvent::MouseWheelRoll(0,0);
			pEvent->_cloneContentFrom( static_cast<const WgEvent::MouseWheelRoll*>(_pEvent) );
			break;

		default:
			return false;
	}

	pEvent->m_pWidget = pRecipient;
	pEvent->m_pForwardedFrom = _pEvent->m_pWidget;
	QueueEvent( pEvent );
	return true;
}

//____ 	IgnoreNextCharacterEvent() ____________________________________________

void WgEventHandler::IgnoreNextCharacterEvent()
{
	m_bIgnoreNextCharacterEvent = true;
}

//____ ProcessEvents() ________________________________________________________

void WgEventHandler::ProcessEvents()
{
	int64_t	time = m_time;

	m_bIsProcessing = true;

	// First thing: we make sure that we know what Widgets pointer is inside, in case that has changed.

	m_insertPos = m_eventQueue.begin();	// Insert any POINTER_ENTER/EXIT right at beginning.
	_updateMarkedWidget(false);

	// Process all the events in the queue

	_processEventQueue();

	// Post Widget-specific tick events now we know how much time has passed

	if( m_time > time )
		_postTickEvents( (int) (m_time-time) );

	// Process Widget-specific tick events (and any events they might trigger)

	_processEventQueue();

	m_bIsProcessing = false;
}

//____ _processEventQueue() ___________________________________________________

void WgEventHandler::_processEventQueue()
{
	while( !m_eventQueue.empty() )
	{
		bool    bEavesdropped = false;

		WgEvent::Event * pEvent = m_eventQueue.front();

		m_insertPos = m_eventQueue.begin()+1;	// Insert position set to right after current event.

		_finalizeEvent( pEvent );

		if( pEvent->IsForWidget() )
		{
			WgWidget * pWidget = pEvent->Widget();
			if( pWidget )
			{
				pWidget->_onEvent( pEvent, this );

				// Need to retrieve widget from weakptr again, in case callback deleted it.
				pWidget = pEvent->Widget();
				if(pWidget)
					bEavesdropped = _handleEavesdropping( pWidget, pEvent );
			}
		}
		else
		{
			_processGeneralEvent( pEvent );
		}
		_processEventCallbacks( pEvent );

		if( pEvent->m_pForwardTo  != nullptr )
			_forwardEvent( pEvent );

		if( pEvent->m_pFinalRecipient )
			pEvent->m_pFinalRecipient->_onEvent( pEvent, this );

		m_eventQueue.pop_front();
		m_insertPos = m_eventQueue.begin();		// Insert position set right to start.

		// Delete event object unless its a BUTTON_PRESS, BUTTON_RELEASE or KEY_PRESS event for NO SPECIFIC WIDGET,
		// which are kept in m_pLatestPressEvents, m_pLatestReleaseEvents and m_keysDown respectively.

		if( !bEavesdropped && (pEvent->IsForWidget() || (pEvent->Type() !=  WG_EVENT_MOUSEBUTTON_PRESS &&
			pEvent->Type() != WG_EVENT_MOUSEBUTTON_RELEASE && pEvent->Type() != WG_EVENT_KEY_PRESS)) )
			delete pEvent;
	}
}

//____ _handleEavesdropping() ________________________________________________

bool WgEventHandler::_handleEavesdropping( WgWidget * pReceiver, WgEvent::Event * pEvent )
{
	auto pWidget = pReceiver->Parent();
	while( pWidget )
	{
		if( pWidget->IsEavesdropping() )
		{
			//NOTE: We only support one eavesdropper for each event in WG2, since we don't reference count the eavesdropped message.
			QueueEvent( new WgEvent::Eavesdrop( pWidget, pEvent ) );
			return true;
		}
		pWidget = pWidget->Parent();
	}

	return false;
}

//____ _widgetLockPointer() __________________________________________________

bool WgEventHandler::_widgetLockPointer(WgWidget * pWidget)
{
    if( m_pWidgetLockedPointer )
        return false;

    m_pWidgetLockedPointer = pWidget;
    return true;
}

//____ _widgetReleasePointer() _______________________________________________

void WgEventHandler::_widgetReleasePointer(WgWidget * pWidget)
{
    if( pWidget == m_pWidgetLockedPointer.GetRealPtr() )
    {
        m_pWidgetLockedPointer = nullptr;
    }
}


//____ _postTickEvents() ______________________________________________________

void WgEventHandler::_postTickEvents( int microsec )
{
	std::vector<WgWidgetWeakPtr>::iterator it = m_vTickWidgets.begin();

	while( it != m_vTickWidgets.end() )
	{
		WgWidget * pWidget = (*it).GetRealPtr();

		if( pWidget && pWidget->Hook() && pWidget->Hook()->Root() == m_pRoot && pWidget->m_bReceiveTick )
		{
			QueueEvent( new WgEvent::Tick( microsec, pWidget, true ) );
			++it;
		}
		else
			it = m_vTickWidgets.erase(it);
	}
}

//____ _addTickReceiver() _____________________________________________________

void WgEventHandler::_addTickReceiver( WgWidget * pWidget )
{
 	if( pWidget && !_isWidgetInList( pWidget, m_vTickWidgets ) )
		m_vTickWidgets.push_back( WgWidgetWeakPtr(pWidget) );
}


//____ _processEventCallbacks() ________________________________________________

void WgEventHandler::_processEventCallbacks( WgEvent::Event * pEvent )
{
	// Call all global callbacks

	Callback * pCallback = m_globalCallbacks.first();

	while( pCallback )
	{
		pCallback->ProcessEvent( pEvent );
		pCallback = pCallback->next();
	}

	// Call all Widget-specific callbacks

	wg::Chain<Callback> * pChain = 0;

	if( pEvent->Widget() )
	{
		std::map<WgWidgetWeakPtr,wg::Chain<Callback> >::iterator it;

		it = m_widgetCallbacks.find(pEvent->WidgetWeakPtr());
		if( it != m_widgetCallbacks.end() )
			pChain = &(it->second);
	}

	if( !pChain )
		return;

	pCallback = pChain->first();

	while( pCallback )
	{
		pCallback->ProcessEvent( pEvent );
		pCallback = pCallback->next();
	}
}


//____ _finalizeEvent() ________________________________________________________

void WgEventHandler::_finalizeEvent( WgEvent::Event * pEvent )
{
	// Fill in missing information in the event-class.

	pEvent->m_timestamp	= m_time/1000;
	pEvent->m_modKeys	= m_modKeys;
	pEvent->m_pointScale = m_pRoot->Scale();

	// Only global POINTER_ENTER & POINTER_MOVE events have these members
	// set, the rest needs to have them filled in.

	if( pEvent->IsForWidget() || (pEvent->Type() != WG_EVENT_MOUSE_MOVE && pEvent->Type() != WG_EVENT_MOUSE_ENTER) )
	{
		pEvent->m_pointerScreenPixelPos = m_pointerPos;
		pEvent->m_pointerLocalPixelPos = m_pointerPos;

		if( pEvent->Widget() )
			pEvent->m_pointerLocalPixelPos -= pEvent->Widget()->ScreenPixelPos();
	}

	// Event specific finalizations

	switch( pEvent->Type() )
	{

		// Key events need translation of keycodes.

		case WG_EVENT_KEY_PRESS:
		case WG_EVENT_KEY_RELEASE:
		case WG_EVENT_KEY_REPEAT:
		{
			WgEvent::KeyEvent* p = static_cast<WgEvent::KeyEvent*>(pEvent);
			p->m_translatedKeyCode = WgBase::TranslateKey(p->m_nativeKeyCode);
			break;
		}

		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			auto p = static_cast<WgEvent::MouseButtonRelease*>(pEvent);

			int button = p->Button();

			if( m_pLatestPressEvents[button] )
				p->m_pressModKeys = m_pLatestPressEvents[button]->ModKeys();
			break;
		}
		default:
			break;
	}



}

//____ _processGeneralEvent() _________________________________________________

void WgEventHandler::_processGeneralEvent( WgEvent::Event * pEvent )
{

	switch( pEvent->Type() )
	{
		case WG_EVENT_FOCUS_GAINED:
			_processFocusGained( (WgEvent::FocusGained*) pEvent );
			break;

		case WG_EVENT_FOCUS_LOST:
			_processFocusLost( (WgEvent::FocusLost*) pEvent );
			break;

		case WG_EVENT_MOUSE_ENTER:
			_processMouseEnter( (WgEvent::MouseEnter*) pEvent );
			break;

		case WG_EVENT_MOUSE_MOVE:
			_processMouseMove( (WgEvent::MouseMove*) pEvent );
			break;

		case WG_EVENT_MOUSE_POSITION:
			_processMousePosition( (WgEvent::MousePosition*) pEvent );
			break;

		case WG_EVENT_MOUSE_LEAVE:
			_processMouseLeave( (WgEvent::MouseLeave*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_PRESS:
			_processMouseButtonPress( (WgEvent::MouseButtonPress*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_REPEAT:
			_processMouseButtonRepeat( (WgEvent::MouseButtonRepeat*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_DRAG:
			_processMouseButtonDrag( (WgEvent::MouseButtonDrag*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_RELEASE:
			_processMouseButtonRelease( (WgEvent::MouseButtonRelease*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_CLICK:
			_processMouseButtonClick( (WgEvent::MouseButtonClick*) pEvent );
			break;

		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
			_processMouseButtonDoubleClick( (WgEvent::MouseButtonDoubleClick*) pEvent );
			break;

		case WG_EVENT_KEY_PRESS:
			_processKeyPress( (WgEvent::KeyPress*) pEvent );
			break;

		case WG_EVENT_KEY_REPEAT:
			_processKeyRepeat( (WgEvent::KeyRepeat*) pEvent );
			break;

		case WG_EVENT_KEY_RELEASE:
			_processKeyRelease( (WgEvent::KeyRelease*) pEvent );
			break;

		case WG_EVENT_CHARACTER:
			_processCharacter( (WgEvent::Character*) pEvent );
			break;

		case WG_EVENT_MOUSEWHEEL_ROLL:
			_processMouseWheelRoll( (WgEvent::MouseWheelRoll*) pEvent );
			break;

		case WG_EVENT_TICK:
			_processTick( (WgEvent::Tick*) pEvent );
			break;

		case WG_EVENT_DUMMY:
			break;

		case WG_EVENT_MAX:
			assert(false);								// NOT ALLOWED!
			break;

				default:
						break;
	}
}

//____ _processTick() ______________________________________________________

void WgEventHandler::_processTick( WgEvent::Tick * pEvent )
{
	// Handle marked lock

	if( m_markedLockCountdown > 0 )
	{
		m_markedLockCountdown -= pEvent->Millisec();
		if( m_markedLockCountdown <= 0 )
		{
			m_markedLockCountdown = 0;
			_updateMarkedWidget(false);
		}
	}

	// Check if we need to post BUTTON_REPEAT

	for( int button = 0 ; button <= WG_MAX_BUTTONS ; button++ )
	{
		if( m_bButtonPressed[button] )
		{
			int buttonDelay = WgBase::MouseButtonRepeatDelay();
			int buttonRate = WgBase::MouseButtonRepeatRate();

			int msSinceRepeatStart = (int) ((m_time/1000) - m_pLatestPressEvents[button]->Timestamp() - buttonDelay );

			// First BUTTON_REPEAT event posted separately.

			if( msSinceRepeatStart < 0 && msSinceRepeatStart + pEvent->Millisec() >= 0 )
				QueueEvent( new WgEvent::MouseButtonRepeat(button) );

			// Calculate ms since last BUTTON_REPEAT event

			int msToProcess;
			if( msSinceRepeatStart < 0 )
				msToProcess = msSinceRepeatStart + pEvent->Millisec();
			else
				msToProcess = (msSinceRepeatStart % buttonRate) + pEvent->Millisec();

			// Post the amount of BUTTON_REPEAT that should be posted.

			while( msToProcess >= buttonRate )
			{
				QueueEvent( new WgEvent::MouseButtonRepeat(button) );
				msToProcess -= buttonRate;
			}
		}
	}

	// Check if we need to post KEY_REPEAT

	int keyDelay 	= WgBase::KeyRepeatDelay();
	int keyRate 	= WgBase::KeyRepeatRate();

	for( unsigned int i = 0 ; i < m_keysDown.size() ; i++ )
	{
		KeyDownInfo * pInfo = m_keysDown[i];

		// Skip posting KEY_REPEAT for modifier keys

		int keyCode = pInfo->pEvent->TranslatedKeyCode();
		if( keyCode == WG_KEY_SHIFT || keyCode == WG_KEY_ALT || keyCode == WG_KEY_STD_CONTROL || keyCode == WG_KEY_MAC_CONTROL || keyCode == WG_KEY_OS )
			continue;

		//

		int timePassed = (int) (pEvent->Timestamp() - pInfo->pEvent->Timestamp());

		int fraction = timePassed - keyDelay;

		if( fraction < 0 )
			fraction += keyRate;
		else
			fraction %= keyRate;

		fraction += pEvent->Millisec();

		while( fraction >= keyRate )
		{
			QueueEvent( new WgEvent::KeyRepeat( pInfo->pEvent->NativeKeyCode() ) );
			fraction -= keyRate;
		}
	}


	// Increase time counter

	m_time += pEvent->Microsec();
}


//____ _processFocusGained() __________________________________________________

void WgEventHandler::_processFocusGained( WgEvent::FocusGained * pEvent )
{
	if( !m_bWindowFocus && m_keyFocusWidget )
		m_keyFocusWidget.GetRealPtr()->_onGotInputFocus();

	m_bWindowFocus = true;
}

//____ _processFocusLost() ____________________________________________________

void WgEventHandler::_processFocusLost( WgEvent::FocusLost * pEvent )
{
	if( m_bWindowFocus && m_keyFocusWidget )
		m_keyFocusWidget.GetRealPtr()->_onLostInputFocus();

	m_bWindowFocus = false;
}


//____ _processMouseEnter() __________________________________________________

void WgEventHandler::_processMouseEnter( WgEvent::MouseEnter * pEvent )
{
	// Post events for button drag

	for( int i = 0 ; i <= WG_MAX_BUTTONS ; i++ )
	{
		if( m_bButtonPressed[i] )
			QueueEvent( new WgEvent::MouseButtonDrag( i, m_pLatestPressEvents[i]->PointerPixelPos(), m_pointerPos, pEvent->PointerPixelPos() ) );
	}

	// Post event for finalizing position once button drag is taken care of.

	QueueEvent( new WgEvent::MousePosition() );

	// Update pointer position

	m_pointerPos = pEvent->PointerPixelPos();
}

//____ _processMouseLeave() ___________________________________________________

void WgEventHandler::_processMouseLeave( WgEvent::MouseLeave * pEvent )
{
	// Post event for finalizing position

	QueueEvent(new WgEvent::MousePosition());

	// Update pointer position

	m_pointerPos = { -1,-1 };
}


//____ _processMouseMove() ___________________________________________________

void WgEventHandler::_processMouseMove( WgEvent::MouseMove * pEvent )
{
	if( pEvent->m_pointerScreenPixelPos == m_pointerPos )
		return;                                              // No real pointer movement.

	// Post events for button drag

	for( int i = 0 ; i <= WG_MAX_BUTTONS ; i++ )
	{
		if( m_bButtonPressed[i] )
			QueueEvent( new WgEvent::MouseButtonDrag( i, m_pLatestPressEvents[i]->PointerPixelPos(), m_pointerPos, pEvent->PointerPixelPos() ) );
	}

	// Post event for finalizing move once button drag is taken care of.

	QueueEvent( new WgEvent::MousePosition() );

	// Update pointer position

	m_pointerPos = pEvent->PointerPixelPos();
}

//____ _processMousePosition() _________________________________________________

void WgEventHandler::_processMousePosition( WgEvent::MousePosition * pEvent )
{
	_updateMarkedWidget(true);

}

//____ _updateMarkedWidget() _______________________________________________

void WgEventHandler::_updateMarkedWidget(bool bMouseMoved)
{
	if( m_markedLockCountdown > 0 )
		return;

    // Figure out which button of currently pressed has been pressed the longest.
    // Mouse is only allowed to mark Widgets that were marked on press of that button.

    int button = 0;                                // Button that has been pressed for longest, 0 = no button pressed
    for( int i = 1 ; i <= WG_MAX_BUTTONS ; i++ )
    {
        if( m_bButtonPressed[i] && (button == 0 || m_pLatestPressEvents[i]->Timestamp() < m_pLatestPressEvents[button]->Timestamp()) )
            button = i;
    }
    
    //
    
    WgWidget*  pNowMarked = 0;
    if( m_pWidgetLockedPointer )
    {
        pNowMarked = m_pWidgetLockedPointer.GetRealPtr();
    }
    else
    {
        WgWidget * pWidgetTarget = m_pRoot->FindWidget( m_pointerPos, WgSearchMode::ActionTarget );

        // We are only marking the Widget if no mouse button is pressed or widget was marked when the first button was pressed

        if( button == 0 )
            pNowMarked = pWidgetTarget;
        else
        {
            // Checking widgets marked while the button was pressed recursively
            //TODO: This check can fail if hierarchy has changed since button was pressed.
            // We need to explicitly save all widgets receiving the press and check against that list.
            // I believe we have the same problem inside _updateEnteredWidgets().

            while (pNowMarked == 0 && pWidgetTarget != 0)
            {
                WgWidget * pPressed = m_latestPressWidgets[button].GetRealPtr();
                while (pPressed)
                {
                    if (pWidgetTarget == pPressed)
                    {
                        pNowMarked = pWidgetTarget;
                        break;
                    }
                    pPressed = pPressed->Parent();
                }
                pWidgetTarget = pWidgetTarget->Parent();
            }
        }
    }

	// Post POINTER_EXIT events for widgets no longer marked,
	// Post POINTER_ENTER events for new marked widgets
	// and POINTER_MOVE event for first already marked (which can and should bubble down).

	WgWidget * pFirstAlreadyMarked = _updateEnteredWidgets( pNowMarked );

	if( bMouseMoved && pFirstAlreadyMarked )
		QueueEvent( new WgEvent::MouseMove( pFirstAlreadyMarked ) );

/*
	if( pNowMarked != m_pMarkedWidget.GetRealPtr() )
	{
		if( m_pMarkedWidget )
			QueueEvent( new WgEvent::MouseLeave( m_pMarkedWidget.GetRealPtr() ) );

		if( pNowMarked )
			QueueEvent( new WgEvent::MouseEnter( pNowMarked ) );
	}
	else if( bPostMouseMoveEvents && pNowMarked )
		QueueEvent( new WgEvent::MouseMove( pNowMarked ) );
*/
	// Copy content of pNowMarked to m_pMarkedWidget

	m_pMarkedWidget = pNowMarked;

	// Update PointerStyle

	WgPointerStyle newStyle;

	WgWidget * p = m_latestPressWidgets[button].GetRealPtr();

	if (button != 0 && p && p->IsEnabled())
		newStyle = p->GetPointerStyle();
	else if (pNowMarked && pNowMarked->IsEnabled())
		newStyle = pNowMarked->GetPointerStyle();
	else
		newStyle = WgPointerStyle::Arrow;

	if( newStyle != m_pointerStyle )
	{
		QueueEvent( new WgEvent::PointerChange( newStyle ) );
		m_pointerStyle = newStyle;
	}

}

//____ _updateEnteredWidgets() _________________________________________________

WgWidget * WgEventHandler::_updateEnteredWidgets( WgWidget * pMarkedWidget )
{
	WgWidget * pFirstAlreadyMarked = 0;

	// Loop through our new widgets and check if they already
	// were entered. Send MouseEnter to all new widgets and notice the first
	// common ancestor .

	for( WgWidget * pWidget = pMarkedWidget ; pWidget != 0 ; pWidget = pWidget->Parent() )
	{
		int ofs = _widgetPosInList( pWidget, m_vEnteredWidgets );
		if( ofs >= 0 )
		{
			if( !pFirstAlreadyMarked )
				pFirstAlreadyMarked = pWidget;
			m_vEnteredWidgets[ofs] = 0;
		}
		else
			QueueEvent( new WgEvent::MouseEnter( pWidget ) );
	}

	// Send MouseLeave to those that were left.

	for( size_t i = 0 ; i < m_vEnteredWidgets.size() ; i++ )
		if(m_vEnteredWidgets[i] )
			QueueEvent( new WgEvent::MouseLeave( m_vEnteredWidgets[i].GetRealPtr()) );

	// Replace the old list with a new one.

	m_vEnteredWidgets.clear();
	for( WgWidget * pWidget = pMarkedWidget ; pWidget != 0 ; pWidget = pWidget->Parent() )
		m_vEnteredWidgets.push_back( pWidget );

	// Return first already marked, calling function might need it.

	return pFirstAlreadyMarked;
}




//____ _processKeyPress() ______________________________________________________

void WgEventHandler::_processKeyPress( WgEvent::KeyPress * pEvent )
{
	// Make sure we don't already have this key press

	for (KeyDownInfo * p : m_keysDown)
	{
		if (p->pEvent->NativeKeyCode() == pEvent->NativeKeyCode())
			return;
	}

	// Fill in the info-structure.

	KeyDownInfo * pInfo = new KeyDownInfo();
	pInfo->pEvent = pEvent;

	// Post KEY_PRESS events for widgets and remember which ones we have posted it for

	WgWidget * pWidget = m_keyFocusWidget.GetRealPtr();
	if( pWidget )
		QueueEvent( new WgEvent::KeyPress( pEvent->NativeKeyCode(), pWidget ) );
	pInfo->pWidget = pWidget;

	// Push the info-structure onto m_keysDown.

	m_keysDown.push_back( pInfo );

	// Update modkeys

	switch( pEvent->TranslatedKeyCode() )
	{
		case WG_KEY_SHIFT:
			m_modKeys = (WgModifierKeys) (m_modKeys | WG_MODKEY_SHIFT);
			break;
		case WG_KEY_ALT:
			m_modKeys = (WgModifierKeys) (m_modKeys | WG_MODKEY_ALT);
			break;
		case WG_KEY_STD_CONTROL:
			m_modKeys = (WgModifierKeys) (m_modKeys | WG_MODKEY_STD_CTRL);
			break;
		case WG_KEY_MAC_CONTROL:
			m_modKeys = (WgModifierKeys) (m_modKeys | WG_MODKEY_MAC_CTRL);
			break;
		case WG_KEY_OS:
			m_modKeys = (WgModifierKeys) (m_modKeys | WG_MODKEY_OS);
			break;
		default:
			break;
	}
}

//____ _processKeyRepeat() ____________________________________________________

void WgEventHandler::_processKeyRepeat( WgEvent::KeyRepeat * pEvent )
{
	// Find right KeyDownInfo structure

	KeyDownInfo * pInfo = 0;

	for( unsigned int i = 0 ; i < m_keysDown.size() ; i++ )
	{
		if( pEvent->NativeKeyCode() == m_keysDown[i]->pEvent->NativeKeyCode() )
		{
			pInfo = m_keysDown[i];
			break;
		}
	}

	assert( pInfo != 0 );		// KEY_RELEASE without a matching KEY_PRESS preceding.
	if( pInfo == 0 )
		return;

	// Post KEY_REPEAT event for widget

		if( pInfo->pWidget )
			QueueEvent( new WgEvent::KeyRepeat( pEvent->NativeKeyCode(), pInfo->pWidget.GetRealPtr() ));
}

//____ _processKeyRelease() ___________________________________________________

void WgEventHandler::_processKeyRelease( WgEvent::KeyRelease * pEvent )
{
	// Find right KeyDownInfo structure and remove it from m_keysDown.

	KeyDownInfo * pInfo = 0;

	std::vector<KeyDownInfo*>::iterator it = m_keysDown.begin();
	while( it != m_keysDown.end() )
	{
		if( pEvent->NativeKeyCode() == (*it)->pEvent->NativeKeyCode() )
		{
			pInfo = *it;
			m_keysDown.erase(it);
			break;
		}
		it++;
	}

//	assert( pInfo != 0 );			// KEY_RELEASE without a matching KEY_PRESS preceding.
	if( !pInfo )
		return;

	// Post KEY_RELEASE event for widget

	if( pInfo->pWidget )
		QueueEvent( new WgEvent::KeyRelease( pEvent->NativeKeyCode(), pInfo->pWidget.GetRealPtr() ));

	// Delete the KeyPress-message and KeyDownInfo-structure

	delete pInfo->pEvent;
	delete pInfo;

	// Update modkeys

	switch( pEvent->TranslatedKeyCode() )
	{
		case WG_KEY_SHIFT:
			m_modKeys = (WgModifierKeys) (m_modKeys & ~WG_MODKEY_SHIFT);
			break;
		case WG_KEY_ALT:
			m_modKeys = (WgModifierKeys) (m_modKeys & ~WG_MODKEY_ALT);
			break;
		case WG_KEY_STD_CONTROL:
			m_modKeys = (WgModifierKeys) (m_modKeys & ~WG_MODKEY_STD_CTRL);
			break;
		case WG_KEY_MAC_CONTROL:
			m_modKeys = (WgModifierKeys) (m_modKeys & ~WG_MODKEY_MAC_CTRL);
			break;
		case WG_KEY_OS:
			m_modKeys = (WgModifierKeys) (m_modKeys & ~WG_MODKEY_OS);
			break;
		default:
			break;
	}
}

//____ _processCharacter() ____________________________________________________

void WgEventHandler::_processCharacter( WgEvent::Character * pEvent )
{
	WgWidget * pWidget = m_keyFocusWidget.GetRealPtr();

	if( pWidget )
		QueueEvent( new WgEvent::Character( pEvent->Char(), pWidget ) );
}

//____ _processMouseWheelRoll() ____________________________________________________

void WgEventHandler::_processMouseWheelRoll( WgEvent::MouseWheelRoll * pEvent )
{
	_updateMarkedWidget(false);

	WgWidget * pWidget = m_pMarkedWidget.GetRealPtr();

	if( pWidget )
		QueueEvent( new WgEvent::MouseWheelRoll( pEvent->Wheel(), pEvent->Distance(), pEvent->InvertScroll(), pWidget ) );

	m_markedLockCountdown = 120;
}

//____ _processMouseButtonPress() ___________________________________________________

void WgEventHandler::_processMouseButtonPress( WgEvent::MouseButtonPress * pEvent )
{
	_updateMarkedWidget(false);

	int button = pEvent->Button();

	// Update m_previousPressWidgets

	m_previousPressWidgets[button] = 0;

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();
	if( pWidget )
		m_previousPressWidgets[button] = pWidget;

	// Post BUTTON_PRESS events for marked widgets and remember which one we have posted it for

	m_latestPressWidgets[button] = 0;

	pWidget = m_pMarkedWidget.GetRealPtr();
	if( pWidget )
	{
		QueueEvent( new WgEvent::MouseButtonPress( button, pWidget ) );
		m_latestPressWidgets[button] = pWidget;
	}

	// Handle possible double-click

	int doubleClickTimeTreshold = WgBase::DoubleClickTimeTreshold();
	int doubleClickDistanceTreshold = WgBase::DoubleClickDistanceTreshold();


	if( m_pLatestPressEvents[button] && m_pLatestPressEvents[button]->Timestamp() + doubleClickTimeTreshold > pEvent->Timestamp() )
	{
		WgCoord distance = pEvent->PointerPixelPos() - m_pLatestPressEvents[button]->PointerPixelPos();

		if( distance.x <= doubleClickDistanceTreshold &&
			distance.x >= -doubleClickDistanceTreshold &&
			distance.y <= doubleClickDistanceTreshold &&
			distance.y >= -doubleClickDistanceTreshold )
			QueueEvent( new WgEvent::MouseButtonDoubleClick(button) );
	}

	// Save info for the future

	delete m_pLatestPressEvents[button];		// Delete previous saved event.
	m_pLatestPressEvents[button] = pEvent;

	m_bButtonPressed[button] = true;
}


//____ _processMouseButtonRepeat() __________________________________________________

void WgEventHandler::_processMouseButtonRepeat( WgEvent::MouseButtonRepeat * pEvent )
{
	_updateMarkedWidget(false);

	int button = pEvent->Button();

	// Post BUTTON_REPEAT event for widget that received the press if we are still inside.

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();
	if( pWidget && pWidget == m_pMarkedWidget.GetRealPtr() )
		QueueEvent( new WgEvent::MouseButtonRepeat(button, pWidget) );
}



//____ _processMouseButtonRelease() _________________________________________________

void WgEventHandler::_processMouseButtonRelease( WgEvent::MouseButtonRelease * pEvent )
{
	_updateMarkedWidget(false);

	int button = pEvent->Button();

	// Post BUTTON_RELEASE events for widget that was pressed

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();
	if( pWidget )
	{
		bool bIsInside = pWidget->ScreenPixelGeo().contains(pEvent->PointerPixelPos());
		QueueEvent( new WgEvent::MouseButtonRelease( button, pWidget, true, bIsInside ) );
	}

	// Post BUTTON_RELEASE events for marked widget that was NOT pressed

	pWidget = m_pMarkedWidget.GetRealPtr();
	if( pWidget )
	{
		if( pWidget != m_latestPressWidgets[button].GetRealPtr() )
		{
			bool bIsInside = pWidget->ScreenPixelGeo().contains(pEvent->PointerPixelPos());
			QueueEvent( new WgEvent::MouseButtonRelease( button, pWidget, false, bIsInside ) );
		}
	}

	// As long as the button was pressed inside our window we have a click
	// on this level.

	if( m_bButtonPressed[button] )
		QueueEvent( new WgEvent::MouseButtonClick( button ) );

	// Save info for the future

	delete m_pLatestReleaseEvents[button];		// Delete previous saved event.
	m_pLatestReleaseEvents[button] = pEvent;
	m_bButtonPressed[button] = false;
}

//____ _processMouseButtonDrag() ____________________________________________________

void WgEventHandler::_processMouseButtonDrag( WgEvent::MouseButtonDrag * pEvent )
{
	int button = pEvent->Button();

	// Post POINTER_DRAG event for pressed widget

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();

	if( pWidget )
	{
		WgCoord	ofs = pWidget->ScreenPixelPos();
		QueueEvent( new WgEvent::MouseButtonDrag( button, pWidget, pEvent->StartPixelPos() - ofs, pEvent->PrevPixelPos() - ofs, pEvent->CurrPixelPos() - ofs ) );
	}

}

//____ _processMouseButtonClick() _________________________________________________

void WgEventHandler::_processMouseButtonClick( WgEvent::MouseButtonClick * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_CLICK events for widget that received the press if we
	// still are inside.

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();
	if( pWidget && pWidget == m_pMarkedWidget.GetRealPtr() )
		QueueEvent( new WgEvent::MouseButtonClick(button, pWidget) );
}

//____ _processMouseButtonDoubleClick() _________________________________________________

void WgEventHandler::_processMouseButtonDoubleClick( WgEvent::MouseButtonDoubleClick * pEvent )
{
	int button = pEvent->Button();

	// Post BUTTON_DOUBLE_CLICK event if gizom received both this and previous press.

	WgWidget * pWidget = m_latestPressWidgets[button].GetRealPtr();
	if( pWidget && pWidget ==  m_previousPressWidgets[button].GetRealPtr() )
		QueueEvent( new WgEvent::MouseButtonDoubleClick(button, pWidget) );
}

//____ _isWidgetInList() ________________________________________________________

bool WgEventHandler::_isWidgetInList( const WgWidget * pWidget, const std::vector<WgWidgetWeakPtr>& list )
{
	for( size_t i = 0 ; i < list.size() ; i++ )
		if( list[i].GetRealPtr() == pWidget )
			return true;

	return false;
}

//____ _widgetPosInList() ________________________________________________________

int WgEventHandler::_widgetPosInList( const WgWidget * pWidget, const std::vector<WgWidgetWeakPtr>& list )
{
	for( size_t i = 0 ; i < list.size() ; i++ )
		if( list[i].GetRealPtr() == pWidget )
			return (int) i;

	return -1;
}




WgEventHandler::WidgetCallback::WidgetCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, WgWidget * pDest), WgWidget * pDest )
{
	m_filter = filter;
	m_pFunction = fp;
	m_pWidget = pDest;
}

void WgEventHandler::WidgetCallback::ProcessEvent( const WgEvent::Event * pEvent )
{
	WgWidget * p = m_pWidget.GetRealPtr();
	if( p && m_filter.FilterEvent(pEvent) )
		m_pFunction(pEvent,p);
}

bool WgEventHandler::WidgetCallback::IsAlive() const
{
	return m_pWidget?true:false;
}

void * WgEventHandler::WidgetCallback::Receiver() const
{
	return m_pWidget.GetRealPtr();
}


WgEventHandler::FunctionCallback::FunctionCallback( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent) )
{
	m_filter = filter;
	m_pFunction = fp;
}

void WgEventHandler::FunctionCallback::ProcessEvent( const WgEvent::Event * pEvent )
{
	if( m_pFunction && m_filter.FilterEvent(pEvent) )
		m_pFunction(pEvent);
}

bool WgEventHandler::FunctionCallback::IsAlive() const
{
	return true;
}

void * WgEventHandler::FunctionCallback::Receiver() const
{
	return reinterpret_cast<void*>(m_pFunction);
}

WgEventHandler::FunctionCallbackParam::FunctionCallbackParam( const WgEventFilter& filter, void(*fp)(const WgEvent::Event * pEvent, void * pDest), void * pParam )
{
	m_filter = filter;
	m_pFunction = fp;
	m_pParam 	= pParam;
}

void WgEventHandler::FunctionCallbackParam::ProcessEvent( const WgEvent::Event * pEvent )
{
	if( m_pFunction && m_filter.FilterEvent(pEvent) )
		m_pFunction(pEvent,m_pParam);
}

bool WgEventHandler::FunctionCallbackParam::IsAlive() const
{
	return true;
}

void * WgEventHandler::FunctionCallbackParam::Receiver() const
{
	return reinterpret_cast<void*>(m_pFunction);
}


WgEventHandler::ListenerCallback::ListenerCallback( const WgEventFilter& filter, WgEventListener * pListener )
{
	m_filter = filter;
	m_pListener = pListener;
}

void WgEventHandler::ListenerCallback::ProcessEvent( const WgEvent::Event * pEvent )
{
	if( m_filter.FilterEvent(pEvent) )
		m_pListener->OnEvent( pEvent );
}

bool WgEventHandler::ListenerCallback::IsAlive() const
{
	return true;
}

void * WgEventHandler::ListenerCallback::Receiver() const
{
	return m_pListener;
}
