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

#ifndef	WG2_TYPES_DOT_H
#define WG2_TYPES_DOT_H

#include <wg_types.h>
#include <wg_state.h>


typedef wg::State          		WgStateEnum;

namespace wg
{
	typedef State           	StateEnum;
}

static const int    WG_NB_STATES = wg::State::NbStates;
static const int    WG_MAX_STATE_VALUE = wg::State::MaxValue;

typedef wg::State       WgState;

typedef wg::ExtChar     WgExtChar;

typedef wg::CodePage    WgCodePage;

const static int   WG_NB_CODEPAGES = wg::CodePage_size;

typedef wg::BreakRules  WgBreakRules;

typedef wg::BlendMode   WgBlendMode;

typedef wg::PointerStyle WgPointerStyle;

typedef wg::SearchMode  WgSearchMode;
typedef wg::Placement   WgOrigo;
typedef wg::Direction   WgDirection;
typedef wg::SizeConstraint  WgSizePolicy;

typedef wg::SampleMethod   WgScaleMode;
typedef wg::PixelFormat WgPixelType;

typedef wg::SortOrder   WgSortOrder;
typedef wg::SelectMode  WgSelectMode;
typedef wg::TextEditMode WgTextEditMode;
typedef wg::MaskOp      WgMaskOp;
typedef wg::PlayMode    WgAnimMode;

typedef wg::PixelDescription    WgPixelFormat;


#	include <stdint.h>			// Use the C99 official header


#ifndef Uint8
	typedef unsigned char	Uint8;
	typedef signed char		Sint8;
	typedef unsigned short	Uint16;
	typedef signed short		Sint16;
	typedef unsigned int		Uint32;
	typedef signed int		Sint32;
	typedef uint64_t			Uint64;
	typedef int64_t			Sint64;
#endif


#define WG_SCALE_BINALS		12
#define WG_SCALE_BASE		(1<<WG_SCALE_BINALS)

//____ WgMode _________________________________________________________________

enum WgMode //: Uint8
{
	WG_MODE_NORMAL =	0,
	WG_MODE_MARKED =	1,		// Widget is highlighted (mouse over)
	WG_MODE_SELECTED =	2,		// Widget is selected (pressed)
	WG_MODE_DISABLED =	3,		// Widget is disabled
	WG_MODE_SPECIAL =	4,		// Graphics block for a special, widget-specific mode.
	WG_MODE_ALL		=	5		// Only supported by certain methods!!!
};

#define	WG2_NB_MODES		5		// Number of modes (excluding WG_MODE_ALL )


//____ WgFontStyle ____________________________________________________________

enum WgFontStyle
{
	// Needs to stay in sync with WgExtChar::WG2_BEGIN_NORMAL and following enums!

	WG_STYLE_NORMAL		= 0,
	WG_STYLE_BOLD		= 1,
	WG_STYLE_BOLD_ITALIC= 2,
	WG_STYLE_ITALIC		= 3,
	WG_STYLE_SUPERSCRIPT= 4,
	WG_STYLE_SUBSCRIPT	= 5,
	WG_STYLE_MONOSPACE	= 6,
	WG_STYLE_HEADING_1	= 7,
	WG_STYLE_HEADING_2	= 8,
	WG_STYLE_HEADING_3	= 9,
	WG_STYLE_HEADING_4	= 10,
	WG_STYLE_HEADING_5	= 11,

	WG_STYLE_USER_1		= 12,
	WG_STYLE_USER_2		= 13,
	WG_STYLE_USER_3		= 14,
	WG_STYLE_USER_4		= 15,
	WG_STYLE_USER_5		= 16,

};

#define WG2_NB_FONTSTYLES	17


//____ WgTintMode _____________________________________________________________

// WgTintMode controls how TintColors are blended hierarchically.

enum WgTintMode
{
	WG_TINTMODE_OPAQUE,
	WG_TINTMODE_MULTIPLY
};




//____ WgEventType ______________________________________________________________

enum    WgEventType
{
    WG_EVENT_DUMMY = 0,
    WG_EVENT_TICK,
    WG_EVENT_POINTER_CHANGE,

    WG_EVENT_FOCUS_GAINED,
    WG_EVENT_FOCUS_LOST,

    WG_EVENT_MOUSE_ENTER,
    WG_EVENT_MOUSE_MOVE,
    WG_EVENT_MOUSE_POSITION,
    WG_EVENT_MOUSE_LEAVE,

    WG_EVENT_MOUSEBUTTON_PRESS,
    WG_EVENT_MOUSEBUTTON_REPEAT,
    WG_EVENT_MOUSEBUTTON_DRAG,
    WG_EVENT_MOUSEBUTTON_RELEASE,
    WG_EVENT_MOUSEBUTTON_CLICK,
    WG_EVENT_MOUSEBUTTON_DOUBLE_CLICK,

    WG_EVENT_KEY_PRESS,
    WG_EVENT_KEY_REPEAT,
    WG_EVENT_KEY_RELEASE,
    WG_EVENT_CHARACTER,
    WG_EVENT_MOUSEWHEEL_ROLL,

