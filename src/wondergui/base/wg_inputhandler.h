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
#ifndef WG_INPUTHANDLER_DOT_H
#define WG_INPUTHANDLER_DOT_H
#pragma once

#include <wg_pointers.h>
#include <wg_geo.h>
#include <wg_widget.h>
#include <wg_key.h>
#include <wg_msg.h>
#include <wg_root.h>

#include	<map>

/* LEFT TO DO:
 *
 *
 * Automatic garbage collection on msgRouter.
 *
 * Character handling.
 */


namespace wg
{
	class InputHandler;
	typedef	StrongPtr<InputHandler>		InputHandler_p;
	typedef	WeakPtr<InputHandler>		InputHandler_wp;

	class InputHandler : public Object
	{

	friend class RootPanel;

	public:
		//.____ Creation __________________________________________

		static InputHandler_p	create() { return new InputHandler(); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________________

		void 		setPointer(Root * pRoot, Coord pos, int64_t timestamp = 0);
		void 		setButton(MouseButton button, bool bPressed, int64_t timestamp = 0);
		void 		setWheelRoll(int wheel, pts distance, bool bInvertScroll = false, int64_t timestamp = 0);

		void 		setFocusedWindow(Root * pRoot);

		void 		setKey(int nativeKeyCode, bool bPressed, int64_t timestamp = 0);

		bool		putText(const CharSeq& text);

		bool		lockHidePointer();
		bool		unlockShowPointer();

		void		setStayEnteredList( intptr_t listId, Widget ** pBegin, Widget ** pEnd );
		void		clearStayEnteredList( intptr_t listId );

		//.____ Behavior ____________________________________________________________

		void		mapKey(int native_keycode, Key translated_keycode);
		void		unmapKey(int native_keycode);
		void		clearKeyMap();

		void		mapCommand(int native_keycode, ModKeys modKeys, EditCmd command);
		void		unmapCommand(int native_keycode, ModKeys modKeys);
		void		clearCommandMap();

		bool		setButtonRepeat(int delay, int rate);
		bool		setKeyRepeat(int delay, int rate);

		inline int	buttonRepeatDelay() { return m_buttonRepeatDelay; }
		inline int	buttonRepeatRate() { return m_buttonRepeatRate; }

		inline int	keyRepeatDelay() { return m_keyRepeatDelay; }
		inline int	keyRepeatRate() { return m_keyRepeatRate; }

		inline bool	setDoubleClickTresholds(int millisec, int pixels);
		inline int	doubleClickTimeTreshold() { return m_doubleClickTimeTreshold; }
		inline int	doubleClickDistanceTreshold() { return m_doubleClickDistanceTreshold; }

		//.____ State _________________________________________________

		Widget_p 	focusedWidget() const { return _focusedWidget(); }
		Root_p 		focusedWindow() const { return m_pFocusedRoot.rawPtr(); }

		bool		isButtonPressed(MouseButton button) const;
		bool		isAnyButtonPressed() const;

		bool		isKeyPressed(int nativeKeyCode) const;

		ModKeys modifierKeys() const { return m_modKeys; }

		Coord		pointerPos() {return m_pointerPos;}
		CoordSPX	pointerSpxPos() {return m_pointerPosSPX;}
		PointerStyle pointerStyle() { return m_pointerStyle; }

		bool		isEntered( Widget * pWidget ) const;

		//.____ Misc _______________________________________________________


		Key			translateKey( int nativeKeyCode );
		int			nativeKeyCode( Key translatedKeyCode );

		EditCmd		translateCommand( int native_keycode, ModKeys modKeys );

		//.____ Internal ______________________________________________________

		void		_yieldButtonEvents(MouseButton button, Widget* pFrom, Widget* pTo);

		void 		_update(int64_t timestamp);


	protected:
		InputHandler();
		~InputHandler();

		bool		_isInStayEnteredList( Widget * pWidget ) const;

		Widget *	_updateEnteredWidgets( Widget * pMarkedWidget, int64_t timestamp );

		void		_processButtonPress( MouseButton button, int64_t timestamp );
		void		_processButtonRelease( MouseButton button, int64_t timestamp );
		void		_handleButtonRepeats( int64_t timestamp );

		void 		_processKeyPress( int nativeKeyCode, int64_t timestamp );
		void 		_processKeyRelease( int nativeKeyCode, int64_t timestamp );
		void		_handleKeyRepeats( int64_t timestamp );


		bool		_focusChanged( Root * pRoot, Widget * pOldFocused, Widget * pNewFocused );

		int			_widgetPosInList( const Widget * pWidget, const std::vector<Widget_wp>& list ) const;

		void 		_setFocused( Widget * pWidget );
		void 		_setUnfocused( Widget * pWidget );

		Widget *	_focusedWidget() const;


		char			m_inputId;
		int64_t			m_timestamp;

		Coord			m_pointerPos;
		CoordSPX		m_pointerPosSPX;

		Root_wp			m_pMarkedRoot;		// Root widget the pointer currently is "inside".
		Widget_wp		m_pMarkedWidget;	// Widget the pointer currently is "inside". Empty if outside a modal widget.

		Root_wp			m_pFocusedRoot;		// RootPanel that is focused (has the focused widget).

		std::vector<Widget_wp>	m_vEnteredWidgets;	// All widgets that pointer is considered to be inside (= markedWidget + its ancestors).


		PointerStyle	m_pointerStyle;
		ModKeys	m_modKeys;


		// Current button states

		bool			m_bButtonPressed[MouseButton_size];

		int				m_pressOrdinal[MouseButton_size];				// Single, double, triple click counter.
		Widget_wp		m_latestPressWidgets[MouseButton_size];		// Widget that received the latest press, for each button.
		int64_t			m_latestPressTimestamps[MouseButton_size];	// Timestamp of the latest press, for each button.
		Coord			m_latestPressPosition[MouseButton_size];	// Coord of the latest press, for each button.

		// Current key states


		struct KeyDownInfo
		{
			int			nativeKeyCode;
			Key			translatedKeyCode;
			int64_t		pressTimestamp;
			Widget_wp	pWidget;
		};

		std::vector<KeyDownInfo>	m_keysDown;				// One entry for each currently depressed key, in order of being pressed.


		// Settings for keyboard/pointer input

		bool			m_bPointerMovedSinceUpdate = false;	// Keep track of if pointer has moved between calls to _update.

		int				m_doubleClickTimeTreshold;		// Maximum millseconds between first and second press to count as a doubleclick.
		int				m_doubleClickDistanceTreshold;	// Maximum distance the pointer may move between first and second press to count as a doubleclick.

		int				m_buttonRepeatDelay;
		int				m_buttonRepeatRate;

		int				m_keyRepeatDelay;
		int				m_keyRepeatRate;

		std::map<int,Key>	m_keycodeMap;		// Maps native keycodes to Key.

		struct CommandEntry
		{
			ModKeys	modKeys;
			EditCmd			command;
		};

		std::map<int,std::vector<CommandEntry>> m_commandMap;

		//

		std::map<intptr_t,std::vector<Widget_wp>>	m_stayEnteredLists;

		//
		
		Widget_wp		m_pWidgetUnderStillPointer;
		RectSPX			m_widgetUnderStillPointerGeo;
		int64_t			m_lastChangeUnderStillPointer;
		int				m_stillPointerMarkDelay = 200;		// Millisec of stillness under widget before marking.
	};



} // namespace wg
#endif //WG_INPUTHANDLER_DOT_H
