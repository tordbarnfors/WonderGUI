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
#include <cstdio>

#include <wg2_eventlogger.h>
#include <wg2_widget.h>

using std::string;

//____ Constructor ____________________________________________________________

WgEventLogger::WgEventLogger( std::ostream& stream ) : m_pStream(&stream)
{
	LogAllEvents();
	IgnoreEvent( WG_EVENT_TICK );
}

WgEventLogger::WgEventLogger( const std::function<void(const char*)> func ) : m_func(func)
{
	LogAllEvents();
	IgnoreEvent( WG_EVENT_TICK );
}


//____ Destructor _____________________________________________________________

WgEventLogger::~WgEventLogger()
{
}

//____ IgnoreEvent ____________________________________________________________

void WgEventLogger::IgnoreEvent( WgEventType event )
{
	m_eventFilter[event] = false;
}

//____ LogEvent _______________________________________________________________

void WgEventLogger::LogEvent( WgEventType event )
{
	m_eventFilter[event] = true;
}

//____ IgnorePointerEvents ____________________________________________________________

void WgEventLogger::IgnorePointerEvents()
{
	m_eventFilter[WG_EVENT_MOUSE_ENTER] = false;
	m_eventFilter[WG_EVENT_MOUSE_MOVE] = false;
	m_eventFilter[WG_EVENT_MOUSE_POSITION] = false;
	m_eventFilter[WG_EVENT_MOUSE_LEAVE] = false;
}

//____ LogPointerEvents _______________________________________________________________

void WgEventLogger::LogPointerEvents()
{
	m_eventFilter[WG_EVENT_MOUSE_ENTER] = true;
	m_eventFilter[WG_EVENT_MOUSE_MOVE] = true;
	m_eventFilter[WG_EVENT_MOUSE_POSITION] = true;
	m_eventFilter[WG_EVENT_MOUSE_LEAVE] = true;
}


//____ IgnoreMouseButtonEvents ____________________________________________________________

void WgEventLogger::IgnoreMouseButtonEvents()
{
	m_eventFilter[WG_EVENT_MOUSEBUTTON_PRESS] = false;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_REPEAT] = false;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_DRAG] = false;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_RELEASE] = false;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_CLICK] = false;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK] = false;
}

//____ LogMouseButtonEvents _______________________________________________________________

void WgEventLogger::LogMouseButtonEvents()
{
	m_eventFilter[WG_EVENT_MOUSEBUTTON_PRESS] = true;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_REPEAT] = true;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_DRAG] = true;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_RELEASE] = true;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_CLICK] = true;
	m_eventFilter[WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK] = true;
}

//____ IgnoreKeyboardEvents ____________________________________________________________

void WgEventLogger::IgnoreKeyboardEvents()
{
	m_eventFilter[WG_EVENT_KEY_PRESS] = false;
	m_eventFilter[WG_EVENT_KEY_REPEAT] = false;
	m_eventFilter[WG_EVENT_KEY_RELEASE] = false;
	m_eventFilter[WG_EVENT_CHARACTER] = false;
}

//____ LogKeyboardEvents _______________________________________________________________

void WgEventLogger::LogKeyboardEvents()
{
	m_eventFilter[WG_EVENT_KEY_PRESS] = true;
	m_eventFilter[WG_EVENT_KEY_REPEAT] = true;
	m_eventFilter[WG_EVENT_KEY_RELEASE] = true;
	m_eventFilter[WG_EVENT_CHARACTER] = true;
}

//____ LogMouseEvents() _______________________________________________________

void WgEventLogger::LogMouseEvents()
{
	LogPointerEvents();
	LogMouseButtonEvents();
	LogEvent( WG_EVENT_MOUSEWHEEL_ROLL );
}

//____ IgnoreMouseEvents() ____________________________________________________