    WG_EVENT_SELECTED,                        // Posted every time a popup menu entry is selected. Should be used by more widgets in the future.
    WG_EVENT_BUTTON_PRESS,                    // Posted every time a button is pressed, either by left mouse button or enter key.

    WG_EVENT_CHECKBOX_CHECK,
    WG_EVENT_CHECKBOX_UNCHECK,
    WG_EVENT_CHECKBOX_TOGGLE,

    WG_EVENT_RADIOBUTTON_SELECT,
    WG_EVENT_RADIOBUTTON_UNSELECT,
    WG_EVENT_RADIOBUTTON_TOGGLE,

    WG_EVENT_ANIMATION_UPDATE,                // Posted every time the animation changes frame.

    WG_EVENT_TAB_SELECT,                    // Posted when a new tab is selected.
    WG_EVENT_TAB_PRESS,                        // Posted every time any mousebutton is pressed on a widget.

    WG_EVENT_SLIDER_MOVE,
    WG_EVENT_SLIDER_STEP_FWD,
    WG_EVENT_SLIDER_STEP_BWD,
    WG_EVENT_SLIDER_JUMP_FWD,
    WG_EVENT_SLIDER_JUMP_BWD,
    WG_EVENT_SLIDER_WHEEL_ROLL,

    WG_EVENT_TEXT_MODIFY,                    // Posted for every change to text through cursor/selection.
    WG_EVENT_TEXT_SET,                        // Posted when edited text is 'set' by removing cursor.

    WG_EVENT_MENUITEM_SELECT,                // Posted when a WgMenuEntry or WgMenuRadioButton is selected.
    WG_EVENT_MENUITEM_CHECK,                // Posted when a WgMenuCheckBox is checked.
    WG_EVENT_MENUITEM_UNCHECK,                // Posted when a WgMenuCheckBox is unchecked.

    WG_EVENT_POPUP_CLOSED,

    WG_EVENT_MODAL_MOVE_OUTSIDE,
    WG_EVENT_MODAL_BLOCKED_PRESS,
    WG_EVENT_MODAL_BLOCKED_RELEASE,

    WG_EVENT_TABLE_CELL_MARK,
    WG_EVENT_TABLE_CELL_UNMARK,

    WG_EVENT_LINK_MARK,
    WG_EVENT_LINK_UNMARK,
    WG_EVENT_LINK_PRESS,
    WG_EVENT_LINK_REPEAT,
    WG_EVENT_LINK_RELEASE,
    WG_EVENT_LINK_CLICK,
    WG_EVENT_LINK_DOUBLE_CLICK,

    WG_EVENT_KNOB_TURN,
    WG_EVENT_RANGE_CHANGED,

    WG_EVENT_SLIDER_PRESS,
    WG_EVENT_SLIDER_MOVED,
    WG_EVENT_SLIDER_ENTER,
    WG_EVENT_SLIDER_LEAVE,

    WG_EVENT_SLIDER_HANDLE_ENTER,
    WG_EVENT_SLIDER_HANDLE_LEAVE,

    WG_EVENT_DROP_PICK,
    WG_EVENT_DROP_PROBE,
    WG_EVENT_DROP_ENTER,
    WG_EVENT_DROP_MOVE,
    WG_EVENT_DROP_LEAVE,
    WG_EVENT_DROP_DELIVER,
    WG_EVENT_DROP_CANCEL,
    WG_EVENT_DROP_COMPLETE,

    WG_EVENT_DROPHOVER_ENTER,
    WG_EVENT_DROPHOVER_MOVE,
    WG_EVENT_DROPHOVER_LEAVE,


    WG_EVENT_LANES_DROP,
	WG_EVENT_LANES_INSERT,
	WG_EVENT_LANES_BUTTON_PRESS,

	WG_EVENT_EAVESDROP,

	WG_EVENT_PIANOKEY_PRESS,
	WG_EVENT_PIANOKEY_RELEASE,

    //    WG_EVENT_SLIDER_PRESS_REPEAT,
    //    WG_EVENT_SLIDER_RELEASE,
    //    WG_EVENT_SLIDER_CLICK,
    //    WG_EVENT_SLIDER_DOUBLE_CLICK,


    /*
     WG_EVENT_ITEM_ENTER,
     WG_EVENT_ITEM_LEAVE,
     WG_EVENT_ITEM_PRESS,
     WG_EVENT_ITEM_DRAG,
     WG_EVENT_ITEM_REPEAT,
     WG_EVENT_ITEM_RELEASE,
     WG_EVENT_ITEM_CLICK,
     WG_EVENT_ITEM_DOUBLE_CLICK,
     */

    WG_EVENT_MAX
};



//____ WgTextPos ______________________________________________________________

class WgTextPos
{
public:
	WgTextPos() : line(0), col(0) {}
	WgTextPos( int line, int col ) : line(line), col(col) {}

	int	line;
	int col;
};

//____ WgRange ____________________________________________________________

class WgRange
{
public:
	WgRange() : ofs(0), len(0) {}
	WgRange( int ofs, int len ) : ofs(ofs), len(len) {}

	inline int	Begin() const { return ofs; }
	inline int	End() const { return ofs+len; }

	int ofs;
	int len;
};


#endif // WG2_TYPES_DOT_H
