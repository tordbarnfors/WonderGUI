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
#ifndef WG_EVENTFILTER_DOT_H
#define WG_EVENTFILTER_DOT_H

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_SMARTPTR_DOT_H
#	include <wg2_smartptr.h>
#endif

#ifndef WG_KEY2_DOT_H
#	include <wg2_key.h>
#endif

namespace WgEvent
{
	class Event;
};

class WgWidget;
class WgEventFilter;

typedef class WgWeakPtr<WgWidget> WgWidgetWeakPtr;

typedef	bool(*WgFilterFuncPtr)(const WgEvent::Event *, const WgEventFilter& );


class WgEventFilter
{
	friend class WgEventHandler;

public:
	WgEventFilter() : m_eventType(WG_EVENT_DUMMY), m_pFilterFunc(_filterDummy) {}	//Not meant to be public, but has to for compatibility with older compilers.

	static WgEventFilter	Type(int type);
	static WgEventFilter	Source(WgWidget * pWidget);
	static WgEventFilter	WidgetId(int id);
	static WgEventFilter	TypeAndSource(int type, WgWidget * pWidget);
	static WgEventFilter	TypeAndWidgetId(int type, int id);


	static WgEventFilter	Tick();
	static WgEventFilter	PointerChange();

	static WgEventFilter	FocusGained();
	static WgEventFilter	FocusGained( WgWidget * pWidget );
	static WgEventFilter	FocusLost();
	static WgEventFilter	FocusLost(WgWidget * pWidget);

	static WgEventFilter	MouseEnter();
	static WgEventFilter	MouseEnter( WgWidget * pWidget );

	static WgEventFilter	MouseLeave();
	static WgEventFilter	MouseLeave( WgWidget * pWidget );

	static WgEventFilter	MouseMove();
	static WgEventFilter	MouseMove( WgWidget * pWidget );

	static WgEventFilter	MouseButtonPress();
	static WgEventFilter	MouseButtonPress( int button);
	static WgEventFilter	MouseButtonPress( WgWidget * pWidget );
	static WgEventFilter	MouseButtonPress( WgWidget * pWidget, int button );

	static WgEventFilter	MouseButtonRelease();
	static WgEventFilter	MouseButtonRelease( int button);
	static WgEventFilter	MouseButtonRelease( WgWidget * pWidget );
	static WgEventFilter	MouseButtonRelease( WgWidget * pWidget, int button );

	static WgEventFilter	MouseButtonDrag();
	static WgEventFilter	MouseButtonDrag( int button);
	static WgEventFilter	MouseButtonDrag( WgWidget * pWidget );
	static WgEventFilter	MouseButtonDrag( WgWidget * pWidget, int button );

	static WgEventFilter	MouseButtonRepeat();
	static WgEventFilter	MouseButtonRepeat( int button);
	static WgEventFilter	MouseButtonRepeat( WgWidget * pWidget );
	static WgEventFilter	MouseButtonRepeat( WgWidget * pWidget, int button );

	static WgEventFilter	MouseButtonClick();
	static WgEventFilter	MouseButtonClick( int button);
	static WgEventFilter	MouseButtonClick( WgWidget * pWidget );
	static WgEventFilter	MouseButtonClick( WgWidget * pWidget, int button );

	static WgEventFilter	MouseButtonDoubleClick();
	static WgEventFilter	MouseButtonDoubleClick( int button);
	static WgEventFilter	MouseButtonDoubleClick( WgWidget * pWidget );
	static WgEventFilter	MouseButtonDoubleClick( WgWidget * pWidget, int button );

	static WgEventFilter	KeyPress();
	static WgEventFilter	KeyPress( WgWidget * pWidget );
	static WgEventFilter	KeyPress( WgKey translated_keycode );
	static WgEventFilter	KeyPress( WgWidget * pWidget, WgKey keycode );

	static WgEventFilter	KeyRelease();
	static WgEventFilter	KeyRelease( WgWidget * pWidget );
	static WgEventFilter	KeyRelease( WgKey translated_keycode );
	static WgEventFilter	KeyRelease( WgWidget * pWidget, WgKey keycode );

	static WgEventFilter	KeyRepeat();
	static WgEventFilter	KeyRepeat( WgWidget * pWidget );
	static WgEventFilter	KeyRepeat( WgKey keycode );
	static WgEventFilter	KeyRepeat( WgWidget * pWidget, WgKey keycode );

	static WgEventFilter	KeyPressNative( int native_keycode );
	static WgEventFilter	KeyPressNative( WgWidget * pWidget, int native_keycode );

	static WgEventFilter	KeyReleaseNative( int native_keycode );
	static WgEventFilter	KeyReleaseNative( WgWidget * pWidget, int native_keycode );

	static WgEventFilter	KeyRepeatNative( int native_keycode );
	static WgEventFilter	KeyRepeatNative( WgWidget * pWidget, int native_keycode );

	static WgEventFilter	Character();
	static WgEventFilter	Character( unsigned short character);
	static WgEventFilter	Character( WgWidget * pWidget );
	static WgEventFilter	Character( WgWidget * pWidget, unsigned short character );

