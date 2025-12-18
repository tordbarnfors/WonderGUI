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
#include <wg2_event.h>
#include <wg2_key.h>
#include <wg2_widget.h>
#include <wg2_button.h>
#include <wg2_checkbox.h>
#include <wg2_radiobutton.h>
#include <wg2_animplayer.h>
#include <wg2_slider.h>
#include <wg2_textdisplay.h>
#include <wg2_lineeditor.h>
#include <wg2_multislider.h>
#include <wg_dataset.h>

#include <cassert>

namespace WgEvent
{
	//____ Event ______________________________________________________________

	WgWidget * Event::Widget() const
	{
		return m_pWidget.GetRealPtr();
	}

	WgWidget * Event::ForwardedFrom() const
	{
		return m_pForwardedFrom.GetRealPtr();
	}


	bool Event::IsMouseEvent() const
	{
		if( m_type == WG_EVENT_MOUSEWHEEL_ROLL ||
			m_type == WG_EVENT_MOUSE_ENTER ||
			m_type == WG_EVENT_MOUSE_LEAVE ||
			m_type == WG_EVENT_MOUSE_MOVE ||
			m_type == WG_EVENT_MOUSE_POSITION ||
			IsMouseButtonEvent() )
			return true;
		else
			return false;

	}

	bool Event::IsMouseButtonEvent() const
	{
		if( m_type == WG_EVENT_MOUSEBUTTON_CLICK ||
			m_type == WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK ||
			m_type == WG_EVENT_MOUSEBUTTON_DRAG ||
			m_type == WG_EVENT_MOUSEBUTTON_PRESS ||
			m_type == WG_EVENT_MOUSEBUTTON_RELEASE ||
			m_type == WG_EVENT_MOUSEBUTTON_REPEAT )
			return true;
		else
			return false;
	}

	bool Event::IsKeyEvent() const
	{
		if( m_type == WG_EVENT_KEY_PRESS ||
			m_type == WG_EVENT_KEY_RELEASE ||
			m_type == WG_EVENT_KEY_REPEAT )
			return true;
		else
			return false;
	}

	void Event::_cloneContentFrom( const Event * pOrg )
	{
		m_type				= pOrg->m_type;
		m_modKeys			= pOrg->m_modKeys;
		m_timestamp			= pOrg->m_timestamp;
		m_bIsForWidget		= pOrg->m_bIsForWidget;
		m_pWidget			= pOrg->m_pWidget;
		m_pointerLocalPixelPos	= pOrg->m_pointerLocalPixelPos;
		m_pointerScreenPixelPos	= pOrg->m_pointerScreenPixelPos;
		m_pointScale        = pOrg->m_pointScale;
	}

	//____ MouseButtonEvent ____________________________________________________

	void MouseButtonEvent::_cloneContentFrom( const Event * _pOrg )
	{
		const MouseButtonEvent * pOrg = static_cast<const MouseButtonEvent*>(_pOrg);

		m_button = pOrg->m_button;
		Event::_cloneContentFrom( pOrg );
	}

	//____ KeyEvent ____________________________________________________________

	bool KeyEvent::IsCursorKey() const
	{
		if( m_translatedKeyCode == WG_KEY_UP || m_translatedKeyCode == WG_KEY_DOWN ||
			m_translatedKeyCode == WG_KEY_LEFT || m_translatedKeyCode == WG_KEY_RIGHT )
			return true;
		else
			return false;
	}

	bool KeyEvent::IsMovementKey() const
	{
		if( m_translatedKeyCode == WG_KEY_PAGE_UP || m_translatedKeyCode == WG_KEY_PAGE_DOWN ||
			m_translatedKeyCode == WG_KEY_HOME || m_translatedKeyCode == WG_KEY_END ||
			IsCursorKey() )
			return true;
		else
			return false;
	}

	void KeyEvent::_cloneContentFrom( const Event * _pOrg )
	{
		const KeyEvent * pOrg = static_cast<const KeyEvent*>(_pOrg);

		m_nativeKeyCode		= pOrg->m_nativeKeyCode;
		m_translatedKeyCode	= pOrg->m_translatedKeyCode;
		Event::_cloneContentFrom( pOrg );
	}

	//____ FocusGained ________________________________________________________

	FocusGained::FocusGained()
	{
		m_type = WG_EVENT_FOCUS_GAINED;
	}

	FocusGained::FocusGained(WgWidget * pWidget)
	{
		m_type = WG_EVENT_FOCUS_GAINED;
		m_bIsForWidget = true;
		m_pWidget = pWidget;
	}

