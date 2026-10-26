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
#include <assert.h>
#include <wg2_key.h>
#include <wg2_event.h>
#include <wg2_eventfilter.h>
#include <wg2_widget.h>
#include <wg2_slider.h>
#include <wg2_checkbox.h>
#include <wg2_radiobutton.h>
#include <wg2_animplayer.h>

#ifdef SOFTUBE_USE_PACE_FUSION
#include "PaceFusionAnnotations.h"
#endif



//____ Source() _________________________________________________________________

WgEventFilter WgEventFilter::Source(WgWidget * pWidget)
{
	return WgEventFilter(WG_EVENT_DUMMY, pWidget, _filterWidget);
}

//____ Type() _________________________________________________________________

WgEventFilter WgEventFilter::Type(int type)
{
	return WgEventFilter((WgEventType) type, 0, _filterType);
}

//____ WidgetId() _________________________________________________________________

WgEventFilter WgEventFilter::WidgetId(int id)
{
	return WgEventFilter(WG_EVENT_DUMMY, 0, _filterWidgetId, id);
}

//____ TypeAndSource() _________________________________________________________________

WgEventFilter WgEventFilter::TypeAndSource(int type, WgWidget * pWidget)
{
	return WgEventFilter((WgEventType)type, pWidget, _filterTypeWidget);
}

//____ TypeAndWidgetId() _________________________________________________________________

WgEventFilter WgEventFilter::TypeAndWidgetId(int type, int id)
{
	return WgEventFilter((WgEventType)type, 0, _filterTypeWidgetId, id);
}

//____ Tick() _________________________________________________________________

WgEventFilter	WgEventFilter::Tick()
{
	return WgEventFilter( WG_EVENT_TICK, 0, _filterType );
}

//____ PointerChange() _________________________________________________________________

WgEventFilter	WgEventFilter::PointerChange()
{
	return WgEventFilter( WG_EVENT_POINTER_CHANGE, 0, _filterType );
}

//____ FocusGained() __________________________________________________________

WgEventFilter	WgEventFilter::FocusGained()
{
	return WgEventFilter( WG_EVENT_FOCUS_GAINED, 0, _filterType );
}

WgEventFilter	WgEventFilter::FocusGained( WgWidget * pWidget )
{
	return WgEventFilter(WG_EVENT_FOCUS_GAINED, pWidget, _filterTypeWidget);
}


//____ FocusGained() __________________________________________________________

WgEventFilter	WgEventFilter::FocusLost()
{
	return WgEventFilter( WG_EVENT_FOCUS_LOST, 0, _filterType );
}

WgEventFilter	WgEventFilter::FocusLost( WgWidget * pWidget )
{
	return WgEventFilter(WG_EVENT_FOCUS_LOST, pWidget, _filterType);
}

//____ MouseEnter() _________________________________________________________

WgEventFilter	WgEventFilter::MouseEnter()
{
	return WgEventFilter( WG_EVENT_MOUSE_ENTER, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::MouseEnter( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSE_ENTER, pWidget, _filterTypeWidget );
}

//____ MouseLeave() _________________________________________________________

WgEventFilter	WgEventFilter::MouseLeave()
{
	return WgEventFilter( WG_EVENT_MOUSE_LEAVE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::MouseLeave( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSE_LEAVE, pWidget, _filterTypeWidget );
}

//____ MouseMove() _________________________________________________________

WgEventFilter	WgEventFilter::MouseMove()
{
	return WgEventFilter( WG_EVENT_MOUSE_MOVE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::MouseMove( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSE_MOVE, pWidget, _filterTypeWidget );
}

//____ MouseButtonPress() _________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonPress()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_PRESS, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonPress( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_PRESS, 0, _filterMouseButtonEvents, button );
}

#ifdef SOFTUBE_USE_PACE_FUSION
PACE_FUSION_NO_USER_CALLBACK
#endif
WgEventFilter	WgEventFilter::MouseButtonPress( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_PRESS, pWidget, _filterMouseButtonEvents, 0 );
}

#ifdef SOFTUBE_USE_PACE_FUSION
PACE_FUSION_NO_USER_CALLBACK
#endif
WgEventFilter	WgEventFilter::MouseButtonPress( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_PRESS, pWidget, _filterMouseButtonEvents, button );
}

