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
#include <wg_inputhandler.h>
#include <wg_base.h>
#include <wg_rootpanel.h>
#include <wg_container.h>

namespace wg
{

	const TypeInfo InputHandler::TYPEINFO = { "InputHandler", &Object::TYPEINFO };

	//____ constructor ____________________________________________________________

	InputHandler::InputHandler()
	{
		m_inputId 		= 1;	//TODO: Make unique between input handlers.

		m_timestamp 	= 0;
		m_pointerStyle 	= PointerStyle::Arrow;
		m_modKeys 		= ModKeys::None;

		m_doubleClickTimeTreshold 		= 250000;
		m_doubleClickDistanceTreshold 	= 2;

		m_buttonRepeatDelay 	= 300000;
		m_buttonRepeatRate 		= 200000;

		m_keyRepeatDelay 		= 300000;
		m_keyRepeatRate 		= 150000;



		for( int i = 0 ; i < MouseButton_size ; i++ )
		{
			m_bButtonPressed[i] = false;
			m_latestPressTimestamps[i] = 0;
			m_pressOrdinal[i] = 0;
		}
	}

	//____ Destructor _____________________________________________________________

	InputHandler::~InputHandler()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& InputHandler::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setDoubleClickTresholds() _______________________________________________

	bool InputHandler::setDoubleClickTresholds( int time, int distance )
	{
		if( time <= 0 || distance <= 0 )
			return false;

		m_doubleClickTimeTreshold		= time;
		m_doubleClickDistanceTreshold	= distance;
		return true;
	}

	//____ setButtonRepeat() ______________________________________________________

	bool InputHandler::setButtonRepeat( int delay, int rate )
	{
		if( delay <= 0 || rate <= 0 )
			return false;

		m_buttonRepeatDelay	= delay;
		m_buttonRepeatRate	= rate;
		return true;
	}

	//____ setKeyRepeat() _________________________________________________________

	bool InputHandler::setKeyRepeat( int delay, int rate )
	{
		if( delay <= 0 || rate <= 0 )
			return false;

		m_keyRepeatDelay	= delay;
		m_keyRepeatRate		= rate;
		return true;
	}

	//____ mapKey() _______________________________________________________________

	void InputHandler::mapKey( int native_keycode, Key translated_keycode )
	{
		m_keycodeMap[native_keycode] = translated_keycode;
	}


	//____ unmapKey() _____________________________________________________________

	void InputHandler::unmapKey( int native_keycode )
	{
		auto it = m_keycodeMap.find( native_keycode );

		if( it != m_keycodeMap.end() )
			m_keycodeMap.erase(it);
	}

	//____ clearKeyMap() __________________________________________________________

	void InputHandler::clearKeyMap()
	{
		m_keycodeMap.clear();
	}

	//____ translateKey() __________________________________________________________

	Key InputHandler::translateKey( int native_keycode )
	{
		auto it = m_keycodeMap.find(native_keycode);
		if( it != m_keycodeMap.end() )
			return  it->second;
		else
			return Key::Unmapped;
	}

	//____ nativeKeyCode() _______________________________________________________

	int InputHandler::nativeKeyCode( Key translatedKeyCode )
	{
		for( auto it = m_keycodeMap.begin() ; it != m_keycodeMap.end() ; it++ )
			if( it->second == translatedKeyCode )
				return it->first;

		return 0;
	}


	//____ mapCommand() ________________________________________________________

	void InputHandler::mapCommand( int native_keycode, ModKeys modKeys, EditCmd command )
	{
		auto& vCommands = m_commandMap[native_keycode];

		for( auto it = vCommands.begin() ; it != vCommands.end() ; it++ )
		{
			if( it->modKeys == modKeys )
			{
				it->command = command;
				return;
			}
		}

		CommandEntry e;
		e.modKeys = modKeys;
		e.command = command;

		vCommands.push_back( e );
	}

	//____ unmapCommand() ______________________________________________________

	void InputHandler::unmapCommand( int native_keycode, ModKeys modKeys )
	{
		auto it = m_commandMap.find(native_keycode);
		if( it != m_commandMap.end() )
		{
			std::vector<CommandEntry>& v = it->second;
			for( auto i = v.begin() ; i != v.end() ; i++ )
			{
				if( i->modKeys == modKeys )
				{
					v.erase(i);
					break;
				}
			}

			if( v.empty() )
				m_commandMap.erase(it);
		}
	}