	static WgEventFilter	MouseWheelRoll();
	static WgEventFilter	MouseWheelRoll( int wheel );
	static WgEventFilter	MouseWheelRoll( WgWidget * pWidget );
	static WgEventFilter	MouseWheelRoll( WgWidget * pWidget, int wheel );

	static WgEventFilter	ButtonPress();
	static WgEventFilter	ButtonPress( WgWidget * pWidget );

	static WgEventFilter	CheckboxCheck();
	static WgEventFilter	CheckboxCheck( WgCheckBox * pWidget );

	static WgEventFilter	CheckboxUncheck();
	static WgEventFilter	CheckboxUncheck( WgCheckBox * pWidget );

	static WgEventFilter	CheckboxToggle();
	static WgEventFilter	CheckboxToggle( WgCheckBox * pWidget );

	static WgEventFilter	RadiobuttonSelect();
	static WgEventFilter	RadiobuttonSelect( WgRadioButton * pWidget );

	static WgEventFilter	RadiobuttonUnselect();
	static WgEventFilter	RadiobuttonUnselect( WgRadioButton * pWidget );

	static WgEventFilter	RadiobuttonToggle();
	static WgEventFilter	RadiobuttonToggle( WgRadioButton * pWidget );

	static WgEventFilter	AnimationUpdate();
	static WgEventFilter	AnimationUpdate( WgAnimPlayer * pWidget );

	static WgEventFilter	SliderMove();
	static WgEventFilter	SliderMove( WgWidgetSlider * pWidget );

	static WgEventFilter	SliderStepFwd();
	static WgEventFilter	SliderStepFwd( WgWidgetSlider * pWidget );

	static WgEventFilter	SliderStepBwd();
	static WgEventFilter	SliderStepBwd( WgWidgetSlider * pWidget );

	static WgEventFilter	SliderJumpFwd();
	static WgEventFilter	SliderJumpFwd( WgWidgetSlider * pWidget );

	static WgEventFilter	SliderJumpBwd();
	static WgEventFilter	SliderJumpBwd( WgWidgetSlider * pWidget );

	static WgEventFilter	SliderWheelRoll();
	static WgEventFilter	SliderWheelRoll( WgWidgetSlider * pWidget );

	static WgEventFilter	TextModify();
	static WgEventFilter	TextModify( WgWidget * pWidget );

	static WgEventFilter	TextSet();
	static WgEventFilter	TextSet( WgWidget * pWidget );

	static WgEventFilter	MenuitemSelect();
	static WgEventFilter	MenuitemSelect( int itemId );
	static WgEventFilter	MenuitemSelect( WgMenu * pWidget );
	static WgEventFilter	MenuitemSelect( WgMenu * pWidget, int itemId );

	static WgEventFilter	MenuitemCheck();
	static WgEventFilter	MenuitemCheck( int itemId );
	static WgEventFilter	MenuitemCheck( WgMenu * pWidget );
	static WgEventFilter	MenuitemCheck( WgMenu * pWidget, int itemId );

	static WgEventFilter	MenuitemUncheck();
	static WgEventFilter	MenuitemUncheck( int itemId );
	static WgEventFilter	MenuitemUncheck( WgMenu * pWidget );
	static WgEventFilter	MenuitemUncheck( WgMenu * pWidget, int itemId );


	//TODO: Add missing filters!


	static WgEventFilter	KnobTurn();
	static WgEventFilter	KnobTurn( WgWidget * pWidget );


	inline bool			FilterEvent( const WgEvent::Event * pEvent ) const { return m_pFilterFunc( pEvent, *this ); }
	inline WgEventType 	EventType() const { return m_eventType; }
	inline bool			FiltersWidget() const { return m_pWidget?true:false; }
	inline WgWidget *	Widget() const { return m_pWidget; }
private:
	WgEventFilter( WgEventType eventType, WgWidget * pWidget, WgFilterFuncPtr pFunc, int data1 = 0, int data2 = 0 )
				: m_eventType(eventType), m_pWidget(pWidget), m_pFilterFunc(pFunc), m_data1(data1), m_data2(data2) {}


	WgEventType			m_eventType;		// Only event type filter accepts, if a specific.
	WgWidget *	 		m_pWidget;			// Only Widget filter accepts, if a specific one.
	WgFilterFuncPtr 	m_pFilterFunc;		// Pointer at filter function.
	int					m_data1;			// Additional data for filter function.
	int					m_data2;			// Additional data for filter function.


	static bool _filterDummy( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterType( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterWidget(const WgEvent::Event * pEvent, const WgEventFilter& filter);
	static bool _filterWidgetId(const WgEvent::Event * pEvent, const WgEventFilter& filter);
	static bool _filterTypeWidget( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterTypeWidgetId(const WgEvent::Event * pEvent, const WgEventFilter& filter);

	static bool _filterPointerEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterMouseButtonEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterKeyEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterNativeKeyEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterCharacterEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterMouseWheelRollEvents( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterTabSelect( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterTabPress( const WgEvent::Event * pEvent, const WgEventFilter& filter );
	static bool _filterMenuitemEvent( const WgEvent::Event * pEvent, const WgEventFilter& filter );
};


#endif // WG_EVENTFILTER_DOT_H