void WgEventLogger::IgnoreMouseEvents()
{
	IgnorePointerEvents();
	IgnoreMouseButtonEvents();
	IgnoreEvent( WG_EVENT_MOUSEWHEEL_ROLL );
}


//____ LogInputEvents() _______________________________________________________

void WgEventLogger::LogInputEvents()
{
	LogMouseEvents();
	LogKeyboardEvents();
}

//____ IgnoreInputEvents() ____________________________________________________

void WgEventLogger::IgnoreInputEvents()
{
	IgnoreMouseEvents();
	IgnoreKeyboardEvents();
}




//____ IgnoreAllEvents ________________________________________________________

void WgEventLogger::IgnoreAllEvents()
{
	for( int i = 0 ; i < WG_EVENT_MAX ; i++ )
		m_eventFilter[i] = false;
}

//____ LogAllEvents ___________________________________________________________
void WgEventLogger::LogAllEvents()
{
	for( int i = 0 ; i < WG_EVENT_MAX ; i++ )
		m_eventFilter[i] = true;
}



//____ OnEvent() ______________________________________________________________

void WgEventLogger::OnEvent( const WgEvent::Event * _pEvent )
{
	if( m_eventFilter[_pEvent->Type()] == false )
		return;

	string	timestamp;
	string	id;
	string	widget;
	string	modkeys;
	string	pointerPos;

	char	params[256]; params[0] = 0;			// Event specific parameters


	timestamp = _formatTimestamp( _pEvent->Timestamp() );

	switch( _pEvent->Type() )
	{
		case WG_EVENT_DUMMY:
			id = "Dummy";
			break;
		case WG_EVENT_TICK:
			id = "Tick";
			snprintf( params, 256, " millisec=%d", ((const WgEvent::Tick*)_pEvent)->Millisec() );
			break;
		case WG_EVENT_POINTER_CHANGE:
			id = "PointerChange";
			snprintf( params, 256, " style=%s", _formatPointerStyle( (const WgEvent::PointerChange*)_pEvent).c_str() );
			break;
		case WG_EVENT_FOCUS_GAINED:
			id = "FocusGained";
			break;
		case WG_EVENT_FOCUS_LOST:
			id = "FocusLost";
			break;
		case WG_EVENT_MOUSE_ENTER:
			id = "MouseEnter";
			break;
		case WG_EVENT_MOUSE_MOVE:
			id = "MouseMove";
			break;
		case WG_EVENT_MOUSE_POSITION:
			id = "MousePosition";
			break;
		case WG_EVENT_MOUSE_LEAVE:
			id = "MouseLeave";
			break;
		case WG_EVENT_MOUSEBUTTON_PRESS:
			id = "MouseButtonPress";
			snprintf( params, 256, " button=%d", ((const WgEvent::MouseButtonPress*)_pEvent)->Button() );
			break;
		case WG_EVENT_MOUSEBUTTON_REPEAT:
			id = "MouseButtonRepeat";
			snprintf( params, 256, " button=%d", ((const WgEvent::MouseButtonRepeat*)_pEvent)->Button() );
			break;
		case WG_EVENT_MOUSEBUTTON_DRAG:
		{
			id = "MouseButtonDrag";

			const WgEvent::MouseButtonDrag * pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

			WgCoord	now		= pEvent->CurrPixelPos();
			WgCoord 	prev	= pEvent->PrevPixelPos();
			WgCoord	start	= pEvent->StartPixelPos();

			WgCoord	dragNow	= pEvent->DraggedNowPixels();
			WgCoord	dragTotal=pEvent->DraggedTotalPixels();

			snprintf( params, 256, " button=%d position(start=%d,%d prev=%d,%d now=%d,%d) dragged(now=%d,%d total=%d,%d)",
					pEvent->Button(), start.x, start.y, prev.x, prev.y, now.x, now.y, dragNow.x, dragNow.y, dragTotal.x, dragTotal.y );
			break;
		}
		case WG_EVENT_MOUSEBUTTON_RELEASE:
		{
			id = "MouseButtonRelease";

			const WgEvent::MouseButtonRelease * pEvent = static_cast<const WgEvent::MouseButtonRelease*>(_pEvent);

			const static char outside[] = "outside";
			const static char inside[] = "inside";
			const char * pPress = outside;
			const char * pRelease = outside;

			if( pEvent->PressInside() )
				pPress = inside;

			if( pEvent->ReleaseInside() )
				pRelease = inside;

			snprintf( params, 256, " button=%d press=%s release=%s", pEvent->Button(), pPress, pRelease );
			break;
		}
		case WG_EVENT_MOUSEBUTTON_CLICK:
			id = "MouseButtonClick";
			snprintf( params, 256, " button=%d", static_cast<const WgEvent::MouseButtonClick*>(_pEvent)->Button() );
			break;
		case WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK:
			id = "MouseButtonDoubleClick";
			snprintf( params, 256, " button=%d", static_cast<const WgEvent::MouseButtonDoubleClick*>(_pEvent)->Button() );
			break;

		case WG_EVENT_KEY_PRESS:
		{
			id = "KeyPress";
			const WgEvent::KeyPress * pEvent = static_cast<const WgEvent::KeyPress*>(_pEvent);
			snprintf( params, 256, "wg_keycode=%d native_keycode=%d", pEvent->TranslatedKeyCode(), pEvent->NativeKeyCode() );
			break;
		}
		case WG_EVENT_KEY_REPEAT:
		{
			id = "KeyRepeat";
			const WgEvent::KeyRepeat * pEvent = static_cast<const WgEvent::KeyRepeat*>(_pEvent);
			snprintf( params, 256, "wg_keycode=%d native_keycode=%d", pEvent->TranslatedKeyCode(), pEvent->NativeKeyCode() );
			break;
		}
		case WG_EVENT_KEY_RELEASE:
		{
			id = "KeyRelease";
			const WgEvent::KeyRelease * pEvent = static_cast<const WgEvent::KeyRelease*>(_pEvent);
			snprintf( params, 256, "wg_keycode=%d native_keycode=%d", pEvent->TranslatedKeyCode(), pEvent->NativeKeyCode() );
			break;
		}
		case WG_EVENT_CHARACTER:
			id = "Character";
			snprintf( params, 256, " char=%X", static_cast<const WgEvent::Character*>(_pEvent)->Char() );
			break;
		case WG_EVENT_MOUSEWHEEL_ROLL:
		{
			id = "MouseWheelRoll";
			const WgEvent::MouseWheelRoll * pEvent = static_cast<const WgEvent::MouseWheelRoll*>(_pEvent);
			snprintf( params, 256, "wheel=%d distance=%d invertScroll=%d", pEvent->Wheel(), pEvent->Distance(), (int) pEvent->InvertScroll() );
			break;
		}

		case WG_EVENT_DROP_PICK:
			id = "DropPick";
			break;
		case WG_EVENT_DROP_PROBE:
			id = "DropProbe";
			break;
		case WG_EVENT_DROP_ENTER:
			id = "DropEnter";
			break;
		case WG_EVENT_DROP_MOVE:
			id = "DropMove";
			break;
		case WG_EVENT_DROP_LEAVE:
			id = "DropLeave";
			break;
		case WG_EVENT_DROP_DELIVER:
			id = "DropDeliver";
			break;
		case WG_EVENT_DROP_CANCEL:
			id = "DropCancel";
			break;
		case WG_EVENT_DROP_COMPLETE:
			id = "DropComplete";
			break;

		case WG_EVENT_DROPHOVER_ENTER:
			id = "DropHoverEnter";
			break;
		case WG_EVENT_DROPHOVER_MOVE:
			id = "DropHoverMove";
			break;
		case WG_EVENT_DROPHOVER_LEAVE:
			id = "DropHoverLeave";
			break;


		case WG_EVENT_LANES_DROP:
			id = "LanesDrop";
			break;

		case WG_EVENT_SELECTED:
			id = "Selected";
			break;

		case WG_EVENT_BUTTON_PRESS:
			id = "ButtonPress";
			break;

		case WG_EVENT_CHECKBOX_CHECK:
			id = "CheckboxCheck";
			break;

		case WG_EVENT_CHECKBOX_UNCHECK:
			id = "CheckboxUncheck";
			break;

		case WG_EVENT_CHECKBOX_TOGGLE:
			id = "CheckboxToggle";
			snprintf( params, 256, "checked=%d", (int) (static_cast<const WgEvent::CheckboxToggle*>(_pEvent)->IsChecked()) );
			break;

		case WG_EVENT_RADIOBUTTON_SELECT:
			id = "RadiobuttonSelect";
			break;

		case WG_EVENT_RADIOBUTTON_UNSELECT:
			id = "RadiobuttonUnselect";
			break;

		case WG_EVENT_RADIOBUTTON_TOGGLE:
			id = "RadiobuttonToggle";
			snprintf( params, 256, "selected=%d", (int) (static_cast<const WgEvent::RadiobuttonToggle*>(_pEvent)->IsSelected()) );
			break;

		case WG_EVENT_ANIMATION_UPDATE:
		{
			id = "AnimationUpdate";
			const WgEvent::AnimationUpdate * pEvent = static_cast<const WgEvent::AnimationUpdate*>(_pEvent);
			snprintf( params, 256, "frame=%d fraction=%f", pEvent->Frame(), pEvent->Fraction() );
			break;
		}

		case WG_EVENT_SLIDER_MOVE:
		{
			id = "SliderMove";
			const WgEvent::SliderEvent * pEvent = static_cast<const WgEvent::SliderEvent*>(_pEvent);
			snprintf( params, 256, "pos=%f length=%f", pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_SLIDER_STEP_FWD:
		{
			id = "SliderStepFwd";
			const WgEvent::SliderEvent * pEvent = static_cast<const WgEvent::SliderEvent*>(_pEvent);
			snprintf( params, 256, "pos=%f length=%f", pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_SLIDER_STEP_BWD:
		{
			id = "SliderStepBwd";
			const WgEvent::SliderEvent * pEvent = static_cast<const WgEvent::SliderEvent*>(_pEvent);
			snprintf( params, 256, "pos=%f length=%f", pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_SLIDER_JUMP_FWD:
		{
			id = "SliderJumpFwd";
			const WgEvent::SliderEvent * pEvent = static_cast<const WgEvent::SliderEvent*>(_pEvent);
			snprintf( params, 256, "pos=%f length=%f", pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_SLIDER_JUMP_BWD:
		{
			id = "SliderJumpBwd";
			const WgEvent::SliderEvent * pEvent = static_cast<const WgEvent::SliderEvent*>(_pEvent);
			snprintf( params, 256, "pos=%f length=%f", pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_SLIDER_WHEEL_ROLL:
		{
			id = "SliderWheelRoll";
			const WgEvent::SliderWheelRolled * pEvent = static_cast<const WgEvent::SliderWheelRolled*>(_pEvent);
			snprintf( params, 256, "distance=%d pos=%f length=%f", pEvent->Distance(), pEvent->Pos(), pEvent->Length() );
			break;
		}

		case WG_EVENT_TEXT_MODIFY:
		{
			id = "TextModify";
			const WgEvent::TextEvent * pEvent = static_cast<const WgEvent::TextEvent*>(_pEvent);
			break;
		}

		case WG_EVENT_TEXT_SET:
		{
			id = "TextSet";
			const WgEvent::TextEvent * pEvent = static_cast<const WgEvent::TextEvent*>(_pEvent);
			break;
		}

		case WG_EVENT_KNOB_TURN:
		{
			id = "KnobTurn";
			const WgEvent::KnobTurn* pEvent = static_cast<const WgEvent::KnobTurn*>(_pEvent);
			snprintf(params, 256, "pos=%d fraction=%f", pEvent->Pos(), pEvent->Fraction());
			break;
		}

		case WG_EVENT_RANGE_CHANGED:
		{
			id = "RangeChanged";
			const WgEvent::RangeChanged* pEvent = static_cast<const WgEvent::RangeChanged*>(_pEvent);
			snprintf(params, 256, "begin=%f end=%f", pEvent->Begin(), pEvent->End());
			break;
		}

		/*
		case WG_EVENT_MENUITEM_SELECT:
		{
			id = "MenuitemSelect";
			const WgEvent::MenuitemEvent * pEvent = static_cast<const WgEvent::MenuitemEvent*>(_pEvent);
			snprintf( params, 256, "itemId=%d", pEvent->ItemId() );
			break;
		}

		case WG_EVENT_MENUITEM_CHECK:
		{
			id = "MenuitemCheck";
			const WgEvent::MenuitemEvent * pEvent = static_cast<const WgEvent::MenuitemEvent*>(_pEvent);
			snprintf( params, 256, "itemId=%d", pEvent->ItemId() );
			break;
		}

		case WG_EVENT_MENUITEM_UNCHECK:
		{
			id = "MenuitemUncheck";
			const WgEvent::MenuitemEvent * pEvent = static_cast<const WgEvent::MenuitemEvent*>(_pEvent);
			snprintf( params, 256, "itemId=%d", pEvent->ItemId() );
			break;
		}

		case WG_EVENT_MENU_CLOSED:
		{
			id = "MenuClosed";
			const WgEvent::MenuClosed * pEvent = static_cast<const WgEvent::MenuClosed*>(_pEvent);
			snprintf( params, 256, "menu=%ld", (long int)pEvent->Menu() );
			break;
		}
				*/

		case WG_EVENT_MODAL_MOVE_OUTSIDE:
			id = "ModalMoveOutside";
			break;
		case WG_EVENT_MODAL_BLOCKED_PRESS:
			id = "ModalBlockedPress";
			snprintf( params, 256, " button=%d", ((const WgEvent::ModalBlockedPress*)_pEvent)->Button() );
			break;
		case WG_EVENT_MODAL_BLOCKED_RELEASE:
			id = "ModalBlockedRelease";
			snprintf( params, 256, " button=%d", ((const WgEvent::ModalBlockedRelease*)_pEvent)->Button() );
			break;

		case WG_EVENT_EAVESDROP:
		{
			id = "Eavesdrop";
			WgEvent::Event * pOrgEvent = ((const WgEvent::Eavesdrop*)_pEvent)->Event();
			string orgWidget = _formatWidget(pOrgEvent);

			snprintf( params, 256, " org event=%d for %s", pOrgEvent->Type(), orgWidget.c_str() );
			break;
		}

		default:
			id = "Unknown Event";
			snprintf( params, 256, " type enum=%d", (_pEvent)->Type() );
			break;
	};

	widget = _formatWidget( _pEvent );
	modkeys = _formatModkeys( _pEvent );
	pointerPos = _formatPointerPos( _pEvent );

	if( m_func )
	{
		const int 	c_len = 2048;
		char		output[c_len];
		
		snprintf( output, c_len, " %s - %s - %s%s%s%s\n", timestamp.c_str(), id.c_str(), widget.c_str(), pointerPos.c_str(), modkeys.c_str(), params );
		m_func(output);
	}
	else
	{
		* m_pStream << timestamp.c_str() << " - " << id.c_str() << " - " << widget.c_str() << pointerPos.c_str() << modkeys.c_str() << params;
		* m_pStream << std::endl;
	}
}


//____ _formatTimestamp() ______________________________________________________

string WgEventLogger::_formatTimestamp( int64_t ms )
{
	char	temp[20];

	int milli	= (int) (ms % 1000);
	int sec		= (int) ((ms/1000) % 60);
	int min		= (int) ((ms/60000) % 60);
	int hour	= (int) (ms/3600000);

	snprintf( temp, 20, "%d:%02d:%02d.%03d", hour, min, sec, milli );
	return string( temp );
}

//____ _formatWidget() __________________________________________________________

string WgEventLogger::_formatWidget( const WgEvent::Event * _pEvent )
{
	std::string	out;

	if( _pEvent->IsForWidget() || _pEvent->Widget() )
	{
		char	temp[64];
		WgWidget * pWidget = _pEvent->Widget();

		static const char def_type[] = "deleted";
		const char * pType = def_type;

		if( pWidget )
			pType = pWidget->Type();

		snprintf( temp, 64, " widget=%p (%s)", pWidget, pType );
		out = temp;
	}

	return out;
}

//____ _formatModkeys() __________________________________________________________

string WgEventLogger::_formatModkeys( const WgEvent::Event * _pEvent )
{
	WgModifierKeys keys = _pEvent->ModKeys();

	string	out;

	if( keys != WG_MODKEY_NONE )
	{
		char	temp[64];
		snprintf( temp, 64, " modkeys=%X", keys );
		out = temp;
	}

	return out;
}

//____ _formatPointerPos() _____________________________________________________

string WgEventLogger::_formatPointerPos( const WgEvent::Event * _pEvent )
{
	WgCoord localPos = _pEvent->PointerPixelPos();
	WgCoord globalPos = _pEvent->PointerScreenPixelPos();


	char	temp[64];

	if( globalPos == localPos )
		snprintf( temp, 64, " pointer=%d,%d", localPos.x, localPos.y );
	else
		snprintf( temp, 64, " pointer=%d,%d (%d,%d)", localPos.x, localPos.y, globalPos.x, globalPos.y );

	return string(temp);
}

//____ _formatPointerStyle() _____________________________________________________

string WgEventLogger::_formatPointerStyle( const WgEvent::PointerChange * _pEvent )
{
	switch( _pEvent->Style() )
	{
		case WgPointerStyle::Arrow:
			return "WgPointerStyle::Arrow";
		case WgPointerStyle::Hourglass:
			return "WgPointerStyle::Hourglass";
		case WgPointerStyle::Hand:
			return "WgPointerStyle::Hand";
		case WgPointerStyle::OpenHand:
			return "WgPointerStyle::OpenHand";
		case WgPointerStyle::ClosedHand:
			return "WgPointerStyle::ClosedHand";
		case WgPointerStyle::Crosshair:
			return "WgPointerStyle::Crosshair";
		case WgPointerStyle::Help:
			return "WgPointerStyle::Help";
		case WgPointerStyle::Ibeam:
			return "WgPointerStyle::Ibeam";
		case WgPointerStyle::Stop:
			return "WgPointerStyle::Stop";
		case WgPointerStyle::UpArrow:
			return "WgPointerStyle::UpArrow";
		case WgPointerStyle::ResizeAll:
			return "WgPointerStyle::ResizeAll";
		case WgPointerStyle::ResizeNeSw:
			return "WgPointerStyle::ResizeNeSw";
		case WgPointerStyle::ResizeNwSe:
			return "WgPointerStyle::ResizeNwSe";
		case WgPointerStyle::ResizeNS:
			return "WgPointerStyle::ResizeNS";
		case WgPointerStyle::ResizeWE:
			return "WgPointerStyle::ResizeWE";
		case WgPointerStyle::ResizeBeamNS:
			return "WgPointerStyle::ResizeBeamNS";
		case WgPointerStyle::ResizeBeamWE:
			return "WgPointerStyle::ResizeBeamWE";
		default:
		{
			char	temp[64];
			snprintf( temp, 64, "%d (unkown enum)", (int) _pEvent->Style() );
			return string(temp);
		}
	}



}