	//____ clearCommandMap() ___________________________________________________

	void InputHandler::clearCommandMap()
	{
		m_commandMap.clear();
	}

	//____ translateCommand() __________________________________________________

	EditCmd InputHandler::translateCommand( int native_keycode, ModKeys modKeys )
	{
		auto it = m_commandMap.find(native_keycode);
		if( it != m_commandMap.end() )
		{
			std::vector<CommandEntry>& v = it->second;
			for( unsigned int i = 0 ; i < v.size() ; i++ )
				if( v[i].modKeys == modKeys )
					return v[i].command;
		}
		return EditCmd::None;
	}



	//____ putText() ___________________________________________________________

	bool InputHandler::putText( const CharSeq& text )
	{
		if( text.length() > 0 )
		{
			Base::msgRouter()->post( new TextInputMsg( m_inputId, text, _focusedWidget() ));
		}
		return true;
	}


	//____ setPointer() ___________________________________________________________

	void InputHandler::setPointer( Root * pRoot, Coord pos, int64_t timestamp )
{
		//TODO: How should we actually handle if pRoot == null?

		Coord	prevPointerPos = m_pointerPos;
		CoordSPX	prevPointerPosSPX = m_pointerPosSPX;

		if( timestamp == 0 )
			timestamp = m_timestamp;

		m_pointerPos = pos;
		m_pMarkedRoot = pRoot;
		m_pointerPosSPX = Util::ptsToSpx(pos, pRoot ? pRoot->scale() : 64);

		Widget_p pNowMarked = 0;
		Widget_p pWidgetTarget = 0;

		if( pRoot && pRoot->geo().contains( pos ) )
		{
			pWidgetTarget = pRoot->findWidget( m_pointerPos, SearchMode::ActionTarget );
		}

		// Figure out which button of currently pressed has been pressed the longest.
		// Mouse is only allowed to mark Widgets that were marked on press of that button.

		int button = 0;								// Button that has been pressed for longest, 0 = no button pressed
		for( int i = 1 ; i < MouseButton_size ; i++ )
		{
			if( m_bButtonPressed[i] && (button == 0 || m_latestPressTimestamps[i] < m_latestPressTimestamps[button]) )
				button = i;
		}

		// We are only marking the Widget if no mouse button is pressed or the first pressed button
		// was pressed on it.

		if( button == 0 || pWidgetTarget.rawPtr() == m_latestPressWidgets[button].rawPtr() )
			pNowMarked = pWidgetTarget;

		// Post Leave events for widgets no longer marked and
		// post Enter events for new marked widgets.

		Widget * pFirstAlreadyMarked = _updateEnteredWidgets( pNowMarked.rawPtr(), timestamp );

		// Copy content of pNowMarked to m_pMarkedWidget

		m_pMarkedWidget = pNowMarked.rawPtr();


 		if( pos != prevPointerPos )
		{
			m_bPointerMovedSinceUpdate = true;

			// Post Move events for those already marked

			if (pFirstAlreadyMarked)
			{
				//TODO: Ideally we should still post mouseMove messages to widgets that have moved even if pointer has not moved.

				MouseMoveMsg_p p = MouseMoveMsg::create(m_inputId, pFirstAlreadyMarked, m_modKeys, pos, m_pointerPosSPX, timestamp);
				p->setCopyTo(pFirstAlreadyMarked);
				Base::msgRouter()->post(p);
			}

			// Post events for button drag

			if( pRoot) 		//TODO: The pRoot-check is just a quick hack to prevent crashes. How should this really work?
			{
				for (int i = 0; i < MouseButton_size; i++)
				{
					if (m_bButtonPressed[i])
					{
						CoordSPX pressPosSPX = Util::ptsToSpx(m_latestPressPosition[i], pRoot->scale());

						MouseDragMsg_p p = MouseDragMsg::create(m_inputId, (MouseButton)i, m_latestPressWidgets[i].rawPtr(), m_latestPressPosition[i], prevPointerPos, pressPosSPX, prevPointerPosSPX, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp);
						p->setCopyTo(m_latestPressWidgets[i].rawPtr());
						Base::msgRouter()->post(p);
					}
				}
			}
		}


		// Update PointerStyle

		PointerStyle newStyle = PointerStyle::Undefined;

		Widget * p = m_latestPressWidgets[button].rawPtr();

		if (button != 0 && p )
			pNowMarked = p;

		if( pNowMarked )
		{
			Widget_p pLastWidget = nullptr;
			auto pWidget = pNowMarked;

			while( pWidget )
			{
				if( !pWidget->isDisabled() )
				{
					auto style = pWidget->pointerStyle();

					if( style != PointerStyle::Undefined )
					{
						newStyle = style;
						break;
					}
				}
				pLastWidget = pWidget;
				pWidget = pWidget->parent();
			}

			if( newStyle == PointerStyle::Undefined )
			{
				if (pLastWidget->_holder())
				{
					auto pRoot = pLastWidget->_holder()->_root();
					if( pRoot )
					{
						auto style = pRoot->pointerStyle();

						if( style != PointerStyle::Undefined )
							newStyle = style;
					}
				}
			}
		}

		if( newStyle == PointerStyle::Undefined )
			newStyle = PointerStyle::Arrow;

		if( newStyle != m_pointerStyle )
		{
			Base::hostBridge()->setPointerStyle(newStyle);
			Base::msgRouter()->post( new PointerChangeMsg( m_inputId, newStyle ) );
			m_pointerStyle = newStyle;
		}
	}