	//____ FocusLost __________________________________________________________

	FocusLost::FocusLost()
	{
		m_type = WG_EVENT_FOCUS_LOST;
	}

	FocusLost::FocusLost(WgWidget * pWidget)
	{
		m_type = WG_EVENT_FOCUS_LOST;
		m_bIsForWidget = true;
		m_pWidget = pWidget;
	}

	//____ MouseEnter _______________________________________________________

	MouseEnter::MouseEnter( const WgCoord& pos )
	{
		m_type = WG_EVENT_MOUSE_ENTER;

		m_pointerLocalPixelPos = pos;
		m_pointerScreenPixelPos = pos;
	}

	MouseEnter::MouseEnter( WgWidget * pWidget )
	{
		m_type = WG_EVENT_MOUSE_ENTER;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MouseLeave ________________________________________________________

	MouseLeave::MouseLeave()
	{
		m_type = WG_EVENT_MOUSE_LEAVE;
	}

	MouseLeave::MouseLeave( WgWidget * pWidget )
	{
		m_type = WG_EVENT_MOUSE_LEAVE;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MouseMove __________________________________________________________

	MouseMove::MouseMove( const WgCoord& pos )
	{
		m_type = WG_EVENT_MOUSE_MOVE;

		m_pointerLocalPixelPos = pos;
		m_pointerScreenPixelPos = pos;
	}

	MouseMove::MouseMove( WgWidget * pWidget )
	{
		m_type = WG_EVENT_MOUSE_MOVE;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MouseButtonPress ______________________________________________________

	MouseButtonPress::MouseButtonPress( int button ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_PRESS;
	}

	MouseButtonPress::MouseButtonPress( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type			= WG_EVENT_MOUSEBUTTON_PRESS;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MouseButtonRepeat ______________________________________________________

	MouseButtonRepeat::MouseButtonRepeat( int button ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_REPEAT;
	}

	MouseButtonRepeat::MouseButtonRepeat( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type			= WG_EVENT_MOUSEBUTTON_REPEAT;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MouseButtonRelease _____________________________________________________

	MouseButtonRelease::MouseButtonRelease( int button ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_RELEASE;

		m_bPressInside = true;			// Always assumed to have had the press inside our window.
		m_bReleaseInside = true;		// Always assumed to be inside our window now.
		m_pressModKeys = WG_MODKEY_NONE;
	}

	MouseButtonRelease::MouseButtonRelease( int button, WgWidget * pWidget, bool bPressInside, bool bReleaseInside ) : MouseButtonEvent(button)
	{
		m_type			= WG_EVENT_MOUSEBUTTON_RELEASE;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;

		m_bPressInside = bPressInside;
		m_bReleaseInside = bReleaseInside;
	}

	void MouseButtonRelease::_cloneContentFrom( const Event * _pOrg )
	{
		const MouseButtonRelease * pOrg = static_cast<const MouseButtonRelease*>(_pOrg);

		m_bPressInside		= pOrg->m_bPressInside;
		m_bReleaseInside	= pOrg->m_bReleaseInside;
		m_pressModKeys      = pOrg->m_pressModKeys;
		MouseButtonEvent::_cloneContentFrom( pOrg );
	}


	bool MouseButtonRelease::PressInside() const
	{
		return m_bPressInside;
	}

	bool MouseButtonRelease::ReleaseInside() const
	{
		return m_bReleaseInside;
	}



	//____ MouseButtonClick _____________________________________________________

	MouseButtonClick::MouseButtonClick( int button ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_CLICK;
	}

	MouseButtonClick::MouseButtonClick( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_CLICK;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}


	//____ MouseButtonDoubleClick _____________________________________________________

	MouseButtonDoubleClick::MouseButtonDoubleClick( int button ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK;
	}

	MouseButtonDoubleClick::MouseButtonDoubleClick( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ KeyPress ___________________________________________________________

	KeyPress::KeyPress( int native_keycode ) : KeyEvent(native_keycode)
	{
		m_type 			= WG_EVENT_KEY_PRESS;
	}

	KeyPress::KeyPress( int native_keycode, WgWidget * pWidget ) : KeyEvent(native_keycode)
	{
		m_type 			= WG_EVENT_KEY_PRESS;
		m_bIsForWidget	= true;
		m_pWidget		= pWidget;
	}

	//____ KeyRepeat ___________________________________________________________

	KeyRepeat::KeyRepeat( int native_keycode ) : KeyEvent(native_keycode)
	{
		m_type 			= WG_EVENT_KEY_REPEAT;
	}

	KeyRepeat::KeyRepeat( int native_keycode, WgWidget * pWidget ) : KeyEvent(native_keycode)
	{
		m_type 			= WG_EVENT_KEY_REPEAT;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ KeyRelease ________________________________________________________

	KeyRelease::KeyRelease( int native_keycode ) : KeyEvent(native_keycode)
	{
		m_type 		= WG_EVENT_KEY_RELEASE;
	}

	KeyRelease::KeyRelease( int native_keycode, WgWidget * pWidget ) : KeyEvent(native_keycode)
	{
		m_type 			= WG_EVENT_KEY_RELEASE;
		m_bIsForWidget	= true;
		m_pWidget		= pWidget;
	}

	//____ Character __________________________________________________________

	Character::Character( unsigned short character )
	{
		m_type = WG_EVENT_CHARACTER;
		m_char = character;
	}

	Character::Character( unsigned short character, WgWidget * pWidget )
	{
		m_type			= WG_EVENT_CHARACTER;
		m_char			= character;
		m_bIsForWidget	= true;
		m_pWidget		= pWidget;
	}

	void Character::_cloneContentFrom( const Event * _pOrg )
	{
		const Character * pOrg = static_cast<const Character*>(_pOrg);

		m_char			= pOrg->m_char;
		Event::_cloneContentFrom( pOrg );
	}

	unsigned short Character::Char() const
	{
		return m_char;
	}

	//____ MouseWheelRoll __________________________________________________________

	MouseWheelRoll::MouseWheelRoll( int wheel, int distance, bool bInvertScroll )
	{
		m_type              = WG_EVENT_MOUSEWHEEL_ROLL;
		m_wheel             = wheel;
		m_distance          = distance;
        m_bInvertScroll     = bInvertScroll;
	}

	MouseWheelRoll::MouseWheelRoll( int wheel, int distance, bool bInvertScroll, WgWidget * pWidget )
	{
		m_type			    = WG_EVENT_MOUSEWHEEL_ROLL;
		m_wheel			    = wheel;
		m_distance		    = distance;
        m_bInvertScroll     = bInvertScroll;
		m_bIsForWidget	    = true;
		m_pWidget		    = pWidget;
	}

	void MouseWheelRoll::_cloneContentFrom( const Event * _pOrg )
	{
		const MouseWheelRoll * pOrg = static_cast<const MouseWheelRoll*>(_pOrg);

		m_wheel			    = pOrg->m_wheel;
		m_distance		    = pOrg->m_distance;
        m_bInvertScroll     = pOrg->m_bInvertScroll;
		Event::_cloneContentFrom( pOrg );
	}


	int MouseWheelRoll::Wheel() const
	{
		return m_wheel;
	}

	int MouseWheelRoll::Distance() const
	{
		return m_distance;
	}

    bool MouseWheelRoll::InvertScroll() const
    {
        return m_bInvertScroll;
    }


	//____ Tick _______________________________________________________________

	Tick::Tick( int millisec, int microsec  )
	{
		m_type = WG_EVENT_TICK;

		if( microsec >= 0 )
			m_microsec = microsec;
		else
			m_microsec = millisec*1000;
	}

	Tick::Tick( int microsec, WgWidget * pWidget, bool b )
	{
		m_type 			= WG_EVENT_TICK;
		m_microsec 		= microsec;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	void Tick::_cloneContentFrom( const Event * _pOrg )
	{
		const Tick * pOrg = static_cast<const Tick*>(_pOrg);

		m_microsec = pOrg->m_microsec;
		Event::_cloneContentFrom( pOrg );
	}

	int Tick::Millisec() const
	{
		return m_microsec/1000;
	}

	int Tick::Microsec() const
	{
		return m_microsec;
	}

	//____ PointerChange _______________________________________________________________

	PointerChange::PointerChange( WgPointerStyle style )
	{
		m_type = WG_EVENT_POINTER_CHANGE;
		m_style = style;
	}

	void PointerChange::_cloneContentFrom( const Event * _pOrg )
	{
		const PointerChange * pOrg = static_cast<const PointerChange*>(_pOrg);

		m_style = pOrg->m_style;
		Event::_cloneContentFrom( pOrg );
	}

	WgPointerStyle PointerChange::Style() const
	{
		return m_style;
	}

	//____ Eavesdrop _______________________________________________________________

	Eavesdrop::Eavesdrop( WgWidget * pEavesdropper, WgEvent::Event * pEvent )
	{
		m_type             = WG_EVENT_EAVESDROP;
		m_pEvent           = pEvent;
		m_bIsForWidget    = true;
		m_pWidget         = pEavesdropper;
	}

	Eavesdrop::~Eavesdrop()
	{
		delete m_pEvent;
	}

	WgEvent::Event * Eavesdrop::Event() const
	{
		return m_pEvent;
	}

	void Eavesdrop::_cloneContentFrom( const WgEvent::Event * _pOrg )
	{
		assert(false);              // Not allowed to clone! Needs a deep cloning of pointed to widget.

		const Tick * pOrg = static_cast<const Tick*>(_pOrg);

		Event::_cloneContentFrom( pOrg );
	}


	//____ DragNDrop _______________________________________________________________

	DragNDrop::DragNDrop( WgEventType type, WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver )
	{
		m_type = type;
		m_pWidget = pSource;
		m_bIsForWidget = true;
		m_pickCategory = pickCategory;
		m_pPayload = pPayload;
		m_pPickedFrom = pPickedFrom;
		m_pFinalRecipient = pFinalReceiver;
	}

	void DragNDrop::_cloneContentFrom( const Event * _pOrg )
	{
		const DragNDrop * pOrg = static_cast<const DragNDrop*>(_pOrg);

		m_pPayload = pOrg->m_pPayload;
		m_pPickedFrom = pOrg->m_pPickedFrom;
		m_pickCategory = pOrg->m_pickCategory;
	}

	DropPick::DropPick( WgWidget * pSource, WgCoord pickOfs, WgWidget * pFinalReceiver )
	: DragNDrop( WG_EVENT_DROP_PICK, pSource, 0, nullptr, pSource, pFinalReceiver ),
	m_dragWidgetPointerOfs(WgCoord()-pickOfs),
	m_pickOfs(pickOfs),
	m_bDeleteWhenDone(true),
	m_pDragWidget(nullptr)
	{
	}

	void DropPick::setPayload( wg::BasicDataset * pPayload )
	{
		m_pPayload = pPayload;
	}

	void DropPick::setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone )
	{
		m_pDragWidget = pWidget;
        m_dragWidgetPointerOfs = pixelPointerOfs;
		m_bDeleteWhenDone = bDeleteWhenDone;
	}

	DropProbe::DropProbe( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver )
	: DragNDrop( WG_EVENT_DROP_PROBE, pSource, pickCategory, pPayload, pPickedFrom, pFinalReceiver ),
	m_bAccepted(false)
	{
	}

	DropEnter::DropEnter( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pDragWidget, WgCoord dragWidgetPointerOfs, bool bDeleteWhenDone, WgWidget * pFinalReceiver )
	: DragNDrop( WG_EVENT_DROP_ENTER, pSource, pickCategory, pPayload, pPickedFrom, pFinalReceiver ),
	m_pDragWidget(pDragWidget),
    m_dragWidgetPointerOfs(dragWidgetPointerOfs),
    m_bDeleteWhenDone(bDeleteWhenDone)
	{
	}

	void DropEnter::setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone )
	{
		m_pDragWidget = pWidget;
        m_dragWidgetPointerOfs = pixelPointerOfs;
        m_bDeleteWhenDone = bDeleteWhenDone;
	}

	DropMove::DropMove( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pDragWidget, WgCoord dragWidgetPointerOfs, bool bDeleteWhenDone, WgWidget * pFinalReceiver )
	: DragNDrop( WG_EVENT_DROP_MOVE, pSource, pickCategory, pPayload, pPickedFrom, pFinalReceiver ),
	m_pDragWidget(pDragWidget),
    m_dragWidgetPointerOfs(dragWidgetPointerOfs),
    m_bDeleteWhenDone(bDeleteWhenDone)
	{
	}

	void DropMove::setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone )
	{
		m_pDragWidget = pWidget;
        m_dragWidgetPointerOfs = pixelPointerOfs;
        m_bDeleteWhenDone = bDeleteWhenDone;
	}


	DropLeave::DropLeave( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom )
	: DragNDrop( WG_EVENT_DROP_LEAVE, pSource, pickCategory, pPayload, pPickedFrom, nullptr )
	{
	}

	DropDeliver::DropDeliver( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver )
	: DragNDrop( WG_EVENT_DROP_DELIVER, pSource, pickCategory, pPayload, pPickedFrom, pFinalReceiver ),
	m_bAccepted(false)
	{
	}

	WgWidget * DropDeliver::deliveredTo() const
	{
		return m_pWidget.GetRealPtr();
	}

    DropCancel::DropCancel(WgWidget *pPickedFrom, int pickCategory, wg::BasicDataset *pPayload, WgWidgetWeakPtr pDropped)
        : DragNDrop(WG_EVENT_DROP_CANCEL, pPickedFrom, pickCategory, pPayload, pPickedFrom, nullptr), m_pDropped{pDropped}
    {
	}

	DropComplete::DropComplete( WgWidget * pPicked, WgWidget * pDeliveree, int pickCategory, wg::BasicDataset * pPayload )
	: DragNDrop( WG_EVENT_DROP_COMPLETE, pPicked, pickCategory, pPayload, pPicked, nullptr ),
	m_pDeliveree(pDeliveree)
	{
	}


	DropHoverEnter::DropHoverEnter( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom )
	: DragNDrop( WG_EVENT_DROPHOVER_ENTER, pSource, pickCategory, pPayload, pPickedFrom, nullptr )
	{
	}

	DropHoverMove::DropHoverMove( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom )
	: DragNDrop( WG_EVENT_DROPHOVER_MOVE, pSource, pickCategory, pPayload, pPickedFrom, nullptr )
	{
	}

	DropHoverLeave::DropHoverLeave( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom )
	: DragNDrop( WG_EVENT_DROPHOVER_LEAVE, pSource, pickCategory, pPayload, pPickedFrom, nullptr )
	{
	}



	//____ ButtonPress _________________________________________________________

	ButtonPress::ButtonPress( WgButton * pWidget )
	{
		m_type 			= WG_EVENT_BUTTON_PRESS;
		m_pWidget 		= pWidget;
	}

	WgButton * ButtonPress::Button() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return static_cast<WgButton*>(pWidget);
		else
			return 0;
	}

	//____ Selected _________________________________________________________

	Selected::Selected(WgWidget * pWidget)
	{
		m_type = WG_EVENT_SELECTED;
		m_pWidget = pWidget;
	}



	//____ Checkbox event methods ______________________________________________

	WgCheckBox * CheckboxEvent::Checkbox() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return static_cast<WgCheckBox*>(pWidget);
		else
			return 0;
	}

	CheckboxCheck::CheckboxCheck( WgCheckBox * pWidget )
	{
		m_type = WG_EVENT_CHECKBOX_CHECK;
		m_pWidget = pWidget;
	}

	CheckboxUncheck::CheckboxUncheck( WgCheckBox * pWidget )
	{
		m_type = WG_EVENT_CHECKBOX_UNCHECK;
		m_pWidget = pWidget;
	}

	CheckboxToggle::CheckboxToggle( WgCheckBox * pWidget, bool bChecked )
	{
		m_type = WG_EVENT_CHECKBOX_TOGGLE;
		m_pWidget = pWidget;
		m_bChecked = bChecked;
	}

	bool CheckboxToggle::IsChecked() const
	{
		return m_bChecked;
	}

	//____ Radiobutton event methods ___________________________________________

	WgRadioButton * RadiobuttonEvent::Radiobutton() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return static_cast<WgRadioButton*>(pWidget);
		else
			return 0;
	}

	RadiobuttonSelect::RadiobuttonSelect( WgRadioButton * pWidget )
	{
		m_type = WG_EVENT_RADIOBUTTON_SELECT;
		m_pWidget = pWidget;
	}

	RadiobuttonUnselect::RadiobuttonUnselect( WgRadioButton * pWidget )
	{
		m_type = WG_EVENT_RADIOBUTTON_UNSELECT;
		m_pWidget = pWidget;
	}

	RadiobuttonToggle::RadiobuttonToggle( WgRadioButton * pWidget, bool bSelected )
	{
		m_type = WG_EVENT_RADIOBUTTON_TOGGLE;
		m_pWidget = pWidget;
		m_bSelected = bSelected;
	}

	bool RadiobuttonToggle::IsSelected() const
	{
		return m_bSelected;
	}

	//____ Animation event methods _____________________________________________

	AnimationUpdate::AnimationUpdate( WgAnimPlayer * pWidget, int frame, float fraction )
	{
		m_type = WG_EVENT_ANIMATION_UPDATE;
		m_pWidget = pWidget;
		m_frame = frame;
		m_fraction = fraction;
	}

	WgAnimPlayer * AnimationUpdate::Animation() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return static_cast<WgAnimPlayer*>(pWidget);
		else
			return 0;
	}

	int AnimationUpdate::Frame() const
	{
		return m_frame;
	}

	float AnimationUpdate::Fraction() const
	{
		return m_fraction;
	}

	//____ Slider event methods _______________________________________________

	SliderEvent::SliderEvent( WgWidgetSlider * pWidget, float pos, float length )
	{
		m_pWidget 	= pWidget;
		m_pos 		= pos;
		m_length 	= length;
	}

	WgWidgetSlider* SliderEvent::Slider() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return static_cast<WgWidgetSlider*>(pWidget);
		else
			return 0;
	}

