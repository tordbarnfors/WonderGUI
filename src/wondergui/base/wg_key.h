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
#ifndef WG_KEY_DOT_H
#define WG_KEY_DOT_H
#pragma once

namespace wg
{


	//____ ModKeys ____________________________________________________

	namespace ModifierKeysNS
	{
		enum ModKeys
		{
			// Don't touch, needs to be first!
			None = 0,			// = 0

			Shift = 1,
			Alt = 2,
			AltShift = 3,

			Command = 4,
			CommandShift = 5,
			CommandAlt = 6,
			CommandAltShift = 7,

			StdCtrl = 4,
			StdCtrlShift = 5,
			StdCtrlAlt = 6,
			StdCtrlAltShift = 7,


			MacCtrl = 8,

			MacCtrlShift = 9,
			MacCtrlAlt = 10,
			MacCtrlAltShift = 11,
			MacCtrlCmd = 12,
			MacCtrlCmdShift = 13,
			MacCtrlCmdAlt = 14,
			MacCtrlCmdAltShift = 15,

			OSKey = 16,

			OSKeyShift = 17,
			OSKeyAlt = 18,
			OSKeyAltShift = 19,
			OSKeyCtrl = 20,
			OSKeyCtrlShift = 21,
			OSKeyCtrlAlt = 22,
			OSKeyCtrlAltShift = 23,
		};
	}

	using ModKeys = ModifierKeysNS::ModKeys;

	//____ Key ________________________________________________________________

	enum class Key
	{
		Left = 1,
		Right,
		Up,
		Down,

		Home,
		End,
		PageUp,
		PageDown,

		Return,
		Backspace,
		Delete,
		Tab,
		Escape,
		Space,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		Shift,
		Alt,
		Command,				// Map against Command on Mac, essentially the same as Control on Windows/Linux.
		StdControl = Command,	// Map against Control on Windows/Linux.
		MacControl,				// Map against Control on a Mac keyboard. Used among else to simulate second button on mouse button press.
		OSKey,					// Map against Start key on Windows/Linux.
		
		Unmapped		// This one must always be last!
	};



} // namespace wg
#endif // WG_KEY_DOT_H
