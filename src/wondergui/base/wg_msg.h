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

#ifndef WG_MSG_DOT_H
#define WG_MSG_DOT_H
#pragma once

#include <string>

#include <wg_types.h>
#include <wg_geo.h>
#include <wg_pointers.h>
#include <wg_widget.h>
#include <wg_key.h>





namespace wg
{

	class MsgRouter;
	class Widget;
	class PopupOverlay;
	class ModalOverlay;
	class DragNDropOverlay;

	class StaticText;
	class Text;
	class EditableText;
	class BasicDataset;

	typedef WeakPtr<Object>		Object_wp;

	typedef	ComponentPtr<EditableText>	EditableText_p;

	typedef    StrongPtr<BasicDataset>  BasicDataset_p;
	typedef    WeakPtr<BasicDataset>    BasicDataset_wp;


	typedef WeakPtr<Widget> Widget_wp;

	class Msg;
	typedef	StrongPtr<Msg>		Msg_p;
	typedef	WeakPtr<Msg>	Msg_wp;

	class InputMsg;
	typedef	StrongPtr<InputMsg>		InputMsg_p;
	typedef	WeakPtr<InputMsg>	InputMsg_wp;


	class FocusGainedMsg;
	typedef	StrongPtr<FocusGainedMsg>		FocusGainedMsg_p;
	typedef	WeakPtr<FocusGainedMsg>	FocusGainedMsg_wp;

	class FocusLostMsg;
	typedef	StrongPtr<FocusLostMsg>		FocusLostMsg_p;
	typedef	WeakPtr<FocusLostMsg>	FocusLostMsg_wp;

	class MouseEnterMsg;
	typedef	StrongPtr<MouseEnterMsg>		MouseEnterMsg_p;
	typedef	WeakPtr<MouseEnterMsg>	MouseEnterMsg_wp;

	class MouseLeaveMsg;
	typedef	StrongPtr<MouseLeaveMsg>		MouseLeaveMsg_p;
	typedef	WeakPtr<MouseLeaveMsg>	MouseLeaveMsg_wp;

	class MouseMoveMsg;
	typedef	StrongPtr<MouseMoveMsg>		MouseMoveMsg_p;
	typedef	WeakPtr<MouseMoveMsg>		MouseMoveMsg_wp;

	class MouseButtonMsg;
	typedef	StrongPtr<MouseButtonMsg>		MouseButtonMsg_p;
	typedef	WeakPtr<MouseButtonMsg>	MouseButtonMsg_wp;

	class MousePressMsg;
	typedef	StrongPtr<MousePressMsg>		MousePressMsg_p;
	typedef	WeakPtr<MousePressMsg>	MousePressMsg_wp;

	class MouseReleaseMsg;
	typedef	StrongPtr<MouseReleaseMsg>		MouseReleaseMsg_p;
	typedef	WeakPtr<MouseReleaseMsg>	MouseReleaseMsg_wp;

	class MouseRepeatMsg;
	typedef	StrongPtr<MouseRepeatMsg>		MouseRepeatMsg_p;
	typedef	WeakPtr<MouseRepeatMsg>	MouseRepeatMsg_wp;

	class MouseClickMsg;
	typedef	StrongPtr<MouseClickMsg>		MouseClickMsg_p;
	typedef	WeakPtr<MouseClickMsg>	MouseClickMsg_wp;

	class MouseDoubleClickMsg;
	typedef	StrongPtr<MouseDoubleClickMsg>		MouseDoubleClickMsg_p;
	typedef	WeakPtr<MouseDoubleClickMsg>	MouseDoubleClickMsg_wp;

	class MouseTripleClickMsg;
	typedef	StrongPtr<MouseTripleClickMsg>	MouseTripleClickMsg_p;
	typedef	WeakPtr<MouseTripleClickMsg>	MouseTripleClickMsg_wp;

	class MouseDragMsg;
	typedef	StrongPtr<MouseDragMsg>		MouseDragMsg_p;
	typedef	WeakPtr<MouseDragMsg>	MouseDragMsg_wp;

	class KeyMsg;
	typedef	StrongPtr<KeyMsg>		KeyMsg_p;
	typedef	WeakPtr<KeyMsg>	KeyMsg_wp;

	class KeyPressMsg;
	typedef	StrongPtr<KeyPressMsg>		KeyPressMsg_p;
	typedef	WeakPtr<KeyPressMsg>	KeyPressMsg_wp;

	class KeyReleaseMsg;
	typedef	StrongPtr<KeyReleaseMsg>		KeyReleaseMsg_p;
	typedef	WeakPtr<KeyReleaseMsg>	KeyReleaseMsg_wp;