	//____ _updateEnteredWidgets() _________________________________________________

	Widget * InputHandler::_updateEnteredWidgets( Widget * pMarkedWidget, int64_t timestamp )
	{
		// Create new list of entered widgets

		std::vector<Widget_wp>	enteredWidgets;
		for( Widget * pWidget = pMarkedWidget ; pWidget != 0 ; pWidget = pWidget->_parent() )
			enteredWidgets.push_back( pWidget );

		// Send MouseLeave to those that are not in the new list.

		for( auto& weakptr : m_vEnteredWidgets )
		{
			Widget * pWidget = weakptr.rawPtr();

			if ( pWidget && (_widgetPosInList( pWidget, enteredWidgets ) < 0 && !_isInStayEnteredList(pWidget)) )
			{
				MouseLeaveMsg_p p = MouseLeaveMsg::create(m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp);
				p->setCopyTo(pWidget);
				Base::msgRouter()->post(p);
			}
		}

		// Loop through our new widgets and check if they already
		// were entered. Send MouseEnter to all new widgets and notice the first
		// common ancestor.

		Widget * pFirstAlreadyMarked = 0;
		for( auto& weakptr : enteredWidgets )
		{
			Widget * pWidget = weakptr.rawPtr();

			if( _widgetPosInList( pWidget, m_vEnteredWidgets ) < 0 && !_isInStayEnteredList(pWidget) )
			{
				MouseEnterMsg_p p = MouseEnterMsg::create(m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp);
				p->setCopyTo(pWidget);
				Base::msgRouter()->post(p);
			}
			else if( !pFirstAlreadyMarked )
				pFirstAlreadyMarked = pWidget;
		}

		// Replace the old list with a new one.

		m_vEnteredWidgets.swap(enteredWidgets);

		// Return first already marked, calling function might need it.

		return pFirstAlreadyMarked;
	}

	//____ _widgetPosInList() ________________________________________________________

	int InputHandler::_widgetPosInList( const Widget * pWidget, const std::vector<Widget_wp>& list ) const
	{
		for( size_t i = 0 ; i < list.size() ; i++ )
			if( list[i].rawPtr() == pWidget )
				return (int) i;

		return -1;
	}



	//____ setButton() _____________________________________________________________

	void InputHandler::setButton( MouseButton button, bool bPressed, int64_t timestamp )
	{
		// Sanity checks

		if( m_bButtonPressed[(int)button] == bPressed )
			return;

		//

		if( bPressed )
			_processButtonPress( button, timestamp );
		else
			_processButtonRelease( button, timestamp );

		m_bButtonPressed[(int)button] = bPressed;
	}



	//____ _processButtonPress() ___________________________________________________