//____ MouseButtonRelease() ________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonRelease()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_RELEASE, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonRelease( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_RELEASE, 0, _filterMouseButtonEvents, button );
}

WgEventFilter	WgEventFilter::MouseButtonRelease( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_RELEASE, pWidget, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonRelease( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_RELEASE, pWidget, _filterMouseButtonEvents, button );
}

//____ MouseButtonDrag() ___________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonDrag()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DRAG, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonDrag( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DRAG, 0, _filterMouseButtonEvents, button );
}

WgEventFilter	WgEventFilter::MouseButtonDrag( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DRAG, pWidget, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonDrag( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DRAG, pWidget, _filterMouseButtonEvents, button );
}

//____ MouseButtonRepeat() ________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonRepeat()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_REPEAT, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonRepeat( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_REPEAT, 0, _filterMouseButtonEvents, button );
}

WgEventFilter	WgEventFilter::MouseButtonRepeat( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_REPEAT, pWidget, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonRepeat( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_REPEAT, pWidget, _filterMouseButtonEvents, button );
}

//____ MouseButtonClick() ________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonClick()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_CLICK, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonClick( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_CLICK, 0, _filterMouseButtonEvents, button );
}

WgEventFilter	WgEventFilter::MouseButtonClick( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_CLICK, pWidget, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonClick( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_CLICK, pWidget, _filterMouseButtonEvents, button );
}

//____ MouseButtonDoubleClick() ________________________________________________________

WgEventFilter	WgEventFilter::MouseButtonDoubleClick()
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK, 0, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonDoubleClick( int button)
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK, 0, _filterMouseButtonEvents, button );
}

WgEventFilter	WgEventFilter::MouseButtonDoubleClick( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK, pWidget, _filterMouseButtonEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseButtonDoubleClick( WgWidget * pWidget, int button )
{
	return WgEventFilter( WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK, pWidget, _filterMouseButtonEvents, button );
}

//____ KeyPress() _____________________________________________________________

WgEventFilter	WgEventFilter::KeyPress()
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, 0, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyPress( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, pWidget, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyPress( WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, 0, _filterKeyEvents, keycode );
}

WgEventFilter	WgEventFilter::KeyPress( WgWidget * pWidget, WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, pWidget, _filterKeyEvents, keycode );
}

//____ KeyRelease() ___________________________________________________________

WgEventFilter	WgEventFilter::KeyRelease()
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, 0, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyRelease( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, pWidget, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyRelease( WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, 0, _filterKeyEvents, keycode );
}

WgEventFilter	WgEventFilter::KeyRelease( WgWidget * pWidget, WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, pWidget, _filterKeyEvents, keycode );
}

//____ KeyRepeat() ____________________________________________________________

WgEventFilter	WgEventFilter::KeyRepeat()
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, 0, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyRepeat( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, pWidget, _filterKeyEvents, WG_KEY_UNMAPPED );
}

WgEventFilter	WgEventFilter::KeyRepeat( WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, 0, _filterKeyEvents, keycode );
}

WgEventFilter	WgEventFilter::KeyRepeat( WgWidget * pWidget, WgKey keycode )
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, pWidget, _filterKeyEvents, keycode );
}

//____ KeyPressNative() _______________________________________________________

WgEventFilter	WgEventFilter::KeyPressNative( int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, 0, _filterNativeKeyEvents, native_keycode );
}

WgEventFilter	WgEventFilter::KeyPressNative( WgWidget * pWidget, int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_PRESS, pWidget, _filterNativeKeyEvents, native_keycode );
}

//____ KeyReleaseNative() _____________________________________________________

WgEventFilter	WgEventFilter::KeyReleaseNative( int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, 0, _filterNativeKeyEvents, native_keycode );
}

WgEventFilter	WgEventFilter::KeyReleaseNative( WgWidget * pWidget, int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_RELEASE, pWidget, _filterNativeKeyEvents, native_keycode );
}

//____ KeyRepeatNative() ______________________________________________________

WgEventFilter	WgEventFilter::KeyRepeatNative( int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, 0, _filterNativeKeyEvents, native_keycode );
}