	class KeyRepeatMsg;
	typedef	StrongPtr<KeyRepeatMsg>		KeyRepeatMsg_p;
	typedef	WeakPtr<KeyRepeatMsg>	KeyRepeatMsg_wp;

	class TextInputMsg;
	typedef	StrongPtr<TextInputMsg>		TextInputMsg_p;
	typedef	WeakPtr<TextInputMsg>	TextInputMsg_wp;

	class EditCommandMsg;
	typedef	StrongPtr<EditCommandMsg>		EditCommandMsg_p;
	typedef	WeakPtr<EditCommandMsg>	EditCommandMsg_wp;

	class WheelRollMsg;
	typedef	StrongPtr<WheelRollMsg>		WheelRollMsg_p;
	typedef	WeakPtr<WheelRollMsg>	WheelRollMsg_wp;

	class PointerChangeMsg;
	typedef	StrongPtr<PointerChangeMsg>		PointerChangeMsg_p;
	typedef	WeakPtr<PointerChangeMsg>	PointerChangeMsg_wp;

	class DropMsg;
	typedef StrongPtr<DropMsg>        DropMsg_p;
	typedef WeakPtr<DropMsg>    DropMsg_wp;

	class PickMsg;
	typedef StrongPtr<PickMsg>  PickMsg_p;
	typedef WeakPtr<PickMsg>    PickMsg_wp;

	class DropProbeMsg;
	typedef StrongPtr<DropProbeMsg>  DropProbeMsg_p;
	typedef WeakPtr<DropProbeMsg>    DropProbeMsg_wp;

	class DropEnterMsg;
	typedef StrongPtr<DropEnterMsg>        DropEnterMsg_p;
	typedef WeakPtr<DropEnterMsg>    DropEnterMsg_wp;

	class DropMoveMsg;
	typedef StrongPtr<DropMoveMsg>        DropMoveMsg_p;
	typedef WeakPtr<DropMoveMsg>    DropMoveMsg_wp;

	class DropLeaveMsg;
	typedef StrongPtr<DropLeaveMsg>        DropLeaveMsg_p;
	typedef WeakPtr<DropLeaveMsg>    DropLeaveMsg_wp;

	class DropDeliverMsg;
	typedef StrongPtr<DropDeliverMsg>        DropDeliverMsg_p;
	typedef WeakPtr<DropDeliverMsg>    DropDeliverMsg_wp;

	class PickedCancelMsg;
	typedef StrongPtr<PickedCancelMsg>        PickedCancelMsg_p;
	typedef WeakPtr<PickedCancelMsg>    PickedCancelMsg_wp;

	class PickedDeliverMsg;
	typedef StrongPtr<PickedDeliverMsg>       PickedDeliverMsg_p;
	typedef WeakPtr<PickedDeliverMsg>    PickedDeliverMsg_wp;


	class SelectMsg;
	typedef	StrongPtr<SelectMsg>		SelectMsg_p;
	typedef	WeakPtr<SelectMsg>	SelectMsg_wp;

	class UnselectMsg;
	typedef	StrongPtr<UnselectMsg>		UnselectMsg_p;
	typedef	WeakPtr<UnselectMsg>	UnselectMsg_wp;

	class ToggleMsg;
	typedef	StrongPtr<ToggleMsg>		ToggleMsg_p;
	typedef	WeakPtr<ToggleMsg>	ToggleMsg_wp;

	class ScrollbarMoveMsg;
	typedef	StrongPtr<ScrollbarMoveMsg>		ScrollbarMoveMsg_p;
	typedef	WeakPtr<ScrollbarMoveMsg>	ScrollbarMoveMsg_wp;

	class ValueUpdateMsg;
	typedef	StrongPtr<ValueUpdateMsg>		ValueUpdateMsg_p;
	typedef	WeakPtr<ValueUpdateMsg>	ValueUpdateMsg_wp;

	class RangeUpdateMsg;
	typedef	StrongPtr<RangeUpdateMsg>		RangeUpdateMsg_p;
	typedef	WeakPtr<RangeUpdateMsg>	RangeUpdateMsg_wp;

	class TextEditMsg;
	typedef	StrongPtr<TextEditMsg>		TextEditMsg_p;
	typedef	WeakPtr<TextEditMsg>	TextEditMsg_wp;

	class ItemMsg;
	typedef	StrongPtr<ItemMsg>		ItemMsg_p;
	typedef	WeakPtr<ItemMsg>	ItemMsg_wp;

	class ItemListMsg;
	typedef	StrongPtr<ItemListMsg>		ItemListMsg_p;
	typedef	WeakPtr<ItemListMsg>	ItemListMsg_wp;

	class ItemsSelectMsg;
	typedef	StrongPtr<ItemsSelectMsg>		ItemsSelectMsg_p;
	typedef	WeakPtr<ItemsSelectMsg>	ItemsSelectMsg_wp;

