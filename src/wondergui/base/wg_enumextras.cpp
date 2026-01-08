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
			"Selekted",
			"FlaggedSelekted",
			"Checked",
			"FlaggedChecked",
			"CheckedSelekted",
			"FlaggedCheckedSelekted",
			"Focused",
			"FlaggedFocused",
			"SelektedFocused",
			"FlaggedSelektedFocused",
			"CheckedFocused",
			"FlaggedCheckedFocused",
			"CheckedSelektedFocused",
			"FlaggedCheckedSelektedFocused",
			"Hovered",
			"FlaggedHovered",
			"SelektedHovered",
			"FlaggedSelektedHovered",
			"CheckedHovered",
			"FlaggedCheckedHovered",
			"CheckedSelektedHovered",
			"FlaggedCheckedSelektedHovered",
			"HoveredFocused",
			"FlaggedHoveredFocused",
			"SelektedHoveredFocused",
			"FlaggedSelektedHoveredFocused",
			"CheckedHoveredFocused",
			"FlaggedCheckedHoveredFocused",
			"CheckedSelektedHoveredFocused",
			"FlaggedCheckedSelektedHoveredFocused",
			"Pressed",
			"FlaggedPressed",
			"SelektedPressed",
			"FlaggedSelektedPressed",
			"CheckedPressed",
			"FlaggedCheckedPressed",
			"CheckedSelektedPressed",
			"FlaggedCheckedSelektedPressed",
			"PressedFocused",
			"FlaggedPressedFocused",
			"SelektedPressedFocused",
			"FlaggedSelektedPressedFocused",
			"CheckedPressedFocused",
			"FlaggedCheckedPressedFocused",
			"CheckedSelektedPressedFocused",
			"FlaggedCheckedSelektedPressedFocused",
			"Targeted",
			"TargetedFlagged",
			"TargetedSelekted",
			"TargetedFlaggedSelekted",
			"TargetedChecked",
			"TargetedFlaggedChecked",
			"TargetedCheckedSelekted",
			"TargetedFlaggedCheckedSelekted",
			"TargetedFocused",
			"TargetedFlaggedFocused",
			"TargetedSelektedFocused",
			"TargetedFlaggedSelektedFocused",
			"TargetedCheckedFocused",
			"TargetedFlaggedCheckedFocused",
			"TargetedCheckedSelektedFocused",
			"TargetedFlaggedCheckedSelektedFocused",
			"Disabled",
			"DisabledFlagged",
			"DisabledSelekted",
			"DisabledFlaggedSelekted",
			"DisabledChecked",
			"DisabledFlaggedChecked",
			"DisabledCheckedSelekted",
			"DisabledFlaggedCheckedSelekted" };

		return names[(int)i];
	}

	const char * toString(CodePage i)
	{
		static const char * names[] = { 
			"Undefined",
			"Latin1",
			"DOS_437",
			"DOS_850",
			"Win_1250",
			"Win_1251",
			"Win_1252",
			"Win_1253",
			"Win_1254",
			"Win_1255",
			"Win_1256",
			"Win_1257",
			"Win_1258",
			"Win_874",
			"MacRoman",
			"AtariST",
			"RiscOS"
		};

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
			"Selected",
			"Unselected",
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

	const char * toString(KeyAction i)
	{
		static const char * names[] = { 
			"None",
			"Insert",
			"ReleaseFocus",
			"CycleFocus" };

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

	const char* toString(ModKeys i)
	{
		static const char* names[] = {
			"None",
			"Shift",
			"Alt",
			"AltShift",
#ifdef __APPLE__
			"Command",
			"CommandShift",
			"CommandAlt",
			"CommandAltShift",
#else
			"StdCtrl",
			"StdCtrlShift",
			"StdCtrlAlt",
			"StdCtrlAltShift",
#endif
			"MacCtrl",
			"MacCtrlShift",
			"MacCtrlAlt",
			"MacCtrlAltShift",
			"MacCtrlCmd",
			"MacCtrlCmdShift",
			"MacCtrlCmdAlt",
			"MacCtrlCmdAltShift",
			"OSKey",
			"OSKeyShift",
			"OSKeyAlt",
			"OSKeyAltShift",
			"OSKeyCtrl",
			"OSKeyCtrlShift",
			"OSKeyCtrlAlt",
			"OSKeyCtrlAltShift" };

		return names[(int)i];
	}



}	//namespace wg
