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
#ifndef WG_EVENT_DOT_H
#define WG_EVENT_DOT_H

#include <string>

#ifndef WG2_TYPES_DOT_H
#	include <wg2_types.h>
#endif

#ifndef WG2_USERDEFINES_DOT_H
#	include <wg2_userdefines.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#ifndef WG2_SMARTPTR_DOT_H
#	include <wg2_smartptr.h>
#endif

#include <wg_dataset.h>

class WgEventHandler;
class WgWidget;
class WgButton;
class WgCheckBox;
class WgRadioButton;
class WgAnimPlayer;
class WgWidgetSlider;
class WgMenu;
class WgPopupLayer;
class WgModalLayer;
class WgInterfaceEditText;
class WgText;
class WgMultiSlider;
class WgDragNDropLayer;
class WgPianoKeyboard;

typedef class WgWeakPtr<WgWidget> WgWidgetWeakPtr;

namespace WgEvent
{
	class Event
	{
		friend class ::WgEventHandler;
		friend class ::WgWidget;
		friend class Eavesdrop;

		public:
			WgEventType		Type() const { return m_type; }
			int64_t			Timestamp() const { return m_timestamp; }
			bool			IsForWidget() const { return m_bIsForWidget; }
			WgWidget *		Widget() const;									// Inlining this would demand include of wg_widget.h.
			WgWidgetWeakPtr	WidgetWeakPtr() const { return m_pWidget; }
			WgWidget *		ForwardedFrom() const;
			WgModifierKeys	ModKeys() const { return m_modKeys; }
			WgCoord			PointerPixelPos() const { return m_pointerLocalPixelPos; }
			WgCoord			PointerScreenPixelPos() const { return m_pointerScreenPixelPos; }

			WgCoord			PointerPointPos() const { return (m_pointerLocalPixelPos * WG_SCALE_BASE) / m_pointScale; }
			WgCoord			PointerScreenPointPos() const { return (m_pointerScreenPixelPos * WG_SCALE_BASE) / m_pointScale; }

			bool			IsMouseEvent() const;
			bool			IsMouseButtonEvent() const;
			bool			IsKeyEvent() const;

		protected:
			Event() : m_type(WG_EVENT_DUMMY), m_modKeys(WG_MODKEY_NONE), m_timestamp(0), m_bIsForWidget(false) {}
			virtual ~Event() {}

			virtual void 	_cloneContentFrom( const Event * pOrg );			// Only subclassed for the standard event types.

			WgEventType		m_type;				// Type of event
			WgModifierKeys	m_modKeys;			// Modifier keys pressed when event posted.
			int64_t			m_timestamp;		// Timestamp of posting this event in milliseconds.
			bool			m_bIsForWidget;		// Set if this event is for a specific Widget (m_pWidget set at creation, even if weak pointer now is null).
			WgWidgetWeakPtr	m_pWidget;			// Widget to receive this event.
			WgWidgetWeakPtr	m_pForwardedFrom;	// Widget this event was forwarded from.
			WgCoord			m_pointerLocalPixelPos;	// Widget-relative position of pointer. Same as m_pointerScreenPixelPos if Widget not set.
			WgCoord			m_pointerScreenPixelPos;	// Screen position of pointer.
			int             m_pointScale;
			WgWidgetWeakPtr m_pFinalRecipient;

			WgWidgetWeakPtr m_pForwardTo;
	};

	class MouseButtonEvent : public Event
	{
		friend class ::WgEventHandler;
	public:
		int		Button() const { return m_button; }
	protected:
		MouseButtonEvent(int button) : m_button(button) {}
		virtual void 	_cloneContentFrom( const Event * pOrg );

		int		m_button;
	};

	class KeyEvent : public Event
	{
		friend class ::WgEventHandler;
	public:
		int		NativeKeyCode() const { return m_nativeKeyCode; }
		int		TranslatedKeyCode() const { return m_translatedKeyCode; }
		bool	IsCursorKey() const;
		bool	IsMovementKey() const;
	protected:
		KeyEvent( int nativeKeyCode ) : m_nativeKeyCode(nativeKeyCode), m_translatedKeyCode(0) {}
		virtual void 	_cloneContentFrom( const Event * pOrg );