	class ItemsUnselectMsg;
	typedef	StrongPtr<ItemsUnselectMsg>		ItemsUnselectMsg_p;
	typedef	WeakPtr<ItemsUnselectMsg>	ItemsUnselectMsg_wp;

	class ItemToggleMsg;
	typedef	StrongPtr<ItemToggleMsg>		ItemToggleMsg_p;
	typedef	WeakPtr<ItemToggleMsg>	ItemToggleMsg_wp;

	class ItemMousePressMsg;
	typedef	StrongPtr<ItemMousePressMsg>		ItemMousePressMsg_p;
	typedef	WeakPtr<ItemMousePressMsg>	ItemMousePressMsg_wp;

	class PopupSelectMsg;
	typedef	StrongPtr<PopupSelectMsg>		PopupSelectMsg_p;
	typedef	WeakPtr<PopupSelectMsg>			PopupSelectMsg_Wp;

	class PopupClosedMsg;
	typedef	StrongPtr<PopupClosedMsg>		PopupClosedMsg_p;
	typedef	WeakPtr<PopupClosedMsg>	PopupClosedMsg_wp;

	class ModalMoveOutsideMsg;
	typedef	StrongPtr<ModalMoveOutsideMsg>		ModalMoveOutsideMsg_p;
	typedef	WeakPtr<ModalMoveOutsideMsg>	ModalMoveOutsideMsg_wp;

	class ModalBlockedPressMsg;
	typedef	StrongPtr<ModalBlockedPressMsg>		ModalBlockedPressMsg_p;
	typedef	WeakPtr<ModalBlockedPressMsg>	ModalBlockedPressMsg_wp;

	class ModalBlockedReleaseMsg;
	typedef	StrongPtr<ModalBlockedReleaseMsg>		ModalBlockedReleaseMsg_p;
	typedef	WeakPtr<ModalBlockedReleaseMsg>	ModalBlockedReleaseMsg_wp;

	class ActivateMsg;
	typedef	StrongPtr<ActivateMsg>		ActivateMsg_p;
	typedef	WeakPtr<ActivateMsg>		ActivateMsg_wp;

	class DeactivateMsg;
	typedef	StrongPtr<DeactivateMsg>	DeactivateMsg_p;
	typedef	WeakPtr<DeactivateMsg>		DeactivateMsg_wp;

	class PingMsg;
	typedef	StrongPtr<PingMsg>			PingMsg_p;
	typedef	WeakPtr<PingMsg>			PingMsg_wp;


	//____ Msg _________________________________________________________________
	/**
	 * @brief Base class for all messages.
	 *
	 * Base class for all messages.
	 */


	class Msg : public Object
	{

		public:

			//.____ Identification __________________________________________

			const TypeInfo&		typeInfo(void) const override;
			const static TypeInfo	TYPEINFO;

			MsgType				type() const { return m_type; }
			bool				isMouseMsg() const;
			bool				isMouseButtonMsg() const;
			bool				isKeyMsg() const;

			//.____ Content ____________________________________________________

			bool				reposted() const { return m_bReposted; }
			Object *			originalSourceRawPtr() const { return m_bReposted? m_pOriginalSource.rawPtr() : m_pSource.rawPtr(); }
			Object_p			originalSource() const { return m_bReposted? m_pOriginalSource : m_pSource; }

			bool				hasSource() const { return m_pSource; }
			Object *			sourceRawPtr() const { return m_pSource.rawPtr(); }
			Object_p			source() const { return m_pSource; }

			bool				hasCopyTo() { return m_pCopyTo; }
			Receiver_p			getCopyTo() { return m_pCopyTo; }
			bool				hasRepost() { return m_pRepostSource; }

			bool                hasFinalRecipient() { return m_pFinalRecipient; }
			Receiver_p          finalRecipient() { return m_pFinalRecipient; }

			//.____ Control ____________________________________________________

			void				setCopyTo( Receiver * pReceiver );
			void				setRepost( Object * pFromSource, Receiver * pCopyTo );
			void				swallow();
			bool				doRepost();

		protected:
			Msg() : m_type(MsgType::Dummy), m_bReposted(false) {}
			virtual ~Msg() {}

			MsgType				m_type;				// Type of message
			Object_p			m_pSource;			// The source of this message, if any. Not necessarily the sender.
			bool				m_bReposted;		// Set if this is a repost.
			Object_p			m_pOriginalSource;	// If reposted, this is the original source.
			Receiver_p			m_pCopyTo;			// Receiver to receive a copy of this message, if any.
			Object_p			m_pRepostSource;	// Object to repost this message from, if any.
			Receiver_p			m_pRepostCopyTo;	// Receiver to copy this message to when reposting, if any.
			Receiver_p          m_pFinalRecipient;  // Final recipent of (possibly modified) message, if any.
	};


