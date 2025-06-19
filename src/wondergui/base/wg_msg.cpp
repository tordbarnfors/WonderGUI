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

#include <wg_msg.h>
#include <wg_key.h>
#include <wg_widget.h>
#include <wg_editabletext.h>
#include <wg_dataset.h>

namespace wg
{

	//____ Msg ______________________________________________________________

	const TypeInfo Msg::TYPEINFO = { "Msg", &Object::TYPEINFO };

	const TypeInfo& Msg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void Msg::setCopyTo( Receiver * pReceiver )
	{
		m_pCopyTo = pReceiver;
	}

	void Msg::setRepost( Object * pSource, Receiver * pCopyTo )
	{
		m_pRepostSource = pSource;
		m_pRepostCopyTo = pCopyTo;
	}

	void Msg::swallow()
	{
		m_pRepostSource = 0;
		m_pRepostCopyTo = 0;
	}

	bool Msg::doRepost()
	{
		if( m_pRepostSource )
		{
			if (!m_bReposted)
			{
				m_pOriginalSource = m_pSource;
				m_bReposted = true;
			}

			m_pSource = m_pRepostSource;
			m_pCopyTo = m_pRepostCopyTo;

			m_pRepostSource = 0;
			m_pRepostCopyTo = 0;

			return true;
		}
		return false;
	}



	bool Msg::isMouseMsg() const
	{
		if( m_type == MsgType::WheelRoll ||
			m_type == MsgType::MouseEnter ||
			m_type == MsgType::MouseLeave ||
			m_type == MsgType::MouseMove ||
			isMouseButtonMsg() )
			return true;
		else
			return false;

	}


	bool Msg::isMouseButtonMsg() const
	{
		if( m_type == MsgType::MouseClick ||
			m_type == MsgType::MouseDoubleClick ||
			m_type == MsgType::MouseTripleClick ||
			m_type == MsgType::MouseDrag ||
			m_type == MsgType::MousePress ||
			m_type == MsgType::MouseRelease ||
			m_type == MsgType::MouseRepeat )
			return true;
		else
			return false;
	}

	bool Msg::isKeyMsg() const
	{
		if( m_type == MsgType::KeyPress ||
			m_type == MsgType::KeyRelease ||
			m_type == MsgType::KeyRepeat )
			return true;
		else
			return false;
	}

	//____ InputMsg __________________________________________________________

	const TypeInfo InputMsg::TYPEINFO = { "InputMsg", &Msg::TYPEINFO };