		int		m_nativeKeyCode;
		int		m_translatedKeyCode;
	};

	class FocusGained : public Event
	{
		friend class ::WgEventHandler;
		friend class ::WgWidget;
	public:
		FocusGained();
	protected:
		FocusGained(WgWidget * pWidget);
	};

	class FocusLost : public Event
	{
		friend class ::WgEventHandler;
		friend class ::WgWidget;
	public:
		FocusLost();
	protected:
		FocusLost(WgWidget * pWidget);
	};

	class MouseEnter : public Event
	{
		friend class ::WgEventHandler;
	public:
		MouseEnter( const WgCoord& pos );
	protected:
		MouseEnter( WgWidget * pWidget );
	};

	class MouseLeave : public Event
	{
		friend class ::WgEventHandler;
	public:
		MouseLeave();
	protected:
		MouseLeave( WgWidget * pWidget );
	};


	class MouseMove : public Event
	{
		friend class ::WgEventHandler;
	protected:
		MouseMove( WgWidget * pWidget );
	public:
		MouseMove( const WgCoord& pos );
	};

	class MouseButtonPress : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	protected:
		MouseButtonPress( int button, WgWidget * pWidget );
	public:
		MouseButtonPress( int button );
	};

	class MouseButtonRelease : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	public:
		MouseButtonRelease( int button );

		bool			PressInside() const;
		bool			ReleaseInside() const;

		WgModifierKeys  PressModKeys() const { return m_pressModKeys; }

	protected:
		MouseButtonRelease( int button, WgWidget * pWidget, bool bPressInside, bool bReleaseInside );
		virtual void 	_cloneContentFrom( const Event * pOrg );

		bool			m_bPressInside;
		bool			m_bReleaseInside;
		WgModifierKeys  m_pressModKeys;     // ModKeys pressed when the button was pressed. Set by _finalizeEvent().
	};

	class KeyPress : public KeyEvent
	{
		friend class ::WgEventHandler;
	public:
		KeyPress( int native_keycode );
	protected:
		KeyPress( int native_keycode, WgWidget * pWidget );
	};

	class KeyRelease : public KeyEvent
	{
		friend class ::WgEventHandler;
	public:
		KeyRelease( int native_keycode );
	protected:
		KeyRelease( int native_keycode, WgWidget * pWidget );
	};

	class Character : public Event
	{
		friend class ::WgEventHandler;
	public:
		Character( unsigned short character );

		unsigned short	Char() const;
	protected:
		Character( unsigned short character, WgWidget * pWidget );
		virtual void 	_cloneContentFrom( const Event * pOrg );
	protected:
		unsigned short	m_char;
	};

	class MouseWheelRoll : public Event
	{
		friend class ::WgEventHandler;
	public:
		MouseWheelRoll( int wheel, int distance, bool bInvertScroll = false );

		int			Wheel() const;
		int			Distance() const;
        bool        InvertScroll() const;
	protected:
		MouseWheelRoll( int wheel, int distance, bool bInvertScroll, WgWidget * pWidget );
		virtual void 	_cloneContentFrom( const Event * pOrg );

		int			m_wheel;
		int			m_distance;
        bool        m_bInvertScroll;        // Set if direction should be inverted for ScrollPanel and ScrollBar.
                                            // Should be ignored by other widgets.
	};

	class Tick : public Event
	{
		friend class ::WgEventHandler;
	public:
		Tick( int millisec, int microsec = -1 );	// Uses microsec instead of millisec if postive value.

		int				Millisec() const;
		int				Microsec() const;

	protected:
		Tick( int microsec, WgWidget * pWidget, bool b );
		virtual void 	_cloneContentFrom( const Event * pOrg );

		int				m_microsec;
	};

	class PointerChange : public Event
	{
		friend class ::WgEventHandler;
	public:
		WgPointerStyle	Style() const;

	protected:
		PointerChange( WgPointerStyle style );
		virtual void 	_cloneContentFrom( const Event * pOrg );

		WgPointerStyle	m_style;
	};


	class Eavesdrop : public Event
	{
		friend class ::WgEventHandler;
	public:

		WgEvent::Event *    Event() const;
	protected:
		Eavesdrop( WgWidget * pEavesdropper, WgEvent::Event * pEvent );
		~Eavesdrop();
		virtual void     _cloneContentFrom( const WgEvent::Event * pOrg );

		WgEvent::Event *   m_pEvent;


	};

	//____ DragNDrop events _____________________________________________________

	class DragNDrop : public Event
	{
		friend class ::WgDragNDropLayer;
	public:
        wg::BasicDataset_p  payload() const { return m_pPayload; }
		WgWidgetWeakPtr     pickedFrom() const { return m_pPickedFrom; }
		int                 pickCategory() const { return m_pickCategory; }

	protected:
		DragNDrop( WgEventType type, WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver );
		virtual void     _cloneContentFrom( const Event * pOrg );

		wg::BasicDataset_p  m_pPayload;
		WgWidgetWeakPtr     m_pPickedFrom;
		int                 m_pickCategory;
	};

	class DropPick : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:
		void                setPayload( wg::BasicDataset * pPayload );
		bool                hasPayload() const { return m_pPayload; }

        void                setHidePointer(bool bHide) { m_bHidePointer = bHide; }
        bool                isHidingPointer() const { return m_bHidePointer; }
        
		WgCoord             pickOfs() const { return m_pickOfs; }
		bool                deleteDragWidgetWhenDone() const { return m_bDeleteWhenDone; }

		void                setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone = true );
		bool                hasDragWidget() const { return m_pDragWidget; }
		WgWidget *          dragWidget() const { return m_pDragWidget; }
		WgCoord             dragWidgetPointerOfs() const { return m_dragWidgetPointerOfs; }

	protected:
		DropPick( WgWidget * pSource, WgCoord pickOfs, WgWidget * pFinalReceiver );

		WgWidget *            m_pDragWidget;
		WgCoord               m_dragWidgetPointerOfs;
		WgCoord               m_pickOfs;
        bool                  m_bHidePointer;
		bool                  m_bDeleteWhenDone;        // Set if m_pDragWidget should be deleted when not used anymore.
	};

	class DropProbe : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

		void            accept( bool bAccept = true ) { m_bAccepted = bAccept; }
		bool            isAccepted() const { return m_bAccepted; }

	protected:
		DropProbe( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver );

		bool        m_bAccepted;
	};

	class DropEnter : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

        void                setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone = true );
		WgWidget *          dragWidget() const { return m_pDragWidget; }
        WgCoord             dragWidgetPointerOfs() const { return m_dragWidgetPointerOfs; }
        bool                deleteDragWidgetWhenDone() const { return m_bDeleteWhenDone; }

	protected:
		DropEnter( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pDragWidget, WgCoord dragWidgetPointerOfs, bool bDeleteWhenDone, WgWidget * pFinalReceiver );

		WgWidget *          m_pDragWidget;
        WgCoord             m_dragWidgetPointerOfs;
        bool                m_bDeleteWhenDone;        // Set if m_pDragWidget should be deleted when not used anymore.
	};

	class DropMove : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

        void                setDragWidget( WgWidget * pWidget, WgCoord pixelPointerOfs, bool bDeleteWhenDone = true );
		WgWidget *          dragWidget() const { return m_pDragWidget; }
        WgCoord             dragWidgetPointerOfs() const { return m_dragWidgetPointerOfs; }
        bool                deleteDragWidgetWhenDone() const { return m_bDeleteWhenDone; }

	protected:
		DropMove( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pDragWidget, WgCoord dragWidgetPointerOfs, bool bDeleteWhenDone, WgWidget * pFinalReceiver );

		WgWidget *          m_pDragWidget;
        WgCoord             m_dragWidgetPointerOfs;
        bool                m_bDeleteWhenDone;        // Set if m_pDragWidget should be deleted when not used anymore.
	};

	class DropLeave : public DragNDrop
	{
		friend class ::WgDragNDropLayer;

	protected:
		DropLeave( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom );
	};

	class DropDeliver : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

		WgWidget *      deliveredTo() const;
		void            accept( bool bAccept = true ) { m_bAccepted = bAccept; }
		bool            isAccepted() const { return m_bAccepted; }

	protected:
		DropDeliver( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom, WgWidget * pFinalReceiver );

		bool        m_bAccepted;
	};

	class DropCancel : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
    public:
        WgWidgetWeakPtr DroppedWidget() { return m_pDropped; };

    protected:
        DropCancel( WgWidget * pPickedFrom, int pickCategory, wg::BasicDataset * pPayload, WgWidgetWeakPtr pDropped);
    private:
        WgWidgetWeakPtr m_pDropped;
    };

    class DropComplete : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

		WgWidget *      deliveredTo() const { return m_pDeliveree; }

	protected:
		DropComplete( WgWidget * pPicked, WgWidget * pDeliveree, int pickCategory, wg::BasicDataset * pPayload );

		WgWidget *     m_pDeliveree;
	};


	class DropHoverEnter : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

	protected:
		DropHoverEnter( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom );
	};

	class DropHoverMove : public DragNDrop
	{
		friend class ::WgDragNDropLayer;
	public:

	protected:
		DropHoverMove( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom );
	};

	class DropHoverLeave : public DragNDrop
	{
		friend class ::WgDragNDropLayer;

	protected:
		DropHoverLeave( WgWidget * pSource, int pickCategory, wg::BasicDataset * pPayload, WgWidget * pPickedFrom );
	};

	//____ WgButton events _______________________________________________

	class ButtonPress : public Event
	{
	public:
		ButtonPress( WgButton * pWidget );
		WgButton *	Button() const;
	};

	//____ Generic events _____________________________________________________

	class Selected : public Event
	{
	public:
		Selected(WgWidget * pWidget);
	};



	//____ Item events ________________________________________________________