	//____ InputMsg ______________________________________________________
	/**
	 * @brief Base class for all mouse and key messages.
	 *
	 */

	class InputMsg : public Msg
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ____________________________________________________

		char				inputId() const { return m_inputId; }
		int64_t				timestamp() const { return m_timestamp; }
		ModKeys				modKeys() const { return m_modKeys; }
		Coord				pointerPos() const { return m_pointerPos; }

		CoordSPX			pointerSpxPos() const { return m_pointerPosSPX; }

	protected:
		InputMsg(char inputId, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp) : m_inputId(inputId), m_modKeys(modKeys), m_pointerPos(pointerPos), m_pointerPosSPX(pointerPosSPX), m_timestamp(timestamp) {}

		char				m_inputId;			// Id of InputHandler posting this message, so we can separate multiple input sources from each other.
		ModKeys		m_modKeys;			// Modifier keys pressed when message posted.
		Coord				m_pointerPos;		// Screen position of pointer in points.
		CoordSPX			m_pointerPosSPX;	// Screen position of pointer in subpixels.
		int64_t				m_timestamp;		// Timestamp of input event.
	};


	//____ MouseButtonMsg ______________________________________________________
	/**
	 * @brief Base class for all mouse button messages.
	 *
	 */

	class MouseButtonMsg : public InputMsg
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ____________________________________________________

		MouseButton		button() const { return m_button; }

	protected:
		MouseButtonMsg(char inputId, MouseButton button, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp) : InputMsg(inputId, modKeys, pointerPos, pointerPosSPX, timestamp), m_button(button) {}

		MouseButton		m_button;
	};


	//____ KeyMsg ______________________________________________________________
	/**
	 * @brief Base class for all keyboard messages.
	 *
	 * Base class for all keyboard messages.
	 */

	class KeyMsg : public InputMsg
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ____________________________________________________

		int		nativeKeyCode() const { return m_nativeKeyCode; }
		Key		translatedKeyCode() const { return m_translatedKeyCode; }
		bool	isCursorKey() const;
		bool	isMovementKey() const;

	protected:
		KeyMsg( char inputId, int nativeKeyCode, Key translatedKeyCode, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp) : InputMsg(inputId, modKeys, pointerPos, pointerPosSPX, timestamp), m_nativeKeyCode(nativeKeyCode), m_translatedKeyCode(translatedKeyCode) {}

		int		m_nativeKeyCode;
		Key		m_translatedKeyCode;
	};

	//____ FocusGainedMsg ______________________________________________________

	class FocusGainedMsg : public InputMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		FocusGainedMsg( char inputId, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp);
	};

	//____ FocusLostMsg ________________________________________________________

	class FocusLostMsg : public InputMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		FocusLostMsg( char inputId, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp);
	};

	//____ MouseEnterMsg _______________________________________________________

	class MouseEnterMsg : public InputMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseEnterMsg_p create( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
			{ return new MouseEnterMsg( inputId, pSource, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
	protected:
		MouseEnterMsg( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ MouseLeaveMsg _______________________________________________________

	class MouseLeaveMsg : public InputMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseLeaveMsg_p create( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
			{ return new MouseLeaveMsg( inputId, pSource, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
	protected:
		MouseLeaveMsg( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ MouseMoveMsg ________________________________________________________

	class MouseMoveMsg : public InputMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseMoveMsg_p create( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
			{ return new MouseMoveMsg( inputId, pSource, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
	protected:
		MouseMoveMsg( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ MousePressMsg _______________________________________________________

	class MousePressMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MousePressMsg_p create( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
			{ return new MousePressMsg( inputId, button, pSource, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;
	protected:
		MousePressMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ MouseReleaseMsg _____________________________________________________

	class MouseReleaseMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseReleaseMsg_p create( char inputId, MouseButton button, Object * pSource, bool bReleaseInside, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
			{ return new MouseReleaseMsg( inputId, button, pSource, bReleaseInside, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		bool			releaseInside() const;

	protected:
		MouseReleaseMsg( char inputId, MouseButton button, Object * pSource, bool bReleaseInside, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );

		bool			m_bReleaseInside;
	};

	//____ KeyPressMsg _________________________________________________________

	class KeyPressMsg : public KeyMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		KeyPressMsg( char inputId, int native_keycode, Key translated_keycode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ KeyReleaseMsg _______________________________________________________

	class KeyReleaseMsg : public KeyMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		KeyReleaseMsg( char inputId, int nativeKeyCode, Key translatedKeyCode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	//____ TextInputMsg ________________________________________________________

	class TextInputMsg : public Msg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		String	text() const;
		char	inputId() const { return m_inputId; }

	protected:
		TextInputMsg( char inputId, String text, Widget * pWidget );
	protected:
		char	m_inputId;
		String	m_text;
	};

	//____ EditCommandMsg ________________________________________________________

	class EditCommandMsg : public Msg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		EditCmd	command() const { return m_command; }
		char	inputId() const { return m_inputId; }

	protected:
		EditCommandMsg( char inputId, EditCmd command, Widget * pWidget );
	protected:
		char	m_inputId;
		EditCmd	m_command;
	};


	//____ WheelRollMsg ________________________________________________________

	class WheelRollMsg : public InputMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		int			wheel() const;
		pts			distance() const;
		bool		invertScroll() const;
	protected:
		WheelRollMsg( char inputId, int wheel, pts distance, bool bInvertScroll, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );

		int			m_wheel;
		pts			m_distance;
		bool		m_bInvertScroll;
	};

	//____ PointerChangeMsg ____________________________________________________

	class PointerChangeMsg : public Msg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		PointerStyle	style() const;
		inline char		inputId() const { return m_inputId; }

	protected:
		PointerChangeMsg( char inputId, PointerStyle style );

		char			m_inputId;
		PointerStyle	m_style;
	};

	//____ PickMsg ___________________________________________________

	class PickMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Coord				pickOfs() const { return m_pickOfs; }

		void				setContent( DropType type, int category, BasicDataset * pDataset );

		void				setHotspot( Placement hotspot );		// Placement::Undefined gives hotspot on cursor.
		Placement			hotspot() const { return m_hotspot; }

		void				setDragWidget( Widget * pWidget, Coord pointerOfs );
		bool				hasDragWidget() const { return m_pDragWidget; }
		Widget_p            dragWidget() const;
		Coord				dragWidgetPointerOfs() const;

		void				setDragConfinement( Rect geo) { m_dragConfinement = geo; }
		bool				hasDragConfinement() const { return !m_dragConfinement.isEmpty(); }
		Rect				dragConfinement() const { return m_dragConfinement; }

		bool				hasDataset() const;
		BasicDataset_p		dataset() const;
		int                 category() const { return m_category; }
		DropType			dropType() const { return m_dropType; }

		Coord				pointerPos() const { return m_pointerPos; }
		ModKeys				modKeys() const { return m_modKeys; }

	protected:
		PickMsg( Widget * pSource, Coord pickOfs, Widget * pFinalReceiver, ModKeys modKeys, Coord pointerPos );

		Widget_p            m_pDragWidget;
		Coord				m_dragWidgetPointerOfs;
		Coord               m_pickOfs;
		Placement			m_hotspot = Placement::Undefined;
		wg::Rect			m_dragConfinement = {0, 0, 0, 0};

		DropType			m_dropType = DropType::Undefined;	// Type of data in m_pDataset.
		int                 m_category = 0;						// User defined category.
		BasicDataset_p      m_pDataset;							// Contains data being dropped.

		Coord				m_pointerPos;		// Screen position of pointer.
		ModKeys				m_modKeys;			// Modifier keys pressed when message posted.
	};

	//____ PickedEnterMsg ______________________________________________

	class PickedEnterMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content __________________________________________________

		Widget_p	target() const;

	protected:
		PickedEnterMsg(Widget* pSource, Widget * pTarget );

		Widget_p	m_pTarget;
	};

	//____ PickedLeaveMsg ______________________________________________

	class PickedLeaveMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content __________________________________________________

		Widget_p	target() const;

	protected:
		PickedLeaveMsg(Widget* pSource, Widget* pTarget);

		Widget_p	m_pTarget;
	};

	//____ DropMsg _______________________________________________________

	class DropMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Coord				dropPos() const { return m_dropPos; }
		ModKeys				modKeys() const { return m_modKeys; }

		bool				hasDataset() const;
		BasicDataset_p		dataset() const;
		Widget_p 			pickedFrom() const { return m_pPickedFrom; }
		int                 category() const { return m_category; }
		DropType			dropType() const { return m_dropType; }

	protected:
		DropMsg( MsgType type, Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos );

		DropType			m_dropType;			// Type of data in m_pDataset.
		int                 m_category;			// User defined category.
		BasicDataset_p      m_pDataset;			// Contains data being dropped.

		Coord				m_dropPos;			// Global drop position, either position of pointer or hotspot if set.
		ModKeys				m_modKeys;			// Modifier keys pressed when message posted.
		Widget_p            m_pPickedFrom;
	 };

	//____ DropProbeMsg ___________________________________________________

	class DropProbeMsg : public DropMsg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		void			accept( bool bAccept = true );
		bool			isAccepted() const { return m_bAccepted; }

	protected:
		DropProbeMsg( Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos  );

  		bool		m_bAccepted = false;
	};


	//____ DropEnterMsg ___________________________________________________

	class DropEnterMsg : public DropMsg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		void                setDragWidget( Widget * pWidget );
		Widget_p            dragWidget() const;

	protected:
		DropEnterMsg( Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pDragWidget, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos );

		Widget_p            m_pDragWidget;
	};

	//____ DropMoveMsg ___________________________________________________

	class DropMoveMsg : public DropMsg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		void                setDragWidget( Widget * pWidget );
		Widget_p            dragWidget() const;

	protected:
		DropMoveMsg( Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pDragWidget, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos );

		Widget_p            m_pDragWidget;
	};

	//____ DropLeaveMsg ___________________________________________________

	class DropLeaveMsg : public DropMsg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

	protected:
		DropLeaveMsg( Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, ModKeys modKeys, Coord dropPos );
	};


	//____ DropDeliverMsg ___________________________________________________

	class DropDeliverMsg : public DropMsg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Widget_p 	deliveredTo() const;
		void		accept( bool bAccept = true );
		bool		isAccepted() const { return m_bAccepted; }

	protected:
		DropDeliverMsg( Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos );

  		bool		m_bAccepted;
	};


	//____ PickedCancelMsg ___________________________________________________

	class PickedCancelMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________


	protected:
		PickedCancelMsg( Widget * pPickedFrom );

	};

	//____ PickedDeliverMsg ___________________________________________________

	class PickedDeliverMsg : public Msg
	{
		friend class DragNDropOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Widget_p 	deliveredTo() const;

	protected:
		PickedDeliverMsg( Widget * pPicked, Widget * pDeliveree );

  		Widget_p 	m_pDeliveree;
	};


	//____ SelectMsg ___________________________________________________________

	class SelectMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static SelectMsg_p	create( Object * pSource ) { return new SelectMsg(pSource); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		SelectMsg( Object * pSource );
	};

	//____ ToggleMsg ___________________________________________________________

	class ToggleMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static ToggleMsg_p	create( Object * pSource, bool bSet ) { return new ToggleMsg(pSource,bSet); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		bool		isSet() const;

	protected:
		ToggleMsg( Object * pSource, bool bSet );

		bool	m_bSet;
	};

	//____ ScrollbarMoveMsg ______________________________________________________

	class ScrollbarMoveMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static ScrollbarMoveMsg_p	create(Object* pSource, pts viewPos, float fracPos) { return new ScrollbarMoveMsg(pSource, viewPos, fracPos); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		pts			viewPos() const;
		float		fracPos() const;

	protected:
		ScrollbarMoveMsg(Object* pSource, pts viewPos, float fracPos);

		pts			m_viewPos;
		float		m_fracPos;
	};


	//____ ValueUpdateMsg ______________________________________________________

	class ValueUpdateMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static ValueUpdateMsg_p	create( Object * pSource, float value, bool bFinal ) { return new ValueUpdateMsg(pSource, value, bFinal ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		float		value() const;
		bool		isFinal() const;

	protected:
		ValueUpdateMsg( Object * pSource, float value, bool bFinal );

		float		m_value;
		bool		m_bFinal;
	};

	//____ RangeUpdateMsg ______________________________________________________

	class RangeUpdateMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static RangeUpdateMsg_p create( Object * pSource, pts ofs, pts length, double fracOfs, double fracLength, bool bFinal )
										{ return new RangeUpdateMsg( pSource, ofs, length, fracOfs, fracLength, bFinal ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		pts			offset() const;
		pts			length() const;
		double		fracOffset() const;
		double		fracLength() const;
		bool		isFinal() const;

	protected:
		RangeUpdateMsg( Object * pSource, int intOfs, int intLength, double fracOfs, double fracLength, bool bFinal );

		pts			m_ofs;
		pts			m_length;
		double		m_fracOfs;
		double		m_fracLength;
		bool		m_bFinal;
	};

	//____ TextEditMsg _________________________________________________________

	class TextEditMsg : public Msg
{
public:
	//.____ Creation __________________________________________
	
	inline static TextEditMsg_p create( EditableText * pText, int offset, int deleted, int inserted ) { return new TextEditMsg( pText, offset, deleted, inserted ); }
	
	//.____ Identification __________________________________________
	
	const TypeInfo&		typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;
	
	//.____ Content ________________________________________________________
	
	EditableText_p		text() const;
	int					offset() const { return m_offset; }
	int					deleted() const { return m_nbDeleted; }
	int					inserted() const { return m_nbInserted; }
		
	protected:
		TextEditMsg( EditableText * pText, int offset, int deleted, int inserted );

		EditableText *		m_pText;
		int					m_offset;
		int					m_nbDeleted;
		int					m_nbInserted;
	};

	//____ ItemInfo _______________________________

	class ItemInfo
	{
	public:

		//.____ Creation _________________________________

		ItemInfo(): id(-1) {}
		ItemInfo( int _id, Object * _pObject ) : id(_id), pObject(_pObject) {}

		//.____ Properties _________________________________

		int				id;
		Object_p		pObject;
	};

	//____ ItemMsg ____________________________________________________________

	class ItemMsg : public Msg
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		int				itemId() const;
		Object_p		itemObject() const;

	protected:
		ItemMsg( Object * pSource, int itemId, Object * pItemObject );

		ItemInfo		m_item;
	};

	//____ ItemToggleMsg ______________________________________________________

	class ItemToggleMsg : public ItemMsg
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		bool		isSet() const;

	//protected:
		ItemToggleMsg( Object * pSource, int itemId, Object * pItemObject, bool bSet );

		bool		m_bSet;
	};

	//____ ItemMousePressMsg ______________________________________________________

	class ItemMousePressMsg : public ItemMsg
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		MouseButton		button() const;
	//protected:
		ItemMousePressMsg( Object * pSource, int itemId, Object * pItemObject, MouseButton button );

		MouseButton		m_button;
	};


	//____ ItemListMsg ___________________________________________________________

	class ItemListMsg : public Msg
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		int					nbItems() const;
		const ItemInfo *	items() const;

	protected:
		ItemListMsg( Object * pSource, int nbItems, ItemInfo * pItems );
		virtual ~ItemListMsg();
		int				m_nbItems;
		ItemInfo *	m_pItems;
	};


	//____ ItemsSelectMsg _____________________________________________________

	class ItemsSelectMsg : public ItemListMsg
	{
	public:
		//.____ Creation __________________________________________

		static ItemsSelectMsg_p	create( Object * pSource, int nbItems, ItemInfo * pItems ) { return new ItemsSelectMsg(pSource, nbItems, pItems); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
			ItemsSelectMsg( Object * pSource, int nbItems, ItemInfo * pItems );
	};

	//____ ItemsUnselectMsg _____________________________________________________

	class ItemsUnselectMsg : public ItemListMsg
	{
	public:
		//.____ Creation __________________________________________

		static ItemsUnselectMsg_p create( Object * pSource, int nbItems, ItemInfo * pItems ) { return new ItemsUnselectMsg( pSource, nbItems, pItems ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ItemsUnselectMsg( Object * pSource, int nbItems, ItemInfo * pItems );

	};

	//____ PopupSelectMsg ___________________________________________________________

	class PopupSelectMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		inline static PopupSelectMsg_p	create( Object * pSource, Widget * pSelected ) { return new PopupSelectMsg(pSource, pSelected); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Widget_p	selected() const { return m_pSelected; }

	protected:
		PopupSelectMsg( Object * pSource, Widget * pSelected );
		
		Widget_p	m_pSelected;
	};

	//____ PopupOpenedMsg ______________________________________________________

	class PopupOpenedMsg : public Msg
	{
		friend class PopupOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		Widget_p	opener() const { return m_pOpener; }

	protected:
		PopupOpenedMsg( Widget * pPopup, const Widget_wp& pOpener );

		Widget_p	m_pOpener;
	};


	//____ PopupClosedMsg ______________________________________________________

	class PopupClosedMsg : public Msg
	{
		friend class PopupOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		PopupClosedMsg( Widget * pPopup, const Widget_wp& pOpener );
	};


	//____ WidgetModalOverlay messages _________________________________________________

	class ModalMoveOutsideMsg : public InputMsg
	{
		friend class ModalOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ModalMoveOutsideMsg( char inputId, Widget * pModalWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	class ModalBlockedPressMsg : public MouseButtonMsg
	{
		friend class ModalOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ModalBlockedPressMsg( char inputId, MouseButton button, Widget * pModalWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	class ModalBlockedReleaseMsg : public MouseButtonMsg
	{
		friend class ModalOverlay;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ModalBlockedReleaseMsg( char inputId, MouseButton button, Widget * pModalWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};


	//____ PianoKeyPressMsg ___________________________________________________
	/**
	 *	@brief Key is pressed by users action.
	 *
	 * This message is posted when a piano key is pressed as the result of a user action, e.i.
	 * pressing left mouse key on a piano key or moving pointer to key while pressed.
	 * 
	 * No message is posted if piano key is pressed through an API call or key already is
	 * pressed when user presses it.
	 *
	 */

	class PianoKeyPressMsg : public Msg
	{
		friend class PianoKeyboard;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		int					key() { return m_keyIdx; }
		int64_t				timestamp() const { return m_timestamp; }

	protected:
		PianoKeyPressMsg(Object * pSource, int keyIdx, int64_t timestamp);

		int		m_keyIdx;
		int64_t	m_timestamp;
	};

	//____ PianoKeyReleaseMsg ___________________________________________________
	/**
	 *	@brief Key is released by users action.
	 *
	 * This message is posted when a piano key is released as the result of a user action, e.i.
	 * releasing mouse key on a piano key or moving pointer away from key while pressed.
	 *
	 * No message is posted if piano key remains pressed (due to pressed state set
	 * through API call).
	 *
	 */

	class PianoKeyReleaseMsg : public Msg
	{
		friend class PianoKeyboard;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ________________________________________________________

		int					key() { return m_keyIdx; }
		int64_t				timestamp() const { return m_timestamp; }

	protected:
		PianoKeyReleaseMsg(Object * pSource, int keyIdx, int64_t timestamp);

		int		m_keyIdx;
		int64_t	m_timestamp;
	};


	//____ ActivateMsg _____________________________________________________

	class ActivateMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		static ActivateMsg_p	create( Object * pSource ) { return new ActivateMsg(pSource); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
			ActivateMsg( Object * pSource );
	};

	//____ DeactivateMsg _____________________________________________________

	class DeactivateMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		static DeactivateMsg_p	create( Object * pSource ) { return new DeactivateMsg(pSource); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
			DeactivateMsg( Object * pSource );
	};

	//____ UpdateMsg _____________________________________________________

	class PingMsg : public Msg
	{
	public:
		//.____ Creation __________________________________________

		static PingMsg_p	create( Object * pSource ) { return new PingMsg(pSource); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
			PingMsg( Object * pSource );
	};


	//____ Internally posted messages ____________________________________________

	class MouseDragMsg : public MouseButtonMsg
	{
	public:

		//.____ Creation __________________________________________

		inline static MouseDragMsg_p create( char inputId, MouseButton button, Object * pSource, const Coord& orgPos, const Coord& prevPos, const CoordSPX& orgPosSPX, const CoordSPX& prevPosSPX, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp )
						{ return new MouseDragMsg( inputId, button, pSource, orgPos, prevPos, orgPosSPX, prevPosSPX, modKeys, pointerPos, pointerPosSPX, timestamp ); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ______________________________________________

		Coord			draggedTotal() const;
		Coord			draggedNow() const;
		Coord			startPos() const;
		Coord			prevPos() const;
		Coord			currPos() const;
		
		//.____ Internal ______________________________________________

		CoordSPX		_draggedTotal() const;
		CoordSPX		_draggedNow() const;
		CoordSPX		_startPos() const;
		CoordSPX		_prevPos() const;
		CoordSPX		_currPos() const;


	protected:
		MouseDragMsg( char inputId, MouseButton button, Object * pSource, const Coord& orgPos, const Coord& prevPos, const CoordSPX& orgPosSPX, const CoordSPX& prevPosSPX, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );

		Coord			m_startPos;
		Coord			m_prevPos;

		CoordSPX		m_startPosSPX;
		CoordSPX		m_prevPosSPX;
	};

	class MouseRepeatMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseRepeatMsg_p create(char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp)
							{ return new MouseRepeatMsg(inputId, button, pSource, modKeys, pointerPos, pointerPosSPX, timestamp); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		MouseRepeatMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	class MouseClickMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseClickMsg_p create(char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp, int duration, int ordinal )
									{ return new MouseClickMsg(inputId, button, pSource, modKeys, pointerPos, pointerPosSPX, timestamp, duration, ordinal); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ______________________________________________

		int		duration() const { return m_duration; }
		int		ordinal() const { return m_ordinal; }

	protected:
		MouseClickMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp, int duration, int ordinal );

		int		m_duration;
		int		m_ordinal;
	};

	class MouseDoubleClickMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseDoubleClickMsg_p create(char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp)
					{ return new MouseDoubleClickMsg(inputId, button, pSource, modKeys, pointerPos, pointerPosSPX, timestamp); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		MouseDoubleClickMsg( char inputId, MouseButton button, Object * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

	class MouseTripleClickMsg : public MouseButtonMsg
	{
	public:
		//.____ Creation __________________________________________

		inline static MouseTripleClickMsg_p create(char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp)
					{ return new MouseTripleClickMsg(inputId, button, pSource, modKeys, pointerPos, pointerPosSPX, timestamp); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		MouseTripleClickMsg( char inputId, MouseButton button, Object * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};


	class KeyRepeatMsg : public KeyMsg
	{
		friend class InputHandler;
	public:
		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		KeyRepeatMsg( char inputId, int native_keycode, Key translated_keycode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp );
	};

} // namespace wg
#endif //WG_MSG_DOT_H