	void InputHandler::_processButtonPress( MouseButton button, int64_t timestamp )
	{
		if( timestamp == 0 )
			timestamp = m_timestamp;

		Widget* pWidget = m_pMarkedWidget.rawPtr();

		// Check stickiness of focus, i.e. wether widget should keep focus when mouse pressed outside.
		// This needs to be done before we post any MousePress messages, so that any FocusLost messages
		// comes before any FocusGained generated from (and inserted right after) the MousePress
		// message.

		Widget* pFocused = _focusedWidget();
		if (pFocused && (!pWidget->isContainer() || static_cast<Container*>(pWidget)->takesFocusFromChild()) )
		{
			bool focusedIsSameOrAncestor = pFocused == pWidget ? true : pFocused->isContainer() && static_cast<Container*>(pFocused)->contains(pWidget);

			if (!focusedIsSameOrAncestor)
			{
				if (!pFocused->hasStickyFocus())
					pFocused->releaseFocus();
			}
		}

		// Post BUTTON_PRESS events for marked widgets and remember which one we have posted it for

		auto pMsg = MousePressMsg::create( m_inputId, button, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp );
		pMsg->setCopyTo(pWidget);
		Base::msgRouter()->post( pMsg );

		// Handle possible double-click

		int newPressCount = 0;		// Revert back to zero unless conditions below are fullfilled.

		if( m_latestPressTimestamps[(int)button] + m_doubleClickTimeTreshold > timestamp )
		{
			Coord distance = m_pointerPos - m_latestPressPosition[(int)button];

			if( distance.x <= m_doubleClickDistanceTreshold &&
				distance.x >= -m_doubleClickDistanceTreshold &&
				distance.y <= m_doubleClickDistanceTreshold &&
				distance.y >= -m_doubleClickDistanceTreshold )
				{

					switch(m_pressOrdinal[(int)button])
					{
						case 0:
						{
							if (pWidget && pWidget == m_latestPressWidgets[(int)button].rawPtr())
							{
								auto p = MouseDoubleClickMsg::create(m_inputId, button, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp);
								p->setCopyTo(pWidget);
								Base::msgRouter()->post(p);
							}
							else
								Base::msgRouter()->post(MouseDoubleClickMsg::create(m_inputId, button, 0, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp));
							break;
						}

						case 1:
						{
							if (pWidget && pWidget == m_latestPressWidgets[(int)button].rawPtr())
							{
								auto p = MouseTripleClickMsg::create(m_inputId, button, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp);
								p->setCopyTo(pWidget);
								Base::msgRouter()->post(p);
							}
							else
								Base::msgRouter()->post(MouseTripleClickMsg::create(m_inputId, button, 0, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp));
							break;
						}

						default:
							break;
					}

					newPressCount = m_pressOrdinal[(int)button] + 1;
				}
		}


		// Save info for the future

		m_latestPressWidgets[(int)button]		= pWidget;
		m_latestPressTimestamps[(int)button]	= timestamp;
		m_latestPressPosition[(int)button]		= m_pointerPos;
		m_pressOrdinal[(int)button] 				= newPressCount;

	}



	//____ _processButtonRelease() _________________________________________________

	void InputHandler::_processButtonRelease( MouseButton button, int64_t timestamp )
	{
		if( timestamp == 0 )
			timestamp = m_timestamp;


		// Post BUTTON_RELEASE events for widget that was pressed

		Widget * pWidget = m_latestPressWidgets[(int)button].rawPtr();
		bool bIsInside = pWidget ? pWidget->globalGeo().contains( m_pointerPos ) : false;

		auto pMsg = MouseReleaseMsg::create( m_inputId, button, pWidget, bIsInside, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp );
		pMsg->setCopyTo(pWidget);
		Base::msgRouter()->post( pMsg );

		// Post click event, regardless of whether press resulted in a double click.

		if (bIsInside)
		{
			auto p = MouseClickMsg::create(m_inputId, button, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp, int(timestamp - m_latestPressTimestamps[(int)button]), m_pressOrdinal[int(button)]);
			p->setCopyTo(pWidget);
			Base::msgRouter()->post(p);
		}
		else
			Base::msgRouter()->post(MouseClickMsg::create(m_inputId, button, 0, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp, int(timestamp - m_latestPressTimestamps[(int)button]), m_pressOrdinal[int(button)]));
	}


	//____ isButtonPressed() _________________________________________________________

	bool InputHandler::isButtonPressed( MouseButton button ) const
	{
		return m_bButtonPressed[(int)button];
	}

	//____ isAnyButtonPressed() ________________________________________________________

	bool InputHandler::isAnyButtonPressed() const
	{
		for( int i = 0 ; i < MouseButton_size ; i++ )
			if( m_bButtonPressed[i] )
				return true;

		return false;
	}


	//____ setFocusedWindow() ____________________________________________________________