/*

	class ItemEvent : public Event
	{
		int	ItemId() const;

		WgRect	ItemPixelGeo() const;
		WgRect	ItemPointGeo() const;

		WgCoord	ItemPointerPixelPos() const;
		WgCoord ItemPointerPointPos() const;

	protected:
		int		m_itemId;
		WgRect	m_itemPixelGeo;
	};


	class ItemButtonEvent

	class ItemEnter : public ItemEvent
	{
	public:
		ItemEnter(WgWidget * pWidget, int itemId);
	};


	class ItemLeave : public ItemEvent
	{
	public:
		ItemLeave(WgWidget * pWidget, int itemId);
	};

	class ItemPress : public ItemButtonEvent
	{
		friend class ::WgEventHandler;
	public:
		ItemPress(int button, WgWidget * pWidget, int itemId);
	};

	class ItemDrag : public ItemButtonEvent
	{
	public:
		ItemDrag(int button, WgWidget * pWidget, int itemId, const WgCoord& orgPos, const WgCoord& prevPos, const WgCoord& currPos);

		WgCoord			DraggedTotalPixels() const;
		WgCoord			DraggedNowPixels() const;
		WgCoord			StartPixelPos() const;
		WgCoord			PrevPixelPos() const;
		WgCoord			CurrPixelPos() const;

		WgCoord			DraggedTotalPoints() const;
		WgCoord			DraggedNowPoints() const;
		WgCoord			StartPointPos() const;
		WgCoord			PrevPointPos() const;
		WgCoord			CurrPointPos() const;
	protected:
		WgCoord			m_startPos;
		WgCoord			m_prevPos;
		WgCoord			m_currPos;
	};

	class ItemRepeat : public ItemButtonEvent
	{
	public:
		ItemRepeat(int button, WgWidget * pWidget, int itemId);
	};


	class ItemRelease : public ItemButtonEvent
	{
	public:
		ItemRelease(int button, WgWidget * pWidget, int itemId);

	};

	class ItemClick : public ItemButtonEvent
	{
	public:
		ItemClick(int button, WgWidget * pWidget, int itemId);
	};

	class ItemDoubleClick : public ItemButtonEvent
	{
	public:
		ItemDoubleClick(int button, WgWidget * pWidget, int itemId);
	};
*/


	//____ WgCheckBox events ______________________________________________

	class CheckboxEvent : public Event
	{
	public:
		WgCheckBox * Checkbox() const;
	};

	class CheckboxCheck : public CheckboxEvent
	{
	public:
		CheckboxCheck( WgCheckBox * pWidget );
	};

	class CheckboxUncheck : public CheckboxEvent
	{
	public:
		CheckboxUncheck( WgCheckBox * pWidget );
	};

	class CheckboxToggle : public CheckboxEvent
	{
	public:
		CheckboxToggle( WgCheckBox * pWidget, bool bChecked );
		bool		IsChecked() const;

	private:
		bool	m_bChecked;
	};

	//____ WgRadioButton events ___________________________________________

	class RadiobuttonEvent : public Event
	{
	public:
		WgRadioButton * Radiobutton() const;
	};

	class RadiobuttonSelect : public RadiobuttonEvent
	{
	public:
		RadiobuttonSelect( WgRadioButton * pWidget );
	};

	class RadiobuttonUnselect : public RadiobuttonEvent
	{
	public:
		RadiobuttonUnselect( WgRadioButton * pWidget );
	};

	class RadiobuttonToggle : public RadiobuttonEvent
	{
	public:
		RadiobuttonToggle( WgRadioButton * pWidget, bool bSelected );
		bool	IsSelected() const;
	private:
		bool	m_bSelected;
	};

	//____ WgAnimPlayer events _____________________________________________

	class AnimationUpdate : public Event
	{
	public:
		AnimationUpdate( WgAnimPlayer * pWidget, int frame, float fraction );
		WgAnimPlayer * Animation() const;
		int		Frame() const;
		float	Fraction() const;
	private:
		int		m_frame;
		float	m_fraction;
	};


	//____ WgWidgetSlider events ________________________________________________

	class SliderEvent : public Event
	{
	public:
		WgWidgetSlider* Slider() const;
		float			Pos() const;
		float			Length() const;
		float			Value() const;

	protected:
		SliderEvent( WgWidgetSlider * pWidget, float pos, float length );
		float			m_pos;
		float			m_length;
	};

	class SliderMove : public SliderEvent
	{
	public:
		SliderMove( WgWidgetSlider* pWidget, float pos, float length );
	};

	class SliderStepFwd : public SliderEvent
	{
	public:
		SliderStepFwd( WgWidgetSlider* pWidget, float pos, float length );
	};

	class SliderStepBwd : public SliderEvent
	{
	public:
		SliderStepBwd( WgWidgetSlider* pWidget, float pos, float length );
	};

	class SliderJumpFwd : public SliderEvent
	{
	public:
		SliderJumpFwd( WgWidgetSlider* pWidget, float pos, float length );
	};

	class SliderJumpBwd : public SliderEvent
	{
	public:
		SliderJumpBwd( WgWidgetSlider* pWidget, float pos, float length );
	};

	class SliderWheelRolled : public SliderEvent
	{
	public:
		SliderWheelRolled( WgWidgetSlider* pWidget, int distance, float pos, float length );
		int			Distance() const;
	protected:
		int			m_distance;
	};

	//____ WgWidgetKnob events _________________________________________________

	class KnobTurn : public Event
	{
	public:
		KnobTurn( WgWidget * pWidget, int pos, float fraction ); //TODO: Change to WgWidgetKnob* when we have impoted WgWidgetKnob to main code.
//		WgWidgetKnob * Knob() const;		TODO: Add when we have imported WgWidgetKnob to main code.
		float	Fraction() const;
		int		Pos() const;
	private:
		float	m_fraction;
		int		m_pos;
	};

	//____ WgRangeSlider events __________________________________________________

	class RangeChanged : public Event
	{
	public:
		RangeChanged(WgWidget* pWidget, float begin, float end);
		float	Begin() const;
		float	End() const;
	private:
		float	m_begin;
		float	m_end;
	};



	//____ Text events ________________________________________

	class TextEvent : public Event
	{
	public:
		WgInterfaceEditText * Interface() const;
		const WgText *		  Text() const;
	protected:
		WgText *	m_pText;
	};

	class TextModify : public TextEvent
	{
	public:
		TextModify( WgWidget * pWidget, WgText * pText );
	};

	class TextSet : public TextEvent
	{
	public:
		TextSet( WgWidget * pWidget, WgText * pText );
	};

	//____ WgMenu events __________________________________________________

	class MenuitemEvent : public Event
	{
	public:
		WgMenu *	Menu() const;
		int				ItemId() const;

	protected:
		int				m_itemId;
	};

	class MenuitemSelect : public MenuitemEvent
	{
	public:
		MenuitemSelect( WgMenu * pMenu, int menuItemId );
	};

	class MenuitemCheck : public MenuitemEvent
	{
	public:
		MenuitemCheck( WgMenu * pMenu, int menuItemId );
	};

	class MenuitemUncheck : public MenuitemEvent
	{
	public:
		MenuitemUncheck( WgMenu * pMenu, int menuItemId );
	};

	//____ WgPopupLayer events __________________________________________________

	class PopupClosed : public Event
	{
		friend class ::WgPopupLayer;
	public:
		WgWidget *		Popup() const;									// Inlining this would demand include of wg_widget.h.
		WgWidgetWeakPtr	PopupWeakPtr() const { return m_pPopup; }

		// Caller is the same as m_pWidget, since m_pWidget should receive
		// the event.

		WgWidget *		Caller() const;									// Inlining this would demand include of wg_widget.h.
		WgWidgetWeakPtr	CallerWeakPtr() const { return m_pWidget; }

	protected:
		PopupClosed( WgWidget * pMenu, const WgWidgetWeakPtr& pCaller );

		WgWidgetWeakPtr m_pPopup;
	};


	//____ WgWidgetModalLayer events _________________________________________________

	class ModalMoveOutside : public Event
	{
		friend class ::WgModalLayer;
	protected:
		ModalMoveOutside( WgWidget * pWidget );
	};

	class ModalBlockedPress : public MouseButtonEvent
	{
		friend class ::WgModalLayer;
	protected:
		ModalBlockedPress( int button, WgWidget * pModalWidget );
	};

	class ModalBlockedRelease : public MouseButtonEvent
	{
		friend class ::WgModalLayer;
	protected:
		ModalBlockedRelease( int button, WgWidget * pModalWidget );
	};

	//____ WgMultiSlider events _______________________________________________

	class SliderEnter : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

	protected:
		SliderEnter(WgMultiSlider * pSlider, int sliderId );

		int        m_id;
	};

	class SliderLeave : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

	protected:
		SliderLeave(WgMultiSlider * pSlider, int sliderId );

		int        m_id;
	};

	class SliderHandleEnter : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

	protected:
		SliderHandleEnter(WgMultiSlider * pSlider, int sliderId );

		int        m_id;
	};

	class SliderHandleLeave : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

	protected:
		SliderHandleLeave(WgMultiSlider * pSlider, int sliderId );

		int        m_id;
	};

	class SliderPressed : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

		int        button() const { return m_button; }
		WgOrigo    sideOfHandle() const { return m_offsetFromHandle; }

	protected:
		SliderPressed(WgMultiSlider * pSlider, int sliderId, int button, WgOrigo offsetFromHandle );

		int        m_id;
		int        m_button;
		WgOrigo    m_offsetFromHandle;
	};

	class SliderMoved : public Event
	{
		friend class ::WgMultiSlider;
	public:
		int id() const { return m_id; }

		float value() { return m_value; }
		float value2() { return m_value2; }

	protected:
		SliderMoved(WgMultiSlider * pSlider, int sliderId, float value, float value2 = NAN);

		int		m_id;
		float	m_value;
		float	m_value2;
	};


	//____ Link events _________________________________________________________

	class LinkEvent : public Event
	{
	public:
		std::string		Link() const;
	protected:
		std::string		m_link;
	};

	class LinkMark : public LinkEvent
	{
	public:
		LinkMark( WgWidget * pWidget, std::string link );
	};

	class LinkUnmark : public LinkEvent
	{
	public:
		LinkUnmark( WgWidget * pWidget, std::string link );
	};

	class LinkButtonEvent : public LinkEvent
	{
	public:
		int			Button() const;
	protected:
		int			m_button;
	};

	class LinkPress : public LinkButtonEvent
	{
	public:
		LinkPress( WgWidget * pWidget, std::string link, int button );
	};

	class LinkRepeat : public LinkButtonEvent
	{
	public:
		LinkRepeat( WgWidget * pWidget, std::string link, int button );
	};

	class LinkRelease : public LinkButtonEvent
	{
	public:
		LinkRelease( WgWidget * pWidget, std::string link, int button );
	};

	class LinkClick : public LinkButtonEvent
	{
	public:
		LinkClick( WgWidget * pWidget, std::string link, int button );
	};

	class LinkDoubleClick : public LinkButtonEvent
	{
	public:
		LinkDoubleClick( WgWidget * pWidget, std::string link, int button );
	};

	//____ PianoKeyPress ___________________________________________________
	/**
	 *	@brief Key is pressed by users action.
	 *
	 * This event is posted when a piano key is pressed as the result of a user action, e.i.
	 * pressing left mouse key on a piano key or moving pointer to key while pressed.
	 *
	 * No event is posted if piano key is pressed through an API call or key already is
	 * pressed when user presses it.
	 *
	 */

	class PianoKeyPress : public Event
	{
		friend class ::WgPianoKeyboard;
	public:

		int					key() { return m_keyIdx; }
		int64_t				timestamp() const { return m_timestamp; }

	protected:
		PianoKeyPress(WgWidget * pSource, int keyIdx, int64_t timestamp);

		int		m_keyIdx;
		int64_t	m_timestamp;
	};

	//____ PianoKeyRelease ___________________________________________________
	/**
	 *	@brief Key is released by users action.
	 *
	 * This event is posted when a piano key is released as the result of a user action, e.i.
	 * releasing mouse key on a piano key or moving pointer away from key while pressed.
	 *
	 * No event is posted if piano key remains pressed (due to pressed state set
	 * through API call).
	 *
	 */

	class PianoKeyRelease : public Event
	{
		friend class ::WgPianoKeyboard;
	public:

		int					key() { return m_keyIdx; }
		int64_t				timestamp() const { return m_timestamp; }

	protected:
		PianoKeyRelease(WgWidget * pSource, int keyIdx, int64_t timestamp);

		int		m_keyIdx;
		int64_t	m_timestamp;
	};



	//____ Internally posted events ____________________________________________

	class MousePosition : public Event
	{
		friend class ::WgEventHandler;
	protected:
		MousePosition();
	};

	class MouseButtonDrag : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	protected:
		MouseButtonDrag( int button, const WgCoord& startPos, const WgCoord& prevPos, const WgCoord& currPos );
		MouseButtonDrag( int button, WgWidget * pWidget, const WgCoord& orgPos, const WgCoord& prevPos, const WgCoord& currPos );
		virtual void 	_cloneContentFrom( const Event * pOrg );
	public:
		WgCoord			DraggedTotalPixels() const;
		WgCoord			DraggedNowPixels() const;
		WgCoord			StartPixelPos() const;
		WgCoord			PrevPixelPos() const;
		WgCoord			CurrPixelPos() const;

		WgCoord			DraggedTotalPoints() const;
		WgCoord			DraggedNowPoints() const;
		WgCoord			StartPointPos() const;
		WgCoord			PrevPointPos() const;
		WgCoord			CurrPointPos() const;
	protected:
		WgCoord			m_startPos;
		WgCoord			m_prevPos;
		WgCoord			m_currPos;
	};

	class MouseButtonRepeat : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	protected:
		MouseButtonRepeat( int button, WgWidget * pWidget );
	public:
		MouseButtonRepeat( int button );
	};

	class MouseButtonClick : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	protected:
		MouseButtonClick( int button );
		MouseButtonClick( int button, WgWidget * pWidget );
	};

	class MouseButtonDoubleClick : public MouseButtonEvent
	{
		friend class ::WgEventHandler;
	protected:
		MouseButtonDoubleClick( int button );
		MouseButtonDoubleClick( int button, WgWidget * pWidget );
	};

	class KeyRepeat : public KeyEvent
	{
		friend class ::WgEventHandler;
	protected:
		KeyRepeat( int native_keycode );
		KeyRepeat( int native_keycode, WgWidget * pWidget );
	};

}



#endif //WG_EVENT_DOT_H