	const TypeInfo& InputMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}



	//____ KeyMsg ____________________________________________________________

	const TypeInfo KeyMsg::TYPEINFO = { "KeyMsg", &InputMsg::TYPEINFO };

	const TypeInfo& KeyMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	bool KeyMsg::isCursorKey() const
	{
		if( m_translatedKeyCode == Key::Up || m_translatedKeyCode == Key::Down ||
			m_translatedKeyCode == Key::Left || m_translatedKeyCode == Key::Right )
			return true;
		else
			return false;
	}

	bool KeyMsg::isMovementKey() const
	{
		if( m_translatedKeyCode == Key::PageUp || m_translatedKeyCode == Key::PageDown ||
			m_translatedKeyCode == Key::Home || m_translatedKeyCode == Key::End ||
			isCursorKey() )
			return true;
		else
			return false;
	}

	//____ MouseButtonMsg _____________________________________________________

	const TypeInfo MouseButtonMsg::TYPEINFO = { "MouseButtonMsg", &InputMsg::TYPEINFO };

	const TypeInfo& MouseButtonMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ FocusGainedMsg ________________________________________________________

	const TypeInfo FocusGainedMsg::TYPEINFO = { "FocusGainedMsg", &InputMsg::TYPEINFO };

	FocusGainedMsg::FocusGainedMsg( char inputId, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp) : InputMsg(inputId, modKeys, pointerPos, pointerPosSPX, timestamp)
	{
		m_type = MsgType::FocusGained;
		m_pSource	= pWidget;
		m_pCopyTo 	= pWidget;
	}

	const TypeInfo& FocusGainedMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ FocusLostMsg __________________________________________________________

	const TypeInfo FocusLostMsg::TYPEINFO = { "FocusLostMsg", &InputMsg::TYPEINFO };

	FocusLostMsg::FocusLostMsg( char inputId, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp) : InputMsg(inputId, modKeys, pointerPos, pointerPosSPX, timestamp)
	{
		m_type = MsgType::FocusLost;
		m_pSource	= pWidget;
		m_pCopyTo 	= pWidget;
	}

	const TypeInfo& FocusLostMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ MouseEnterMsg _______________________________________________________

	const TypeInfo MouseEnterMsg::TYPEINFO = { "MouseEnterMsg", &InputMsg::TYPEINFO };

	MouseEnterMsg::MouseEnterMsg( char inputId, Object * pObject, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : InputMsg(inputId, modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseEnter;
		m_pSource	= pObject;
//		m_pCopyTo 	= pWidget;
	}

	const TypeInfo& MouseEnterMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ MouseLeaveMsg ________________________________________________________

	const TypeInfo MouseLeaveMsg::TYPEINFO = { "MouseLeaveMsg", &InputMsg::TYPEINFO };

	MouseLeaveMsg::MouseLeaveMsg( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : InputMsg(inputId, modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseLeave;
		m_pSource		= pSource;
	}

	const TypeInfo& MouseLeaveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ MouseMoveMsg __________________________________________________________

	const TypeInfo MouseMoveMsg::TYPEINFO = { "MouseMoveMsg", &InputMsg::TYPEINFO };

	MouseMoveMsg::MouseMoveMsg( char inputId, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : InputMsg(inputId, modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseMove;
		m_pSource		= pSource;
	}

	const TypeInfo& MouseMoveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ MousePressMsg ______________________________________________________

	const TypeInfo MousePressMsg::TYPEINFO = { "MousePressMsg", &MouseButtonMsg::TYPEINFO };

	MousePressMsg::MousePressMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type			= MsgType::MousePress;
		m_pSource		= pSource;
	}

	const TypeInfo& MousePressMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ MouseRepeatMsg ______________________________________________________

	const TypeInfo MouseRepeatMsg::TYPEINFO = { "MouseRepeatMsg", &MouseButtonMsg::TYPEINFO };

	MouseRepeatMsg::MouseRepeatMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type			= MsgType::MouseRepeat;
		m_pSource		= pSource;
	}

	const TypeInfo& MouseRepeatMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ MouseReleaseMsg _____________________________________________________

	const TypeInfo MouseReleaseMsg::TYPEINFO = { "MouseReleaseMsg", &MouseButtonMsg::TYPEINFO };


	MouseReleaseMsg::MouseReleaseMsg( char inputId, MouseButton button, Object * pSource, bool bReleaseInside, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type			= MsgType::MouseRelease;
		m_pSource		= pSource;

		m_bReleaseInside = bReleaseInside;
	}

	const TypeInfo& MouseReleaseMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	bool MouseReleaseMsg::releaseInside() const
	{
		return m_bReleaseInside;
	}



	//____ MouseClickMsg _____________________________________________________

	const TypeInfo MouseClickMsg::TYPEINFO = { "MouseClickMsg", &MouseButtonMsg::TYPEINFO };

	MouseClickMsg::MouseClickMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp, int duration, int ordinal ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseClick;
		m_pSource		= pSource;
		m_duration		= duration;
		m_ordinal		= ordinal;
	}

	const TypeInfo& MouseClickMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ MouseDoubleClickMsg _____________________________________________________

	const TypeInfo MouseDoubleClickMsg::TYPEINFO = { "MouseDoubleClickMsg", &MouseButtonMsg::TYPEINFO };

	MouseDoubleClickMsg::MouseDoubleClickMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseDoubleClick;
		m_pSource		= pSource;
	}

	const TypeInfo& MouseDoubleClickMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ MouseTripleClickMsg _____________________________________________________

	const TypeInfo MouseTripleClickMsg::TYPEINFO = { "MouseTripleClickMsg", &MouseButtonMsg::TYPEINFO };

	MouseTripleClickMsg::MouseTripleClickMsg( char inputId, MouseButton button, Object * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId,button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type = MsgType::MouseTripleClick;
		m_pSource		= pSource;
	}

	const TypeInfo& MouseTripleClickMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ KeyPressMsg ___________________________________________________________

	const TypeInfo KeyPressMsg::TYPEINFO = { "KeyPressMsg", &KeyMsg::TYPEINFO };

	KeyPressMsg::KeyPressMsg( char inputId, int nativeKeyCode, Key translatedKeyCode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : KeyMsg(inputId,nativeKeyCode,translatedKeyCode,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type 			= MsgType::KeyPress;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}

	const TypeInfo& KeyPressMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ KeyRepeatMsg ___________________________________________________________

	const TypeInfo KeyRepeatMsg::TYPEINFO = { "KeyRepeatMsg", &KeyMsg::TYPEINFO };

	KeyRepeatMsg::KeyRepeatMsg( char inputId, int native_keycode, Key translated_keycode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : KeyMsg(inputId,native_keycode, translated_keycode,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type 			= MsgType::KeyRepeat;
		m_pSource		= pWidget;
		m_pCopyTo 		= pWidget;
	}

	const TypeInfo& KeyRepeatMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ KeyReleaseMsg ________________________________________________________

	const TypeInfo KeyReleaseMsg::TYPEINFO = { "KeyReleaseMsg", &KeyMsg::TYPEINFO };

	KeyReleaseMsg::KeyReleaseMsg( char inputId, int native_keycode, Key translated_keycode, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : KeyMsg(inputId,native_keycode,translated_keycode,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type 			= MsgType::KeyRelease;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}

	const TypeInfo& KeyReleaseMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ TextInputMsg __________________________________________________________

	const TypeInfo TextInputMsg::TYPEINFO = { "TextInputMsg", &Msg::TYPEINFO };


	TextInputMsg::TextInputMsg( char inputId, String text, Widget * pWidget )
	{
		m_type			= MsgType::TextInput;
		m_text			= text;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
		m_inputId		= inputId;
	}

	String TextInputMsg::text() const
	{
		return m_text;
	}

	const TypeInfo& TextInputMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ EditCommandMsg __________________________________________________________

	const TypeInfo EditCommandMsg::TYPEINFO = { "EditCommandMsg", &Msg::TYPEINFO };


	EditCommandMsg::EditCommandMsg( char inputId, EditCmd command, Widget * pWidget )
	{
		m_type			= MsgType::EditCommand;
		m_command		= command;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
		m_inputId		= inputId;
	}

	const TypeInfo& EditCommandMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ WheelRollMsg __________________________________________________________

	const TypeInfo WheelRollMsg::TYPEINFO = { "WheelRollMsg", &InputMsg::TYPEINFO };

	WheelRollMsg::WheelRollMsg( char inputId, int wheel, pts distance, bool bInvertScroll, Widget * pWidget, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : InputMsg( inputId, modKeys, pointerPos, pointerPosSPX, timestamp )
	{
		m_type			= MsgType::WheelRoll;
		m_wheel			= wheel;
		m_distance		= distance;
		m_bInvertScroll = bInvertScroll;
		m_pSource		= pWidget;
		m_pCopyTo		= pWidget;
	}

	int WheelRollMsg::wheel() const
	{
		return m_wheel;
	}

	pts WheelRollMsg::distance() const
	{
		return m_distance;
	}

	bool WheelRollMsg::invertScroll() const
	{
		return m_bInvertScroll;
	}

	const TypeInfo& WheelRollMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PointerChangeMsg _______________________________________________________________

	const TypeInfo PointerChangeMsg::TYPEINFO = { "PointerChangeMsg", &Msg::TYPEINFO };

	PointerChangeMsg::PointerChangeMsg( char inputId, PointerStyle style )
	{
		m_type = MsgType::PointerChange;
		m_style = style;
		m_inputId = inputId;
	}

	PointerStyle PointerChangeMsg::style() const
	{
		return m_style;
	}

	const TypeInfo& PointerChangeMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PickMsg ___________________________________________________

	const TypeInfo PickMsg::TYPEINFO = { "PickMsg", &DropMsg::TYPEINFO };

	PickMsg::PickMsg( Widget * pSource, Coord pickOfs, Widget * pFinalReceiver, ModKeys modKeys, Coord pointerPos ) :
	m_dragWidgetPointerOfs(Coord()-pickOfs),
	m_pickOfs(pickOfs)
	{
		m_type = MsgType::Pick;
		m_pSource = pSource;
		m_pCopyTo = pSource;
		m_pFinalRecipient = pFinalReceiver;
		m_modKeys = modKeys;
		m_pointerPos = pointerPos;
	}

	const TypeInfo& PickMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void PickMsg::setContent( DropType type, int category, BasicDataset * pDataset )
	{
		m_dropType = type;
		m_category = category;
		m_pDataset = pDataset;
	}

	void PickMsg::setHotspot(Placement hotspot)
	{
		m_hotspot = hotspot;
	}

	void PickMsg::setDragWidget( Widget * pWidget, Coord pointerOfs )
	{
		m_pDragWidget = pWidget;
		m_dragWidgetPointerOfs = pointerOfs;
	}

	Widget_p PickMsg::dragWidget() const
	{
		return m_pDragWidget;
	}

	Coord PickMsg::dragWidgetPointerOfs() const
	{
		return m_dragWidgetPointerOfs;
	}

	bool PickMsg::hasDataset() const
	{
		return m_pDataset;
	}

	BasicDataset_p PickMsg::dataset() const
	{
		return m_pDataset;
	};

	//____ PickedEnterMsg _______________________________________________

	const TypeInfo PickedEnterMsg::TYPEINFO = { "PickedEnterMsg", &Msg::TYPEINFO };

	PickedEnterMsg::PickedEnterMsg(Widget* pSource, Widget* pTarget)
	{
		m_type = MsgType::PickedEnter;
		m_pSource = pSource;
		m_pCopyTo = pSource;
		m_pTarget = pTarget;
	}

	const TypeInfo& PickedEnterMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	Widget_p PickedEnterMsg::target() const
	{
		return m_pTarget;
	}

	//____ PickedLeaveMsg _______________________________________________

	const TypeInfo PickedLeaveMsg::TYPEINFO = { "PickedLeaveMsg", &Msg::TYPEINFO };

	PickedLeaveMsg::PickedLeaveMsg(Widget* pSource, Widget* pTarget)
	{
		m_type = MsgType::PickedLeave;
		m_pSource = pSource;
		m_pCopyTo = pSource;
		m_pTarget = pTarget;
	}

	const TypeInfo& PickedLeaveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	Widget_p PickedLeaveMsg::target() const
	{
		return m_pTarget;
	}

	//____ DropMsg _______________________________________________________

	const TypeInfo DropMsg::TYPEINFO = { "DropMsg", &Msg::TYPEINFO };

	DropMsg::DropMsg( MsgType type, Widget * pSource, DropType dropType, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos )
	{
		m_type = type;
		m_pSource = pSource;
		m_pCopyTo = pSource;
		m_pFinalRecipient = pFinalReceiver;
		m_modKeys = modKeys;
		m_dropPos = dropPos;
		m_pPickedFrom = pPickedFrom;

		m_dropType = dropType;
		m_category = category;
		m_pDataset = pDataset;
	}

	const TypeInfo& DropMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	bool DropMsg::hasDataset() const
	{
		return m_pDataset;
	}

	BasicDataset_p DropMsg::dataset() const
	{
		return m_pDataset;
	};

	//____ DropProbeMsg ___________________________________________________

	const TypeInfo DropProbeMsg::TYPEINFO = { "DropProbeMsg", &DropMsg::TYPEINFO };

	DropProbeMsg::DropProbeMsg(Widget * pSource, DropType type, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos)
		: DropMsg( MsgType::DropProbe, pSource, type, category, pDataset, pPickedFrom, pFinalReceiver, modKeys, dropPos)
	{
	}

	const TypeInfo& DropProbeMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void DropProbeMsg::accept(bool bAccepted)
	{
		m_bAccepted = bAccepted;
	}

	//____ DropEnterMsg ___________________________________________________

	const TypeInfo DropEnterMsg::TYPEINFO = { "DropEnterMsg", &DropMsg::TYPEINFO };

	DropEnterMsg::DropEnterMsg(Widget * pSource, DropType type, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pDragWidget,
							   Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos)
		: DropMsg( MsgType::DropEnter, pSource, type, category, pDataset, pPickedFrom, pFinalReceiver, modKeys, dropPos),
		m_pDragWidget(pDragWidget)
	{
	}

	const TypeInfo& DropEnterMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void DropEnterMsg::setDragWidget(Widget * pWidget)
	{
		m_pDragWidget = pWidget;
	}

	Widget_p DropEnterMsg::dragWidget() const
	{
		return m_pDragWidget;
	}


	//____ DropMoveMsg ___________________________________________________

	const TypeInfo DropMoveMsg::TYPEINFO = { "DropMoveMsg", &DropMsg::TYPEINFO };

	DropMoveMsg::DropMoveMsg(Widget * pSource, DropType type, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pDragWidget,
							 Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos)
		: DropMsg( MsgType::DropMove, pSource, type, category, pDataset, pPickedFrom, pFinalReceiver, modKeys, dropPos),
		m_pDragWidget(pDragWidget)
	{
	}

	const TypeInfo& DropMoveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	void DropMoveMsg::setDragWidget(Widget * pWidget)
	{
		m_pDragWidget = pWidget;
	}

	Widget_p DropMoveMsg::dragWidget() const
	{
		return m_pDragWidget;
	}

	//____ DropLeaveMsg ___________________________________________________

	const TypeInfo DropLeaveMsg::TYPEINFO = { "DropLeaveMsg", &DropMsg::TYPEINFO };

	DropLeaveMsg::DropLeaveMsg(Widget * pSource, DropType type, int category, BasicDataset * pDataset, Widget * pPickedFrom, ModKeys modKeys, Coord dropPos)
		: DropMsg( MsgType::DropLeave, pSource, type, category, pDataset, pPickedFrom, nullptr, modKeys, dropPos)
	{
	}

	const TypeInfo& DropLeaveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ DropDeliverMsg ___________________________________________________

	const TypeInfo DropDeliverMsg::TYPEINFO = { "DropDeliverMsg", &DropMsg::TYPEINFO };

	DropDeliverMsg::DropDeliverMsg(Widget * pSource, DropType type, int category, BasicDataset * pDataset, Widget * pPickedFrom, Widget * pFinalReceiver, ModKeys modKeys, Coord dropPos)
		: DropMsg( MsgType::DropDeliver, pSource, type, category, pDataset, pPickedFrom, pFinalReceiver, modKeys, dropPos),
		m_bAccepted(false)
	{
	}

	const TypeInfo& DropDeliverMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	Widget_p DropDeliverMsg::deliveredTo() const
	{
		return static_cast<Widget*>(m_pSource.rawPtr());
	}

	void DropDeliverMsg::accept(bool bAccept)
	{
		m_bAccepted = bAccept;
	}

	//____ PickedCancelMsg ___________________________________________________

	const TypeInfo PickedCancelMsg::TYPEINFO = { "PickedCancelMsg", &Msg::TYPEINFO };

	PickedCancelMsg::PickedCancelMsg(Widget * pPickedFrom)
	{
		m_type = MsgType::PickedCancel;
		m_pSource = pPickedFrom;
		m_pCopyTo = pPickedFrom;
	}

	const TypeInfo& PickedCancelMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PickedDeliverMsg ___________________________________________________

	const TypeInfo PickedDeliverMsg::TYPEINFO = { "PickedDeliverMsg", &Msg::TYPEINFO };

	PickedDeliverMsg::PickedDeliverMsg(Widget * pPicked, Widget * pDeliveree)
	{
		m_type = MsgType::PickedDeliver;
		m_pSource = pPicked;
		m_pCopyTo = pPicked;
		m_pDeliveree = pDeliveree;
	}

	const TypeInfo& PickedDeliverMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	Widget_p PickedDeliverMsg::deliveredTo() const
	{
		return m_pDeliveree;
	}

	//____ SelectMsg _________________________________________________________

	const TypeInfo SelectMsg::TYPEINFO = { "SelectMsg", &Msg::TYPEINFO };

	SelectMsg::SelectMsg(Object* pSource)
	{
		m_type = MsgType::Select;
		m_pSource = pSource;
	}

	const TypeInfo& SelectMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}
	
	//____ ToggleMsg ______________________________________________

	const TypeInfo ToggleMsg::TYPEINFO = { "ToggleMsg", &Msg::TYPEINFO };

	ToggleMsg::ToggleMsg( Object * pSource, bool bChecked )
	{
		m_type = MsgType::Toggle;
		m_pSource = pSource;
		m_bChecked = bChecked;
	}

	bool ToggleMsg::isChecked() const
	{
		return m_bChecked;
	}

	const TypeInfo& ToggleMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ScrollbarMoveMsg ___________________________________________

	const TypeInfo ScrollbarMoveMsg::TYPEINFO = { "ScrollbarMoveMsg", &Msg::TYPEINFO };

	ScrollbarMoveMsg::ScrollbarMoveMsg(Object* pSource, pts viewPos, float fracPos)
	{
		m_type = MsgType::ScrollbarMove;
		m_pSource = pSource;
		m_viewPos = viewPos;
		m_fracPos = fracPos;
	}

	pts ScrollbarMoveMsg::viewPos() const
	{
		return m_viewPos;
	}

	float ScrollbarMoveMsg::fracPos() const
	{
		return m_fracPos;
	}


	const TypeInfo& ScrollbarMoveMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ValueUpdateMsg ___________________________________________

	const TypeInfo ValueUpdateMsg::TYPEINFO = { "ValueUpdateMsg", &Msg::TYPEINFO };

	ValueUpdateMsg::ValueUpdateMsg( Object * pSource, float value, bool bFinal )
	{
		m_type = MsgType::ValueUpdate;
		m_pSource = pSource;
		m_value = value;
		m_bFinal = bFinal;
	}

	float ValueUpdateMsg::value() const
	{
		return m_value;
	}

	bool ValueUpdateMsg::isFinal() const
	{
		return m_bFinal;
	}

	const TypeInfo& ValueUpdateMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ RangeUpdateMsg ___________________________________________

	const TypeInfo RangeUpdateMsg::TYPEINFO = { "RangeUpdateMsg", &Msg::TYPEINFO };

	RangeUpdateMsg::RangeUpdateMsg( Object * pSource, int intOfs, int intLength, double fracOfs, double fracLength, bool bFinal )
	{
		m_type = MsgType::RangeUpdate;
		m_pSource 	= pSource;
		m_ofs		= intOfs;
		m_length	= intLength;
		m_fracOfs	= fracOfs;
		m_fracLength= fracLength;
		m_bFinal	= bFinal;
	}

	const TypeInfo& RangeUpdateMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	pts RangeUpdateMsg::offset() const
	{
		return m_ofs;
	}

	pts RangeUpdateMsg::length() const
	{
		return m_length;
	}

	double RangeUpdateMsg::fracOffset() const
	{
		return m_fracOfs;
	}

	double RangeUpdateMsg::fracLength() const
	{
		return m_fracLength;
	}

	bool RangeUpdateMsg::isFinal() const
	{
		return m_bFinal;
	}


	//____ TextEditMsg ________________________________________________________

	const TypeInfo TextEditMsg::TYPEINFO = { "TextEditMsg", &Msg::TYPEINFO };

	EditableText_p TextEditMsg::text() const
	{
		return EditableText_p(m_pSource,*m_pText);
	}

	TextEditMsg::TextEditMsg( EditableText * pText, int offset, int deleted, int inserted )
	{
		m_type 		= MsgType::TextEdit;
		m_pSource 	= pText->_widget();
		m_pText 	= pText;
		m_offset	= offset;
		m_nbDeleted = deleted;
		m_nbInserted = inserted;
	}

	const TypeInfo& TextEditMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ItemMsg ________________________________________________________

	const TypeInfo ItemMsg::TYPEINFO = { "ItemMsg", &Msg::TYPEINFO };

	ItemMsg::ItemMsg( Object * pSource, int itemId, Object * pItemObject ) : m_item(itemId,pItemObject)
	{
		m_pSource 	= pSource;
	}

	const TypeInfo& ItemMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	int ItemMsg::itemId() const
	{
		return m_item.id;
	}

	Object_p ItemMsg::itemObject() const
	{
		return m_item.pObject;
	}

	//____ ItemToggleMsg ________________________________________________________

	const TypeInfo ItemToggleMsg::TYPEINFO = { "ItemToggleMsg", &ItemMsg::TYPEINFO };

	ItemToggleMsg::ItemToggleMsg( Object * pSource, int itemId, Object * pItemObject, bool bSet ) : ItemMsg(pSource,itemId,pItemObject)
	{
		m_type = MsgType::ItemToggle;
		m_bSet = bSet;
	}

	const TypeInfo& ItemToggleMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	bool ItemToggleMsg::isSet() const
	{
		return m_bSet;
	}

	//____ ItemMousePressMsg ________________________________________________________

	const TypeInfo ItemMousePressMsg::TYPEINFO = { "ItemMousePressMsg", &ItemMsg::TYPEINFO };

	ItemMousePressMsg::ItemMousePressMsg( Object * pSource, int itemId, Object * pItemObject, MouseButton button ) : ItemMsg(pSource,itemId,pItemObject)
	{
		m_type = MsgType::ItemMousePress;
		m_button = button;
	}

	const TypeInfo& ItemMousePressMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	MouseButton ItemMousePressMsg::button() const
	{
		return m_button;
	}

	//____ ItemListMsg ________________________________________________________

	const TypeInfo ItemListMsg::TYPEINFO = { "ItemListMsg", &Msg::TYPEINFO };

	ItemListMsg::ItemListMsg( Object * pSource, int nbItems, ItemInfo * pItems )
	{
		m_pSource 	= pSource;
		m_nbItems	= nbItems;
		m_pItems	= pItems;

		assert(pItems != nullptr);

	}

	ItemListMsg::~ItemListMsg()
	{
		delete [] m_pItems;
	}

	const TypeInfo& ItemListMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	int ItemListMsg::nbItems() const
	{
		return m_nbItems;
	}

	const ItemInfo * ItemListMsg::items() const
	{
		return m_pItems;
	}

	//____ ItemsSelectMsg ________________________________________________________

	const TypeInfo ItemsSelectMsg::TYPEINFO = { "ItemsSelectMsg", &ItemListMsg::TYPEINFO };

	ItemsSelectMsg::ItemsSelectMsg( Object * pSource, int nbItems, ItemInfo * pItems ) : ItemListMsg(pSource,nbItems,pItems)
	{
		m_type = MsgType::ItemsSelect;
	}

	const TypeInfo& ItemsSelectMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ItemsUnselectMsg ________________________________________________________

	const TypeInfo ItemsUnselectMsg::TYPEINFO = { "ItemsUnselectMsg", &ItemListMsg::TYPEINFO };

	ItemsUnselectMsg::ItemsUnselectMsg( Object * pSource, int nbItems, ItemInfo * pItems ) : ItemListMsg(pSource,nbItems,pItems)
	{
		m_type = MsgType::ItemsUnselect;
	}

	const TypeInfo& ItemsUnselectMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PopupSelectMsg _________________________________________________________

	const TypeInfo PopupSelectMsg::TYPEINFO = { "PopupSelectMsg", &Msg::TYPEINFO };

	PopupSelectMsg::PopupSelectMsg( Object * pSource, Widget * pSelected )
	{
		m_type		= MsgType::PopupSelect;
		m_pSource	= pSource;
		m_pSelected	= pSelected;
	}

	const TypeInfo& PopupSelectMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PopupOpenedMsg ____________________________________________

	const TypeInfo PopupOpenedMsg::TYPEINFO = { "PopupOpenedMsg", &Msg::TYPEINFO };

	PopupOpenedMsg::PopupOpenedMsg( Widget * pPopup, const Widget_wp& pOpener )
	{
		m_type			= MsgType::PopupOpened;
		m_pCopyTo		= pOpener.rawPtr();
		m_pSource		= pPopup;
		m_pOpener		= pOpener;
	}

	const TypeInfo& PopupOpenedMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PopupClosedMsg ____________________________________________

	const TypeInfo PopupClosedMsg::TYPEINFO = { "PopupClosedMsg", &Msg::TYPEINFO };

	PopupClosedMsg::PopupClosedMsg( Widget * pPopup, const Widget_wp& pCaller )
	{
		m_type			= MsgType::PopupClosed;
		m_pCopyTo		= pCaller.rawPtr();
		m_pSource		= pPopup;
	}

	const TypeInfo& PopupClosedMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ ModalMoveOutsideMsg ___________________________________________________

	const TypeInfo ModalMoveOutsideMsg::TYPEINFO = { "ModalMoveOutsideMsg", &InputMsg::TYPEINFO };

	ModalMoveOutsideMsg::ModalMoveOutsideMsg( char inputId, Widget * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : InputMsg(inputId, modKeys, pointerPos, pointerPosSPX, timestamp)
	{
		m_type = MsgType::ModalMoveOutside;
		m_pSource		= pSource;
	}

	const TypeInfo& ModalMoveOutsideMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ ModalBlockedPressMsg _______________________________________________

	const TypeInfo ModalBlockedPressMsg::TYPEINFO = { "ModalBlockedPressMsg", &InputMsg::TYPEINFO };

	ModalBlockedPressMsg::ModalBlockedPressMsg( char inputId, MouseButton button, Widget * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId, button, modKeys, pointerPos, pointerPosSPX, timestamp )
	{
		m_type			= MsgType::ModalBlockedPress;
		m_pSource 		= pSource;
	}

	const TypeInfo& ModalBlockedPressMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ ModalBlockedReleaseMsg _____________________________________________

	const TypeInfo ModalBlockedReleaseMsg::TYPEINFO = { "ModalBlockedReleaseMsg", &InputMsg::TYPEINFO };

	ModalBlockedReleaseMsg::ModalBlockedReleaseMsg( char inputId, MouseButton button, Widget * pSource, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId, button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type			= MsgType::ModalBlockedRelease;
		m_pSource 		= pSource;
	}

	const TypeInfo& ModalBlockedReleaseMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ PianoKeyPressMsg ___________________________________________________

	const TypeInfo PianoKeyPressMsg::TYPEINFO = { "PianoKeyPressMsg", &Msg::TYPEINFO };

	PianoKeyPressMsg::PianoKeyPressMsg(Object * pSource, int keyIdx, int64_t timestamp )
	{
		m_type = MsgType::PianoKeyPress;
		m_pSource = pSource;
		m_keyIdx = keyIdx;
		m_timestamp = timestamp;
	}

	const TypeInfo& PianoKeyPressMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ActivateMsg ___________________________________________________

	const TypeInfo ActivateMsg::TYPEINFO = { "ActivateMsg", &Msg::TYPEINFO };

	ActivateMsg::ActivateMsg(Object * pSource )
	{
		m_type = MsgType::Activate;
		m_pSource = pSource;
	}

	const TypeInfo& ActivateMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ DeactivateMsg ___________________________________________________

	const TypeInfo DeactivateMsg::TYPEINFO = { "DeactivateMsg", &Msg::TYPEINFO };

	DeactivateMsg::DeactivateMsg(Object * pSource )
	{
		m_type = MsgType::Deactivate;
		m_pSource = pSource;
	}

	const TypeInfo& DeactivateMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ PingMsg ___________________________________________________

	const TypeInfo PingMsg::TYPEINFO = { "PingMsg", &Msg::TYPEINFO };

	PingMsg::PingMsg(Object * pSource )
	{
		m_type = MsgType::Ping;
		m_pSource = pSource;
	}

	const TypeInfo& PingMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ PianoKeyReleaseMsg _________________________________________________

	const TypeInfo PianoKeyReleaseMsg::TYPEINFO = { "PianoKeyReleaseMsg", &Msg::TYPEINFO };

	PianoKeyReleaseMsg::PianoKeyReleaseMsg(Object * pSource, int keyIdx, int64_t timestamp)
	{
		m_type = MsgType::PianoKeyRelease;
		m_pSource = pSource;
		m_keyIdx = keyIdx;
		m_timestamp = timestamp;
	}

	const TypeInfo& PianoKeyReleaseMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}


	//____ MouseDragMsg _________________________________________________________

	const TypeInfo MouseDragMsg::TYPEINFO = { "MouseDragMsg", &MouseButtonMsg::TYPEINFO };


	MouseDragMsg::MouseDragMsg( char inputId, MouseButton button, Object * pSource, const Coord& startPos, const Coord& prevPos, const CoordSPX& startPosSPX, const CoordSPX& prevPosSPX, ModKeys modKeys, Coord pointerPos, CoordSPX pointerPosSPX, int64_t timestamp ) : MouseButtonMsg(inputId, button,modKeys,pointerPos,pointerPosSPX,timestamp)
	{
		m_type 			= MsgType::MouseDrag;
		m_pSource		= pSource;

		m_startPos = startPos;
		m_prevPos = prevPos;

		m_startPosSPX = startPosSPX;
		m_prevPosSPX = prevPosSPX;
	}

	const TypeInfo& MouseDragMsg::typeInfo(void) const
	{
		return TYPEINFO;
	}

	Coord MouseDragMsg::draggedTotal() const
	{
		return m_pointerPos - m_startPos;
	}

	Coord MouseDragMsg::draggedNow() const
	{
		return m_pointerPos - m_prevPos;
	}

	Coord MouseDragMsg::currPos() const
	{
		return m_pointerPos;
	}

	Coord MouseDragMsg::startPos() const
	{
		return m_startPos;
	}

	Coord MouseDragMsg::prevPos() const
	{
		return m_prevPos;
	}

	CoordSPX MouseDragMsg::_draggedTotal() const
	{
		return m_pointerPosSPX - m_startPosSPX;
	}

	CoordSPX MouseDragMsg::_draggedNow() const
	{
		return m_pointerPosSPX - m_prevPosSPX;
	}

	CoordSPX MouseDragMsg::_currPos() const
	{
		return m_pointerPosSPX;
	}

	CoordSPX MouseDragMsg::_startPos() const
	{
		return m_startPosSPX;
	}

	CoordSPX MouseDragMsg::_prevPos() const
	{
		return m_prevPosSPX;
	}

} // namespace wg