	void InputHandler::setFocusedWindow( Root * pRoot )
	{
		if( pRoot == m_pFocusedRoot.rawPtr() )
			return;

		if( m_pFocusedRoot )
		{
			Widget * p = m_pFocusedRoot->_focusedChild();
			if( p )
				_setUnfocused( p );
		}
		m_pFocusedRoot = pRoot;

		if( pRoot )
		{
			Widget * p = pRoot->_focusedChild();

			if( p )
				_setFocused( p );
			else
				_setFocused( pRoot->_child() );				// Bottom container gets focus per default.
		}
	}

	//____ _focusChanged() _____________________________________________________

	bool InputHandler::_focusChanged( Root * pRoot, Widget * pOldFocused, Widget * pNewFocused )
	{
		if (pRoot != m_pFocusedRoot.rawPtr() )
			return false;					// Root does not have focus, thus can't assign focus to its child.

		if( pOldFocused )
			_setUnfocused( pOldFocused );

		if (pNewFocused )
			_setFocused(pNewFocused);

		return true;
	}

	//____ _focusedWidget() ____________________________________________________

	Widget * InputHandler::_focusedWidget() const
	{
		if( m_pFocusedRoot )
			return m_pFocusedRoot->_focusedChild();

		return 0;
	}

	//____ _setFocused() _______________________________________________________

	void InputHandler::_setFocused( Widget * pWidget )
	{
		FocusGainedMsg_p pMsg = new FocusGainedMsg( m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, m_timestamp );
		Base::msgRouter()->post( pMsg );
	}

	//____ _setUnfocused() _______________________________________________________

	void InputHandler::_setUnfocused( Widget * pWidget )
	{
		FocusLostMsg_p pMsg = new FocusLostMsg( m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, m_timestamp );
		Base::msgRouter()->post( pMsg );
	}

	//____ _yieldButtonEvents() ______________________________________________

	void InputHandler::_yieldButtonEvents(MouseButton button, Widget * pFrom, Widget * pTo)
	{
		if (m_bButtonPressed[(int)button] && m_latestPressWidgets[(int)button] == pFrom)
			m_latestPressWidgets[(int)button] = pTo;
	}


	//____ setKey() ________________________________________________________________

	void InputHandler::setKey( int nativeKeyCode, bool bPressed, int64_t timestamp )
	{
		if( timestamp == 0 )
			timestamp = m_timestamp;

		if( bPressed )
			_processKeyPress( nativeKeyCode, timestamp );
		else
			_processKeyRelease( nativeKeyCode, timestamp );
	}


	//____ _processKeyPress() ______________________________________________________

	void InputHandler::_processKeyPress( int nativeKeyCode, int64_t timestamp )
	{
		// Verify that this key isn't already pressed

		for( auto i : m_keysDown )
		{
			if( i.nativeKeyCode == nativeKeyCode )
				return;
		}

		//

		Key translatedKeyCode = translateKey(nativeKeyCode);

		// Create and fill in the info-structure.

		KeyDownInfo info;
		info.nativeKeyCode = nativeKeyCode;
		info.translatedKeyCode = translatedKeyCode;
		info.pressTimestamp = timestamp;
		info.pWidget = _focusedWidget();

		m_keysDown.push_back( info );

		// Update modkeys

		switch(translatedKeyCode)
		{
			case Key::Shift:
				m_modKeys = (ModKeys)(m_modKeys | ModKeys::Shift);
				break;
			case Key::Alt:
				m_modKeys = (ModKeys)(m_modKeys | ModKeys::Alt);
				break;
			case Key::StdControl:
				m_modKeys = (ModKeys)(m_modKeys | ModKeys::StdCtrl);
				break;
			case Key::MacControl:
				m_modKeys = (ModKeys)(m_modKeys | ModKeys::MacCtrl);
				break;
			case Key::OSKey:
				m_modKeys = (ModKeys)(m_modKeys | ModKeys::OSKey);
				break;
			default:
				break;
		}

		// Post KEY_PRESS message

		Widget * pWidget = _focusedWidget();
		Base::msgRouter()->post( new KeyPressMsg( m_inputId, nativeKeyCode, translatedKeyCode, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp ) );

		// Post an EditCommand if that is associated with the key-combo.

		EditCmd cmd = translateCommand( nativeKeyCode, m_modKeys );
		if( cmd != EditCmd::None )
			Base::msgRouter()->post( new EditCommandMsg( m_inputId, cmd, pWidget ));

	}


	//____ _processKeyRelease() ___________________________________________________