WgEventFilter	WgEventFilter::KeyRepeatNative( WgWidget * pWidget, int native_keycode )
{
	return WgEventFilter( WG_EVENT_KEY_REPEAT, pWidget, _filterNativeKeyEvents, native_keycode );
}

//____ Character() ____________________________________________________________

WgEventFilter	WgEventFilter::Character()
{
	return WgEventFilter( WG_EVENT_CHARACTER, 0, _filterCharacterEvents, 0 );
}

WgEventFilter	WgEventFilter::Character( unsigned short character)
{
	return WgEventFilter( WG_EVENT_CHARACTER, 0, _filterCharacterEvents, character );
}

WgEventFilter	WgEventFilter::Character( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_CHARACTER, pWidget, _filterCharacterEvents, 0 );
}

WgEventFilter	WgEventFilter::Character( WgWidget * pWidget, unsigned short character )
{
	return WgEventFilter( WG_EVENT_CHARACTER, pWidget, _filterCharacterEvents, character );
}

//____ MouseWheelRoll() ____________________________________________________________

WgEventFilter	WgEventFilter::MouseWheelRoll()
{
	return WgEventFilter( WG_EVENT_MOUSEWHEEL_ROLL, 0, _filterMouseWheelRollEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseWheelRoll( int wheel )
{
	return WgEventFilter( WG_EVENT_MOUSEWHEEL_ROLL, 0, _filterMouseWheelRollEvents, wheel );
}

WgEventFilter	WgEventFilter::MouseWheelRoll( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_MOUSEWHEEL_ROLL, pWidget, _filterMouseWheelRollEvents, 0 );
}

WgEventFilter	WgEventFilter::MouseWheelRoll( WgWidget * pWidget, int wheel )
{
	return WgEventFilter( WG_EVENT_MOUSEWHEEL_ROLL, pWidget, _filterMouseWheelRollEvents, wheel );
}

//____ ButtonPress() __________________________________________________________

WgEventFilter	WgEventFilter::ButtonPress()
{
	return WgEventFilter( WG_EVENT_BUTTON_PRESS, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::ButtonPress( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_BUTTON_PRESS, pWidget, _filterTypeWidget );
}

//____ CheckboxCheck() __________________________________________________________

WgEventFilter	WgEventFilter::CheckboxCheck()
{
	return WgEventFilter( WG_EVENT_CHECKBOX_CHECK, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::CheckboxCheck( WgCheckBox * pWidget )
{
	return WgEventFilter( WG_EVENT_CHECKBOX_CHECK, pWidget, _filterTypeWidget );
}

//____ CheckboxUncheck() __________________________________________________________

WgEventFilter	WgEventFilter::CheckboxUncheck()
{
	return WgEventFilter( WG_EVENT_CHECKBOX_UNCHECK, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::CheckboxUncheck( WgCheckBox * pWidget )
{
	return WgEventFilter( WG_EVENT_CHECKBOX_UNCHECK, pWidget, _filterTypeWidget );
}


//____ CheckboxToggle() __________________________________________________________

WgEventFilter	WgEventFilter::CheckboxToggle()
{
	return WgEventFilter( WG_EVENT_CHECKBOX_TOGGLE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::CheckboxToggle( WgCheckBox * pWidget )
{
	return WgEventFilter( WG_EVENT_CHECKBOX_TOGGLE, pWidget, _filterTypeWidget );
}

//____ RadiobuttonSelect() __________________________________________________________

WgEventFilter	WgEventFilter::RadiobuttonSelect()
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_SELECT, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::RadiobuttonSelect( WgRadioButton * pWidget )
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_SELECT, pWidget, _filterTypeWidget );
}

//____ RadiobuttonUnselect() __________________________________________________________

WgEventFilter	WgEventFilter::RadiobuttonUnselect()
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_UNSELECT, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::RadiobuttonUnselect( WgRadioButton * pWidget )
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_UNSELECT, pWidget, _filterTypeWidget );
}


//____ RadiobuttonToggle() __________________________________________________________

WgEventFilter	WgEventFilter::RadiobuttonToggle()
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_TOGGLE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::RadiobuttonToggle( WgRadioButton * pWidget )
{
	return WgEventFilter( WG_EVENT_RADIOBUTTON_TOGGLE, pWidget, _filterTypeWidget );
}

//____ AnimationUpdate() _______________________________________________________

WgEventFilter	WgEventFilter::AnimationUpdate()
{
	return WgEventFilter( WG_EVENT_ANIMATION_UPDATE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::AnimationUpdate( WgAnimPlayer * pWidget )
{
	return WgEventFilter( WG_EVENT_ANIMATION_UPDATE, pWidget, _filterTypeWidget );
}

//____ SliderMove() __________________________________________________________

WgEventFilter	WgEventFilter::SliderMove()
{
	return WgEventFilter( WG_EVENT_SLIDER_MOVE, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderMove( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_MOVE, pWidget, _filterTypeWidget );
}

//____ SliderStepFwd() __________________________________________________________

WgEventFilter	WgEventFilter::SliderStepFwd()
{
	return WgEventFilter( WG_EVENT_SLIDER_STEP_FWD, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderStepFwd( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_STEP_FWD, pWidget, _filterTypeWidget );
}

//____ SliderStepBwd() __________________________________________________________

WgEventFilter	WgEventFilter::SliderStepBwd()
{
	return WgEventFilter( WG_EVENT_SLIDER_STEP_BWD, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderStepBwd( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_STEP_BWD, pWidget, _filterTypeWidget );
}

//____ SliderJumpFwd() __________________________________________________________

WgEventFilter	WgEventFilter::SliderJumpFwd()
{
	return WgEventFilter( WG_EVENT_SLIDER_JUMP_FWD, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderJumpFwd( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_JUMP_FWD, pWidget, _filterTypeWidget );
}

//____ SliderJumpBwd() __________________________________________________________

WgEventFilter	WgEventFilter::SliderJumpBwd()
{
	return WgEventFilter( WG_EVENT_SLIDER_JUMP_BWD, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderJumpBwd( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_JUMP_BWD, pWidget, _filterTypeWidget );
}

//____ SliderWheelRoll() __________________________________________________________

WgEventFilter	WgEventFilter::SliderWheelRoll()
{
	return WgEventFilter( WG_EVENT_SLIDER_WHEEL_ROLL, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::SliderWheelRoll( WgWidgetSlider * pWidget )
{
	return WgEventFilter( WG_EVENT_SLIDER_WHEEL_ROLL, pWidget, _filterTypeWidget );
}

//____ TextModify() _______________________________________________________

WgEventFilter	WgEventFilter::TextModify()
{
	return WgEventFilter( WG_EVENT_TEXT_MODIFY, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::TextModify( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_TEXT_MODIFY, pWidget, _filterTypeWidget );
}

//____ TextSet() _______________________________________________________

WgEventFilter	WgEventFilter::TextSet()
{
	return WgEventFilter( WG_EVENT_TEXT_SET, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::TextSet( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_TEXT_SET, pWidget, _filterTypeWidget );
}

//____ KnobTurn() __________________________________________________________

WgEventFilter	WgEventFilter::KnobTurn()
{
	return WgEventFilter( WG_EVENT_KNOB_TURN, 0, _filterTypeWidget );
}

WgEventFilter	WgEventFilter::KnobTurn( WgWidget * pWidget )
{
	return WgEventFilter( WG_EVENT_KNOB_TURN, pWidget, _filterTypeWidget );
}

//____ MenuitemSelect() __________________________________________________________
/*
WgEventFilter	WgEventFilter::MenuitemSelect()
{
	return WgEventFilter( WG_EVENT_MENUITEM_SELECT, 0, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemSelect( int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_SELECT, 0, _filterMenuitemEvent, itemId );
}

WgEventFilter	WgEventFilter::MenuitemSelect( WgMenu * pWidget )
{
	return WgEventFilter( WG_EVENT_MENUITEM_SELECT, pWidget, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemSelect( WgMenu * pWidget, int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_SELECT, pWidget, _filterMenuitemEvent, itemId );
}

//____ MenuitemCheck() __________________________________________________________

WgEventFilter	WgEventFilter::MenuitemCheck()
{
	return WgEventFilter( WG_EVENT_MENUITEM_CHECK, 0, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemCheck( int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_CHECK, 0, _filterMenuitemEvent, itemId );
}

WgEventFilter	WgEventFilter::MenuitemCheck( WgMenu * pWidget )
{
	return WgEventFilter( WG_EVENT_MENUITEM_CHECK, pWidget, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemCheck( WgMenu * pWidget, int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_CHECK, pWidget, _filterMenuitemEvent, itemId );
}

//____ MenuitemUncheck() __________________________________________________________

WgEventFilter	WgEventFilter::MenuitemUncheck()
{
	return WgEventFilter( WG_EVENT_MENUITEM_UNCHECK, 0, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemUncheck( int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_UNCHECK, 0, _filterMenuitemEvent, itemId );
}

WgEventFilter	WgEventFilter::MenuitemUncheck( WgMenu * pWidget )
{
	return WgEventFilter( WG_EVENT_MENUITEM_UNCHECK, pWidget, _filterMenuitemEvent );
}

WgEventFilter	WgEventFilter::MenuitemUncheck( WgMenu * pWidget, int itemId )
{
	return WgEventFilter( WG_EVENT_MENUITEM_UNCHECK, pWidget, _filterMenuitemEvent, itemId );
}
*/

//_____________________________________________________________________________

bool WgEventFilter::_filterDummy( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	return true;
}

bool WgEventFilter::_filterType( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() )
		return true;

	return false;
}

bool WgEventFilter::_filterWidget(const WgEvent::Event * pEvent, const WgEventFilter& filter)
{
	if (pEvent->Widget() == filter.Widget())
		return true;

	return false;
}

bool WgEventFilter::_filterWidgetId(const WgEvent::Event * pEvent, const WgEventFilter& filter)
{
	if (pEvent->Widget() && pEvent->Widget()->Id() == filter.m_data1)
		return true;

	return false;
}


bool WgEventFilter::_filterTypeWidget( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
		return true;

	return false;
}

bool WgEventFilter::_filterTypeWidgetId(const WgEvent::Event * pEvent, const WgEventFilter& filter)
{
	if (pEvent->Type() == filter.EventType() && (pEvent->Widget() && pEvent->Widget()->Id() == filter.m_data1))
		return true;

	return false;
}


bool WgEventFilter::_filterMouseButtonEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		if( filter.m_data1 == 0 )
			return true;
		else
		{
			const WgEvent::MouseButtonEvent * p = static_cast<const WgEvent::MouseButtonEvent*>(pEvent);

			if( p->Button() == filter.m_data1 )
				return true;
		}
	}
	return false;
}

bool WgEventFilter::_filterKeyEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		if( filter.m_data1 == WG_KEY_UNMAPPED )
			return true;
		else
		{
			const WgEvent::KeyEvent * p = static_cast<const WgEvent::KeyEvent*>(pEvent);

			if( p->TranslatedKeyCode() == filter.m_data1 )
				return true;
		}
	}
	return false;
}

bool WgEventFilter::_filterNativeKeyEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		const WgEvent::KeyEvent * p = static_cast<const WgEvent::KeyEvent*>(pEvent);

		if( p->NativeKeyCode() == filter.m_data1 )
			return true;
	}
	return false;
}

bool WgEventFilter::_filterCharacterEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		int chr = (static_cast<const WgEvent::Character *>(pEvent))->Char();

		if( chr == filter.m_data1 )
			return true;
	}
	return false;
}

bool WgEventFilter::_filterMouseWheelRollEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		int chr = (static_cast<const WgEvent::MouseWheelRoll *>(pEvent))->Wheel();

		if( chr == filter.m_data1 )
			return true;
	}
	return false;
}



bool WgEventFilter::_filterMenuitemEvent( const WgEvent::Event * pEvent, const WgEventFilter& filter )
{
  /*
	if( pEvent->Type() == filter.EventType() && (!filter.Widget() || filter.Widget() == pEvent->Widget()) )
	{
		if( filter.m_data1 == 0 )
			return true;
		else
		{
			const WgEvent::MenuitemEvent * p = static_cast<const WgEvent::MenuitemEvent*>(pEvent);

			if( p->ItemId() == filter.m_data1 )
				return true;
		}
	}
  */
	return false;
}