	float SliderEvent::Pos() const
	{
		return m_pos;
	}

	float SliderEvent::Length() const
	{
		return m_length;
	}

	float SliderEvent::Value() const
	{
		float max = 1.f - m_length;

		if( max == 0.f )
			return 0.5f;		// is this logically correct?
		else
			return m_pos / max;
	}

	SliderMove::SliderMove( WgWidgetSlider* pWidget, float pos, float length ) : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_MOVE;
	}

	SliderStepFwd::SliderStepFwd( WgWidgetSlider* pWidget, float pos, float length ) : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_STEP_FWD;
	}

	SliderStepBwd::SliderStepBwd( WgWidgetSlider* pWidget, float pos, float length ) : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_STEP_BWD;
	}

	SliderJumpFwd::SliderJumpFwd( WgWidgetSlider* pWidget, float pos, float length ) : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_JUMP_FWD;
	}

	SliderJumpBwd::SliderJumpBwd( WgWidgetSlider* pWidget, float pos, float length ) : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_JUMP_BWD;
	}

	SliderWheelRolled::SliderWheelRolled( WgWidgetSlider* pWidget, int distance, float pos, float length )  : SliderEvent( pWidget, pos, length )
	{
		m_type = WG_EVENT_SLIDER_WHEEL_ROLL;
		m_distance = distance;
	}

	int SliderWheelRolled::Distance() const
	{
		return m_distance;
	}

	//____ Text event methods __________________________________________________

	WgInterfaceEditText * TextEvent::Interface() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
		{
			if( pWidget->Type() == WgTextDisplay::GetClass() )
				return static_cast<WgTextDisplay*>(pWidget);
			else if( pWidget->Type() == WgLineEditor::GetClass() )
				return static_cast<WgLineEditor*>(pWidget);
		}
		return 0;
	}

	const WgText * TextEvent::Text() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget )
			return m_pText;
		else
			return 0;
	}

	TextModify::TextModify( WgWidget * pWidget, WgText * pText )
	{
		m_type 		= WG_EVENT_TEXT_MODIFY;
		m_pWidget 	= pWidget;
		m_pText 	= pText;
	}

	TextSet::TextSet( WgWidget * pWidget, WgText * pText )
	{
		m_type 		= WG_EVENT_TEXT_SET;
		m_pWidget 	= pWidget;
		m_pText 	= pText;
	}

	//____ Menu event methods __________________________________________________
  // Menu is not used by Softube. Remove this code to avoid having to bring in a lot of source files just related to menus
  /*
	WgMenu *	MenuitemEvent::Menu() const
	{
		WgWidget * pWidget = m_pWidget.GetRealPtr();
		if( pWidget && pWidget->Type() == WgMenu::GetClass() )
			return static_cast<WgMenu*>(pWidget);

		return 0;
	}

	int MenuitemEvent::ItemId() const
	{
		return m_itemId;
	}

	MenuitemSelect::MenuitemSelect( WgMenu * pMenu, int menuItemId )
	{
		m_type		= WG_EVENT_MENUITEM_SELECT;
		m_pWidget	= pMenu;
		m_itemId	= menuItemId;
	}

	MenuitemCheck::MenuitemCheck( WgMenu * pMenu, int menuItemId )
	{
		m_type		= WG_EVENT_MENUITEM_CHECK;
		m_pWidget	= pMenu;
		m_itemId	= menuItemId;
	}

	MenuitemUncheck::MenuitemUncheck( WgMenu * pMenu, int menuItemId )
	{
		m_type		= WG_EVENT_MENUITEM_UNCHECK;
		m_pWidget	= pMenu;
		m_itemId	= menuItemId;
	}
	*/

	//____ PopupLayer event methods ____________________________________________

	PopupClosed::PopupClosed( WgWidget * pPopup, const WgWidgetWeakPtr& pCaller )
	{
		m_type		= WG_EVENT_POPUP_CLOSED;
		m_pWidget	= pCaller;
		m_bIsForWidget	= true;
		m_pPopup	= pPopup;
	}

	WgWidget * PopupClosed::Popup() const
	{
		return m_pPopup.GetRealPtr();
	}

	WgWidget * PopupClosed::Caller() const
	{
		return m_pWidget.GetRealPtr();
	}

	//____ Modal event methods ___________________________________________________

	ModalMoveOutside::ModalMoveOutside( WgWidget * pWidget )
	{
		m_type = WG_EVENT_MODAL_MOVE_OUTSIDE;
		m_bIsForWidget	= true;
		m_pWidget		= pWidget;
	}

	ModalBlockedPress::ModalBlockedPress( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type			= WG_EVENT_MODAL_BLOCKED_PRESS;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	ModalBlockedRelease::ModalBlockedRelease( int button, WgWidget * pWidget ) : MouseButtonEvent(button)
	{
		m_type			= WG_EVENT_MODAL_BLOCKED_RELEASE;
		m_bIsForWidget	= true;
		m_pWidget 		= pWidget;
	}

	//____ MultiSlider event methods __________________________________________

	SliderEnter::SliderEnter(WgMultiSlider * pSlider, int sliderId)
	{
		m_type        = WG_EVENT_SLIDER_ENTER;
		m_pWidget    = pSlider;
		m_id        = sliderId;
	}

	SliderLeave::SliderLeave(WgMultiSlider * pSlider, int sliderId)
	{
		m_type        = WG_EVENT_SLIDER_LEAVE;
		m_pWidget    = pSlider;
		m_id        = sliderId;
	}

	SliderHandleEnter::SliderHandleEnter(WgMultiSlider * pSlider, int sliderId)
	{
		m_type        = WG_EVENT_SLIDER_HANDLE_ENTER;
		m_pWidget    = pSlider;
		m_id        = sliderId;
	}

	SliderHandleLeave::SliderHandleLeave(WgMultiSlider * pSlider, int sliderId)
	{
		m_type        = WG_EVENT_SLIDER_HANDLE_LEAVE;
		m_pWidget    = pSlider;
		m_id        = sliderId;
	}

	SliderPressed::SliderPressed(WgMultiSlider * pSlider, int sliderId, int button, WgOrigo offsetFromHandle)
	{
		m_type        = WG_EVENT_SLIDER_PRESS;
		m_pWidget    = pSlider;
		m_id        = sliderId;
		m_button    = button;
		m_offsetFromHandle = offsetFromHandle;
	}

	SliderMoved::SliderMoved(WgMultiSlider * pSlider, int sliderId, float value, float value2)
	{
		m_type		= WG_EVENT_SLIDER_MOVED;
		m_pWidget	= pSlider;
		m_id		= sliderId;
		m_value		= value;
		m_value2	= value2;
	}



	//____ Link event methods __________________________________________________

	std::string LinkEvent::Link() const
	{
		return m_link;
	}

	LinkMark::LinkMark( WgWidget * pWidget, std::string link )
	{
		m_type			= WG_EVENT_LINK_MARK;
		m_pWidget		= pWidget;
		m_link			= link;
	}

	LinkUnmark::LinkUnmark( WgWidget * pWidget, std::string link )
	{
		m_type			= WG_EVENT_LINK_UNMARK;
		m_pWidget		= pWidget;
		m_link			= link;
	}

	int LinkButtonEvent::Button() const
	{
		return m_button;
	}

	LinkPress::LinkPress( WgWidget * pWidget, std::string link, int button )
	{
		m_type			= WG_EVENT_LINK_PRESS;
		m_pWidget		= pWidget;
		m_link			= link;
		m_button		= button;
	}

	LinkRepeat::LinkRepeat( WgWidget * pWidget, std::string link, int button )
	{
		m_type			= WG_EVENT_LINK_REPEAT;
		m_pWidget		= pWidget;
		m_link			= link;
		m_button		= button;
	}

	LinkRelease::LinkRelease( WgWidget * pWidget, std::string link, int button )
	{
		m_type			= WG_EVENT_LINK_RELEASE;
		m_pWidget		= pWidget;
		m_link			= link;
		m_button		= button;
	}

	LinkClick::LinkClick( WgWidget * pWidget, std::string link, int button )
	{
		m_type			= WG_EVENT_LINK_CLICK;
		m_pWidget		= pWidget;
		m_link			= link;
		m_button		= button;
	}

	LinkDoubleClick::LinkDoubleClick( WgWidget * pWidget, std::string link, int button )
	{
		m_type			= WG_EVENT_LINK_DOUBLE_CLICK;
		m_pWidget		= pWidget;
		m_link			= link;
		m_button		= button;
	}

	//____ PianoKey event methods ________________________________________________

	PianoKeyPress::PianoKeyPress(WgWidget * pSource, int keyIdx, int64_t timestamp)
	{
		m_type			= WG_EVENT_PIANOKEY_PRESS;
		m_pWidget		= pSource;
		m_keyIdx		= keyIdx;
		m_timestamp		= timestamp;
	}

	PianoKeyRelease::PianoKeyRelease(WgWidget * pSource, int keyIdx, int64_t timestamp)
	{
		m_type			= WG_EVENT_PIANOKEY_RELEASE;
		m_pWidget		= pSource;
		m_keyIdx		= keyIdx;
		m_timestamp		= timestamp;
	}


	//____ Knob event methods ____________________________________________________

	KnobTurn::KnobTurn( WgWidget * pWidget, int pos, float fraction )
	{
		m_type		= WG_EVENT_KNOB_TURN;
		m_pWidget	= pWidget;
		m_pos		= pos;
		m_fraction	= fraction;
	}

	float KnobTurn::Fraction() const
	{
		return m_fraction;
	}

	int KnobTurn::Pos() const
	{
		return m_pos;
	}

	//____ WgRangeSlider events __________________________________________________

	RangeChanged::RangeChanged(WgWidget* pWidget, float begin, float end)
	{
		m_type = WG_EVENT_RANGE_CHANGED;
		m_pWidget = pWidget;
		m_begin = begin;
		m_end = end;
	}

	float RangeChanged::Begin() const
	{
		return m_begin;
	}

	float RangeChanged::End() const
	{
		return m_end;
	}


	//____ MousePosition _______________________________________________________

	MousePosition::MousePosition()
	{
		m_type = WG_EVENT_MOUSE_POSITION;
	}

	//____ MouseButtonDrag _________________________________________________________

	MouseButtonDrag::MouseButtonDrag( int button, const WgCoord& startPos, const WgCoord& prevPos, const WgCoord& currPos ) : MouseButtonEvent(button)
	{

		m_type = WG_EVENT_MOUSEBUTTON_DRAG;

		m_startPos = startPos;
		m_prevPos = prevPos;
		m_currPos = currPos;
	}

	MouseButtonDrag::MouseButtonDrag( int button, WgWidget * pWidget, const WgCoord& startPos, const WgCoord& prevPos, const WgCoord& currPos ) : MouseButtonEvent(button)
	{
		m_type = WG_EVENT_MOUSEBUTTON_DRAG;
		m_bIsForWidget		= true;
		m_pWidget 			= pWidget;

		m_startPos = startPos;
		m_prevPos = prevPos;
		m_currPos = currPos;
	}

	void MouseButtonDrag::_cloneContentFrom( const Event * _pOrg )
	{
		const MouseButtonDrag * pOrg = static_cast<const MouseButtonDrag*>(_pOrg);

		m_startPos = pOrg->m_startPos;
		m_prevPos = pOrg->m_prevPos;
		m_currPos = pOrg->m_currPos;
		MouseButtonEvent::_cloneContentFrom( pOrg );
	}


	WgCoord MouseButtonDrag::DraggedTotalPixels() const
	{
		return m_currPos - m_startPos;
	}

	WgCoord MouseButtonDrag::DraggedNowPixels() const
	{
		return m_currPos - m_prevPos;
	}

	WgCoord MouseButtonDrag::CurrPixelPos() const
	{
		return m_currPos;
	}

	WgCoord MouseButtonDrag::StartPixelPos() const
	{
		return m_startPos;
	}

	WgCoord MouseButtonDrag::PrevPixelPos() const
	{
		return m_prevPos;
	}


	WgCoord MouseButtonDrag::DraggedTotalPoints() const
	{
		return CurrPointPos() - StartPointPos();
	}

	WgCoord MouseButtonDrag::DraggedNowPoints() const
	{
		return CurrPointPos() - PrevPointPos();
	}

	WgCoord MouseButtonDrag::CurrPointPos() const
	{
		return (m_currPos*WG_SCALE_BASE)/m_pointScale;
	}

	WgCoord MouseButtonDrag::StartPointPos() const
	{
		return (m_startPos*WG_SCALE_BASE)/m_pointScale;
	}

	WgCoord MouseButtonDrag::PrevPointPos() const
	{
		return (m_prevPos*WG_SCALE_BASE)/m_pointScale;
	}


};
