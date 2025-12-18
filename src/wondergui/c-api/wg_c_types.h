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
#ifndef WG_C_TYPES_DOT_H
#define WG_C_TYPES_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

//____ wg_state _______________________________________________________________

typedef struct wg_state_struct
{
	uint8_t	focused : 1;
	uint8_t	hovered : 1;
	uint8_t	pressed : 1;
	uint8_t	selected : 1;
	uint8_t	targeted : 1;
	uint8_t	disabled : 1;
} wg_state;

//____ wg_pointerStyle _______________________________________________________

typedef enum
{
	WG_POINTER_ARROW,				// default arrow
	WG_POINTER_DEFAULT = WG_POINTER_ARROW,	// default arrow
	WG_POINTER_HOURGLASS,			// hourglass
	WG_POINTER_HAND,				// hand
	WG_POINTER_OPENHAND,			// open hand
	WG_POINTER_CLOSEDHAND,			// closed hand
	WG_POINTER_CROSSHAIR,			// crosshair
	WG_POINTER_HELP,				// help
	WG_POINTER_IBEAM,				// I-beam
	WG_POINTER_STOP,				// slashed circle
	WG_POINTER_UPARROW,			// arrow pointing up
	WG_POINTER_RESIZEALL,			// four-pointed arrow in all directions
	WG_POINTER_RESIZENESW,			// double-pointed arrow pointing northeast and southwest
	WG_POINTER_RESIZENWSE,			// double-pointed arrow pointing northwest and southeast
	WG_POINTER_RESIZENS,			// double-pointed arrow pointing north and south
	WG_POINTER_RESIZEWE,			// double-pointed arrow pointing west and east
	WG_POINTER_RESIZEBEAMNS,		// double-pointed arrow pointing north and south
	WG_POINTER_RESIZEBEAMWE,		// double-pointed arrow pointing north and south

} wg_pointerStyle;

//____ wg_key ______________________________________________________________

typedef enum
{
	WG_KEY_LEFT = 1,
	WG_KEY_RIGHT,
	WG_KEY_UP,
	WG_KEY_DOWN,

	WG_KEY_HOME,
	WG_KEY_END,
	WG_KEY_PAGE_UP,
	WG_KEY_PAGE_DOWN,

	WG_KEY_RETURN,
	WG_KEY_BACKSPACE,
	WG_KEY_DELETE,
	WG_KEY_TAB,
	WG_KEY_ESCAPE,
	WG_KEY_SPACE,

	WG_KEY_F1,
	WG_KEY_F2,
	WG_KEY_F3,
	WG_KEY_F4,
	WG_KEY_F5,
	WG_KEY_F6,
	WG_KEY_F7,
	WG_KEY_F8,
	WG_KEY_F9,
	WG_KEY_F10,
	WG_KEY_F11,
	WG_KEY_F12,

	WG_KEY_SHIFT,
	WG_KEY_ALT,
	WG_KEY_COMMAND,
	WG_KEY_STD_CONTROL = WG_KEY_COMMAND,			// Should map against Control on a Windows keyboard and Command on a Mac keyboard (mostly same functionality)
	WG_KEY_MAC_CONTROL,
	WG_KEY_OS,

	WG_KEY_UNMAPPED
} wg_key;


//____ wg_modkeys __________________________________________________________

typedef enum
{
	WG_MODKEYS_NONE = 0,
	WG_MODKEYS_SHIFT = 1,
	WG_MODKEYS_ALT = 2,
	WG_MODKEYS_ALT_SHIFT = 3,

	WG_MODKEYS_CMD = 4,
	WG_MODKEYS_CMD_SHIFT = 5,
	WG_MODKEYS_CMD_ALT = 6,
	WG_MODKEYS_CMD_ALT_SHIFT = 7,

	WG_MODKEYS_STD_CTRL = 4,
	WG_MODKEYS_STD_CTRL_SHIFT = 5,
	WG_MODKEYS_STD_CTRL_ALT = 6,
	WG_MODKEYS_STD_CTRL_ALT_SHIFT = 7,

	WG_MODKEYS_MAC_CTRL = 8,
	WG_MODKEYS_MAC_CTRL_SHIFT = 9,
	WG_MODKEYS_MAC_CTRL_ALT = 10,
	WG_MODKEYS_MAC_CTRL_ALT_SHIFT = 11,
	WG_MODKEYS_MAC_CTRL_CMD = 12,
	WG_MODKEYS_MAC_CTRL_CMD_SHIFT = 13,
	WG_MODKEYS_MAC_CTRL_CMD_ALT = 14,
	WG_MODKEYS_MAC_CTRL_CMD_ALT_SHIFT = 15,

	WG_MODKEYS_OS = 16,
	WG_MODKEYS_OS_SHIFT = 17,
	WG_MODKEYS_OS_ALT = 18,
	WG_MODKEYS_OS_ALT_SHIFT = 19,
	WG_MODKEYS_OS_CTRL = 20,
	WG_MODKEYS_OS_CTRL_SHIFT = 21,
	WG_MODKEYS_OS_CTRL_ALT = 22,
	WG_MODKEYS_OS_CTRL_ALT_SHIFT = 23,

} wg_modkeys;


//____ wg_editCommand _____________________________________________________

typedef enum
{
	WG_EDITCMD_NONE,
	WG_EDITCMD_CUT,
	WG_EDITCMD_COPY,
	WG_EDITCMD_PASTE,
	WG_EDITCMD_UNDO,
	WG_EDITCMD_REDO,
	WG_EDITCMD_SELECT_ALL,
	WG_EDITCMD_ESCAPE,
	WG_EDITCMD_BREAK
	
} wg_editCommand;


#ifdef __cplusplus
}
#endif

#endif