	void InputHandler::_processKeyRelease( int nativeKeyCode, int64_t timestamp )
	{
		// Find right KeyDownInfo structure and remove it from m_keysDown.

		auto it = m_keysDown.begin();
		while( it != m_keysDown.end() )
		{
			if( nativeKeyCode == it->nativeKeyCode )
			{
				m_keysDown.erase(it);
				break;
			}
			it++;
		}

		// Post KEY_RELEASE message

		Key translatedKeyCode = translateKey(nativeKeyCode);

		Widget * pWidget = _focusedWidget();
		Base::msgRouter()->post( new KeyReleaseMsg( m_inputId, nativeKeyCode, translatedKeyCode, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp ) );

		// Update modkeys

		switch( translatedKeyCode )
		{
			case Key::Shift:
				m_modKeys = (ModKeys) (m_modKeys & ~ModKeys::Shift);
				break;
			case Key::Alt:
				m_modKeys = (ModKeys) (m_modKeys & ~ModKeys::Alt);
				break;
			case Key::StdControl:
				m_modKeys = (ModKeys) (m_modKeys & ~ModKeys::StdCtrl);
				break;
			case Key::MacControl:
				m_modKeys = (ModKeys) (m_modKeys & ~ModKeys::MacCtrl);
				break;
			case Key::OSKey:
				m_modKeys = (ModKeys) (m_modKeys & ~ModKeys::OSKey);
				break;
			default:
				break;
		}
	}


	//____ setWheelRoll() __________________________________________________________

	void InputHandler::setWheelRoll( int wheel, pts distance, bool bInvertScroll, int64_t timestamp )
	{
		if( m_pMarkedWidget )
		{
			if( timestamp == 0 )
				timestamp = m_timestamp;
			Base::msgRouter()->post( new WheelRollMsg( m_inputId, wheel, distance, bInvertScroll, m_pMarkedWidget.rawPtr(), m_modKeys, m_pointerPos, m_pointerPosSPX, timestamp ) );
		}
	}

	//____ isKeyPressed() ______________________________________________________

	bool InputHandler::isKeyPressed( int nativeKeyCode ) const
	{
		for( auto key : m_keysDown )
			if( key.nativeKeyCode == nativeKeyCode )
				return true;

		return false;
	}

	//____ isEntered() ___________________________________________________________

	bool InputHandler::isEntered( Widget * pWidget ) const
	{
		if( _widgetPosInList( pWidget, m_vEnteredWidgets ) >= 0 )
			return true;

		return _isInStayEnteredList(pWidget);
	}

	//____ _isInStayEnteredList() ____________________________________________________

	bool InputHandler::_isInStayEnteredList( Widget * pWidget ) const
	{
		for( auto& vec : m_stayEnteredLists )
		{
			if( _widgetPosInList( pWidget, vec.second ) >= 0 )
				return true;
		}

		return false;
	}

	//____ _update() _________________________________________________________________

	void InputHandler::_update( int64_t timestamp )
	{
		_handleButtonRepeats( timestamp );
		_handleKeyRepeats( timestamp );

		m_timestamp = timestamp;

		// If mouse doesn't move but a new widet shows up under it, we should wait until that widget doesn't move
		// and then mark it.

		if (m_bPointerMovedSinceUpdate)
		{
			m_pWidgetUnderStillPointer = nullptr;

			m_bPointerMovedSinceUpdate = false;
		}
		else
		{
			auto	pRoot = m_pMarkedRoot;
			auto&	pos	= m_pointerPos;


			if (pRoot && pRoot->geo().contains(pos))
			{
				Widget* pMarked = pRoot->findWidget(m_pointerPos, SearchMode::ActionTarget);
				if (pMarked)
				{
					RectSPX markedGeo = pMarked->_globalGeo();

					if (pMarked != m_pWidgetUnderStillPointer || markedGeo != m_widgetUnderStillPointerGeo)
					{
						m_pWidgetUnderStillPointer = pMarked;
						m_widgetUnderStillPointerGeo = markedGeo;

						m_lastChangeUnderStillPointer = timestamp;
					}
					else if(m_lastChangeUnderStillPointer + m_stillPointerMarkDelay < timestamp )
						setPointer(pRoot, pos);
				}
			}
		}

	}

	//____ _handleButtonRepeats() _________________________________________

