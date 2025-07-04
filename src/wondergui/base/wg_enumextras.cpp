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

#include <wg_enumextras.h>
#include <assert.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const char * toString(PrimState i)
	{
		static const char * names[] = { 
			"Focused",
			"Hovered",
			"Pressed",
			"Selected",
			"Checked",
			"Flagged",
			"Targeted",
			"Disabled" };

		return names[(int)i];
	}

	const char * toString(StateEnum i)
	{
		static const char * names[] = { 
			"Default",
			"Flagged",
			"Selected",
			"SelectedFlagged",
			"Checked",
			"CheckedFlagged",
			"CheckedSelected",
			"CheckedSelectedFlagged",
			"Focused",
			"FocusedFlagged",
			"FocusedSelected",
			"FocusedSelectedFlagged",
			"FocusedChecked",
			"FocusedCheckedFlagged",
			"FocusedCheckedSelected",
			"FocusedCheckedSelectedFlagged",
			"Hovered",
			"HoveredFlagged",
			"HoveredSelected",
			"HoveredSelectedFlagged",
			"HoveredChecked",
			"HoveredCheckedFlagged",
			"HoveredCheckedSelected",
			"HoveredCheckedSelectedFlagged",
			"HoveredFocused",
			"HoveredFocusedFlagged",
			"HoveredFocusedSelected",
			"HoveredFocusedSelectedFlagged",
			"HoveredFocusedChecked",
			"HoveredFocusedCheckedFlagged",
			"HoveredFocusedCheckedSelected",
			"HoveredFocusedCheckedSelectedFlagged",
			"Pressed",
			"PressedFlagged",
			"PressedSelected",
			"PressedSelectedFlagged",
			"PressedChecked",
			"PressedCheckedFlagged",
			"PressedCheckedSelected",
			"PressedCheckedSelectedFlagged",
			"PressedFocused",
			"PressedFocusedFlagged",
			"PressedFocusedSelected",
			"PressedFocusedSelectedFlagged",
			"PressedFocusedChecked",
			"PressedFocusedCheckedFlagged",
			"PressedFocusedCheckedSelected",
			"PressedFocusedCheckedSelectedFlagged",
			"Targeted",
			"TargetedFlagged",
			"TargetedSelected",
			"TargetedSelectedFlagged",
			"TargetedChecked",
			"TargetedCheckedFlagged",
			"TargetedCheckedSelected",
			"TargetedCheckedSelectedFlagged",
			"TargetedFocused",
			"TargetedFocusedFlagged",
			"TargetedFocusedSelected",
			"TargetedFocusedSelectedFlagged",
			"TargetedFocusedChecked",
			"TargetedFocusedCheckedFlagged",
			"TargetedFocusedCheckedSelected",
			"TargetedFocusedCheckedSelectedFlagged",
			"Disabled",
			"DisabledFlagged",
			"DisabledSelected",
			"DisabledSelectedFlagged",
			"DisabledChecked",
			"DisabledCheckedFlagged",
			"DisabledCheckedSelected",
			"DisabledCheckedSelectedFlagged" };

		return names[(int)i];
	}

	const char * toString(CodePage i)
	{
		static const char * names[] = { 
			"Latin1",
			"_1250",
			"_1251",
			"_1252",
			"_1253",
			"_1254",
			"_1255",
			"_1256",
			"_1257",
			"_1258",
			"_874" };

		return names[(int)i];
	}

	const char * toString(PointerStyle i)
	{
		static const char * names[] = { 
			"Undefined",
			"Arrow",
			"Hourglass",
			"Hand",
			"OpenHand",
			"ClosedHand",
			"Crosshair",
			"Help",
			"Ibeam",
			"Stop",
			"UpArrow",
			"ResizeAll",
			"ResizeNeSw",
			"ResizeNwSe",
			"ResizeNS",
			"ResizeWE",
			"ResizeBeamNS",
			"ResizeBeamWE" };

		return names[(int)i];
	}

	const char * toString(MouseButton i)
	{
		static const char * names[] = { 
			"None",
			"Left",
			"Middle",
			"Right",
			"X1",
			"X2" };

		return names[(int)i];
	}

	const char * toString(PlayMode i)
	{
		static const char * names[] = { 
			"Forward",
			"Backward",
			"Looping",
			"BackwardLooping",
			"PingPong",
			"BackwardPingPong" };

		return names[(int)i];
	}

	const char * toString(SearchMode i)
	{
		static const char * names[] = { 
			"MarkPolicy",
			"Geometry",
			"ActionTarget" };

		return names[(int)i];
	}

	const char * toString(MarkPolicy i)
	{
		static const char * names[] = { 
			"Undefined",
			"Ignore",
			"AlphaTest",
			"Geometry" };

		return names[(int)i];
	}

	const char * toString(SizeConstraint i)
	{
		static const char * names[] = { 
			"None",
			"Equal",
			"LessOrEqual",
			"GreaterOrEqual" };

		return names[(int)i];
	}

	const char * toString(SizePolicy2D i)
	{
		static const char * names[] = { 
			"Original",
			"Stretch",
			"Scale",
			"StretchDown",
			"ScaleDown" };

		return names[(int)i];
	}

	const char * toString(EdgePolicy i)
	{
		static const char * names[] = { 
			"Confine",
			"Clip" };

		return names[(int)i];
	}

	const char * toString(MsgType i)
	{
		static const char * names[] = { 
			"Dummy",
			"PointerChange",
			"FocusGained",
			"FocusLost",
			"MouseEnter",
			"MouseMove",
			"MouseLeave",
			"MousePress",
			"MouseRepeat",
			"MouseDrag",
			"MouseRelease",
			"MouseClick",
			"MouseDoubleClick",
			"MouseTripleClick",
			"KeyPress",
			"KeyRepeat",
			"KeyRelease",
			"TextInput",
			"EditCommand",
			"WheelRoll",
			"Pick",
			"PickedEnter",
			"PickedLeave",
			"PickedCancel",
			"PickedDeliver",
			"DropProbe",
			"DropEnter",
			"DropMove",
			"DropLeave",
			"DropDeliver",
			"/* Old namesDropPick",
			"PickedEnter",
			"PickedLeave",
			"PickedCancel",
			"PickedDeliver",
			"DropProbe",
			"DropEnter",
			"DropMove",
			"DropLeave",
			"DropDeliver",
			"*/Select",
			"Toggle",
			"ScrollbarMove",
			"ValueUpdate",
			"RangeUpdate",
			"TextEdit",
			"ItemToggle",
			"ItemMousePress",
			"ItemsSelect",
			"ItemsUnselect",
			"PopupOpened",
			"PopupSelect",
			"PopupClosed",
			"ModalMoveOutside",
			"ModalBlockedPress",
			"ModalBlockedRelease",
			"PianoKeyPress",
			"PianoKeyRelease",
			"Activate",
			"Deactivate",
			"Ping",
			"Custom" };

		return names[(int)i];
	}

	const char * toString(SortOrder i)
	{
		static const char * names[] = { 
			"None",
			"Ascending",
			"Descending" };

		return names[(int)i];
	}

	const char * toString(SelectMode i)
	{
		static const char * names[] = { 
			"Unselectable",
			"SingleEntry",
			"MultiEntries",
			"FlipOnSelect" };

		return names[(int)i];
	}

	const char * toString(TextEditMode i)
	{
		static const char * names[] = { 
			"Static",
			"Selectable",
			"Editable" };

		return names[(int)i];
	}

	const char * toString(MaskOp i)
	{
		static const char * names[] = { 
			"Recurse",
			"Skip",
			"Mask" };

		return names[(int)i];
	}


//=========================================================================
//. endAutoSection



}	//namespace wg