	void InputHandler::_handleButtonRepeats( int64_t timestamp )
	{
		for( int button = 0 ; button < MouseButton_size ; button++ )
		{
			if( m_bButtonPressed[button] )
			{
				int64_t firstRepeat = m_latestPressTimestamps[button] + m_buttonRepeatDelay;
				int64_t repeatPos;			// Next timestamp where a button repeat should occur.

				if( m_timestamp < firstRepeat )
					repeatPos = firstRepeat;
				else
					repeatPos = firstRepeat + ((m_timestamp - firstRepeat) / m_buttonRepeatRate) * m_buttonRepeatRate + m_buttonRepeatRate;

				while( repeatPos <= timestamp )
				{
					auto pWidget = m_latestPressWidgets[button].rawPtr();
					auto pMsg = MouseRepeatMsg::create(m_inputId, (MouseButton)button, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, repeatPos);
					pMsg->setCopyTo(pWidget);
					Base::msgRouter()->post(pMsg);
					repeatPos += m_keyRepeatRate;
				}
			}
		}
	}


	//____ _handleKeyRepeats() _________________________________________

	void InputHandler::_handleKeyRepeats( int64_t timestamp )
	{
		for( auto key : m_keysDown )
		{
			int64_t firstRepeat = key.pressTimestamp + m_keyRepeatDelay;
			int64_t repeatPos;			// Next timestamp where a key repeat should occur.

			if( m_timestamp < firstRepeat )
				repeatPos = firstRepeat;
			else
				repeatPos = firstRepeat + ((m_timestamp - firstRepeat) / m_keyRepeatRate) * m_keyRepeatRate + m_keyRepeatRate;

			while( repeatPos <= timestamp )
			{
				auto pWidget = key.pWidget.rawPtr();
				auto pMsg = new KeyRepeatMsg(m_inputId, key.nativeKeyCode, key.translatedKeyCode, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, repeatPos);
				pMsg->setCopyTo(pWidget);
				Base::msgRouter()->post(pMsg);

				EditCmd cmd = translateCommand( key.nativeKeyCode, m_modKeys );
				if( cmd != EditCmd::None )
					Base::msgRouter()->post( new EditCommandMsg( m_inputId, cmd, key.pWidget.rawPtr() ));

				repeatPos += m_keyRepeatRate;
			}
		}
	}

	//____ lockHidePointer() __________________________________________________

	bool InputHandler::lockHidePointer()
	{
		auto pHost = Base::hostBridge();
		if (pHost)
			return pHost->lockHidePointer();
		else
			return false;
	}

	//____ unlockShowPointer() __________________________________________________

	bool InputHandler::unlockShowPointer()
	{
		auto pHost = Base::hostBridge();
		if (pHost)
			return pHost->unlockShowPointer();
		else
			return false;
	}

	//____ setStayEnteredList() _____________________________________________

	void InputHandler::setStayEnteredList( intptr_t listId, Widget ** pBegin, Widget ** pEnd )
	{
		std::vector<Widget_wp> vec;

		// Only alredy entered widgets can stay entered

		while( pBegin < pEnd )
		{
			Widget * pWidget = * pBegin++;
			if( isEntered(pWidget) )
				vec.push_back(pWidget);
		}

		// If this releases a previous list, we might need to leave some widgets

		auto it = m_stayEnteredLists.find(listId );
		if( it != m_stayEnteredLists.end() )
		{
			it->second.swap( vec );

			for( auto& pWidget : vec )
			{
				if( pWidget != nullptr && !isEntered(pWidget) )
				{
					MouseLeaveMsg_p p = MouseLeaveMsg::create(m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, m_timestamp);
					p->setCopyTo(pWidget);
					Base::msgRouter()->post(p);
				}
			}
		}
		else
		{
			m_stayEnteredLists[listId] = vec;
		}
	}

//____ clearStayEnteredList() _____________________________________________

	void InputHandler::clearStayEnteredList( intptr_t listId )
	{
		auto it = m_stayEnteredLists.find(listId );
		if( it != m_stayEnteredLists.end() )
		{
			std::vector<Widget_wp> vec;

			it->second.swap( vec );
			m_stayEnteredLists.erase(it);

			for( auto& pWidget : vec )
			{
				if( pWidget != nullptr && !isEntered(pWidget) )
				{
					MouseLeaveMsg_p p = MouseLeaveMsg::create(m_inputId, pWidget, m_modKeys, m_pointerPos, m_pointerPosSPX, m_timestamp);
					p->setCopyTo(pWidget);
					Base::msgRouter()->post(p);
				}
			}
		}
	}


} // namespace wg
