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

#ifndef	WG_TYPES_DOT_H
#define WG_TYPES_DOT_H
#pragma once

#include <cstdint>

#include <wg_gfxtypes.h>

namespace wg
{
	static const int	PrimState_Nb = 8;
	static const int	PrimState_MaxValue = 7;

	typedef unsigned int	RouteId;

	typedef uint16_t		TextStyle_h;

	//___ PrimState ____________________________________________________

	// These are the primitive or primary states that are the building 
	// blocks of the 72 allowed States. 
	// 
	// These are used in the API where a specific (non-default) PrimState 
	// needs to be specified. They are also used internally when a State
	// needs to be broken up into its components for certain calculations.
	//
	// They are numbered in order of priority, where a higher PrimState
	// trumps a lower one where only one can be chosen.


	enum class PrimState
	{					
		Focused = 0,
		Hovered = 1,
		Pressed = 2,
		Selected = 3,
		Checked = 4,
		Flagged = 5,
		Targeted = 6,
		Disabled = 7
	};

	//___ StateEnum ____________________________________________________

	enum class StateEnum : uint8_t
	{
		Default,
		Flagged,
		Selected,
		FlaggedSelected,
		Checked,
		FlaggedChecked,
		CheckedSelected,
		FlaggedCheckedSelected,
		Focused,
		FlaggedFocused,
		SelectedFocused,
		FlaggedSelectedFocused,
		CheckedFocused,
		FlaggedCheckedFocused,
		CheckedSelectedFocused,
		FlaggedCheckedSelectedFocused,
		Hovered,
		FlaggedHovered,
		SelectedHovered,
		FlaggedSelectedHovered,
		CheckedHovered,
		FlaggedCheckedHovered,
		CheckedSelectedHovered,
		FlaggedCheckedSelectedHovered,
		HoveredFocused,
		FlaggedHoveredFocused,
		SelectedHoveredFocused,
		FlaggedSelectedHoveredFocused,
		CheckedHoveredFocused,
		FlaggedCheckedHoveredFocused,
		CheckedSelectedHoveredFocused,
		FlaggedCheckedSelectedHoveredFocused,
		Pressed,
		FlaggedPressed,
		SelectedPressed,
		FlaggedSelectedPressed,
		CheckedPressed,
		FlaggedCheckedPressed,
		CheckedSelectedPressed,
		FlaggedCheckedSelectedPressed,
		PressedFocused,
		FlaggedPressedFocused,
		SelectedPressedFocused,
		FlaggedSelectedPressedFocused,
		CheckedPressedFocused,
		FlaggedCheckedPressedFocused,
		CheckedSelectedPressedFocused,
		FlaggedCheckedSelectedPressedFocused,
		Targeted,
		TargetedFlagged,
		TargetedSelected,
		TargetedFlaggedSelected,
		TargetedChecked,
		TargetedFlaggedChecked,
		TargetedCheckedSelected,
		TargetedFlaggedCheckedSelected,
		TargetedFocused,
		TargetedFlaggedFocused,
		TargetedSelectedFocused,
		TargetedFlaggedSelectedFocused,
		TargetedCheckedFocused,
		TargetedFlaggedCheckedFocused,
		TargetedCheckedSelectedFocused,
		TargetedFlaggedCheckedSelectedFocused,
		Disabled,
		DisabledFlagged,
		DisabledSelected,
		DisabledFlaggedSelected,
		DisabledChecked,
		DisabledFlaggedChecked,
		DisabledCheckedSelected,
		DisabledFlaggedCheckedSelected
	};


//____ ExtChar __________________________________________________________

	enum class ExtChar : uint16_t
	{
		BreakPermitted			= 0x82,
		HyphenBreakPermitted	= 0x83,
		NoBreakSpace			= 0xA0,

		Ellipsis				= 0x2026
	};

	//____ CodePage ______________________________________________________________

	enum class CodePage		//. autoExtras
	{
		Latin1,
		_1250,		///< Windows Cental Europe
		_1251,		///< Windows Cyrillic
		_1252,		///< Windows Latin-1 extended
		_1253,		///< Windows Greek
		_1254,		///< Windows Turkish
		_1255,		///< Windows Hebrew
		_1256,		///< Windows Arabic
		_1257,		///< Windows Baltic
		_1258,		///< Windows Vietnam
		_874,		///< Windows Thai
	};

	static const CodePage DefaultCodePage = CodePage::_1252;

	//____ BreakRules ____________________________________________________________

	enum class BreakRules : uint8_t
	{
		NoBreak		= 0,
		BreakBefore	= 16,
		BreakOn		= 32,
		BreakAfter	= 64
	};

	//____ PointerStyle __________________________________________________________

	enum class PointerStyle : uint8_t	//. autoExtras
	{
		Undefined,
		Arrow,				// default arrow
		Hourglass,			// hourglass
		Hand,				// hand
		OpenHand,			// open hand
		ClosedHand,			// closed hand
		Crosshair,			// crosshair
		Help,				// help
		Ibeam,				// I-beam
		Stop,				// slashed circle
		UpArrow,			// arrow pointing up
		ResizeAll,			// four-pointed arrow in all directions
		ResizeNeSw,			// double-pointed arrow pointing northeast and southwest
		ResizeNwSe,			// double-pointed arrow pointing northwest and southeast
		ResizeNS,			// double-pointed arrow pointing north and south
		ResizeWE,			// double-pointed arrow pointing west and east
		ResizeBeamNS,		// double-pointed arrow pointing north and south
		ResizeBeamWE,		// double-pointed arrow pointing north and south
	};

	//____ MouseButton _________________________________________________________

	enum class MouseButton : uint8_t	//. autoExtras
	{
		None = 0,
		Left,
		Middle,
		Right,
		X1,
		X2,
	};


	//____ PlayMode _____________________________________________________________

	enum class PlayMode : uint8_t	//. autoExtras
	{
		Forward,
		Backward,
		Looping,
		BackwardLooping,
		PingPong,
		BackwardPingPong
	};


	//____ SearchMode _____________________________________________________________

	enum class SearchMode	//. autoExtras
	{
		MarkPolicy,			///< Perform a mark test on widget according to widgets MarkPolicy.
		Geometry,			///< Goes strictly on geometry, ignores markPolicy.
		ActionTarget,		///< Like MarkPolicy, but takes modality into account.
	};

	//____ MarkPolicy ____________________________________________________________

	enum class MarkPolicy : uint8_t	//. autoExtras
	{
		Undefined,
		Ignore,				///< Widget can not be marked, hovered or pressed by pointer.
		AlphaTest,			///< An alpha-test is performed to determine if widget is marked.
		Geometry			///< Widget is marked if pointer is within its geometry.
	};


	//____ SizeConstraint ___________________________________________________________
	/**
		SizeConstraint is used by certain containers, including FlexPanel and ScrollPanel,
		to limit/control the geometry of children. Different SizePolicies can be set
		for horizontal and vertical size.

		It is used in combination with a size specified by parent. In the case of
		ScrollPanel it is the size of the window to the scrollarea. In the case of
		FlexPanel, it is a size specified for the child.
	*/
	enum class SizeConstraint : uint8_t		//. autoExtras
	{
		None = 0,				///< Childs size is unaffected by size specified by parent, so it gets its default size.
		Equal,					///< Child is bound to the exact size specified by parent.
		LessOrEqual,			///< Childs size is limited to the size specified by parent.
		GreaterOrEqual			///< Childs size is set to at least the size specified by parent.
	};


	//____ SizePolicy2D ___________________________________________________________

	enum class SizePolicy2D : uint8_t	//. autoExtras
	{
		Original,						///< Object maintains its original size.
		Stretch,						///< Object is stretched to fill the space, ignoring aspect ratio.
		Scale,							///< Object is scaled to fill the space as much as possible, respecting original aspect ratio.
		StretchDown,					///< Object is shrunk to fit the space if it is too big, ignoring aspect ratio.
		ScaleDown						///< Object is shrunk to fit the space if it is too big, respecting original aspect ratio.
	};

	//____ EdgePolicy _________________________________________________________

	enum class EdgePolicy : uint8_t		//. autoExtras
	{
		Confine,
		Clip
	};


	//____ MsgType ______________________________________________________________

	enum class MsgType	//. autoExtras
	{
		Dummy = 0,
		PointerChange,

		FocusGained,
		FocusLost,

		MouseEnter,
		MouseMove,
		MouseLeave,

		MousePress,
		MouseRepeat,
		MouseDrag,
		MouseRelease,
		MouseClick,
		MouseDoubleClick,
		MouseTripleClick,

		KeyPress,
		KeyRepeat,
		KeyRelease,
		TextInput,
		EditCommand,
		WheelRoll,

		// New names

		Pick,						// Sent to picked
		PickedEnter,				// Sent to picked
		PickedLeave,				// Sent to picked
		PickedCancel,				// Sent to picked
		PickedDeliver,				// Sent to picked

		DropProbe,					// Sent to target
		DropEnter,					// Sent to target
		DropMove,					// Sent to target
		DropLeave,					// Sent to target
		DropDeliver,				// Sent to target


/* Old names


		DropPick,					// Sent to picked
		PickedEnter,				// Sent to picked
		PickedLeave,				// Sent to picked
		PickedCancel,				// Sent to picked
		PickedDeliver,				// Sent to picked
		DropProbe,					// Sent to target
		DropEnter,					// Sent to target
		DropMove,					// Sent to target
		DropLeave,					// Sent to target
		DropDeliver,				// Sent to target
 
 */
		Select,						// Non-value widget triggered, like a button being pressed.
		Toggle,						// Boolean value widget toggled, like for checkboxes, radiobuttons etc.
		ScrollbarMove,
		ValueUpdate,					// Value of widget changed, like for editvalue, animations, sliders etc
		RangeUpdate,					// Range widget updated, such as scrollbar, scrollpanel, rangesliders etc.

		TextEdit,						// Text widget edited, like texteditor, editline, etc.


		ItemToggle,
	//	ItemMouseEnter,
	//	ItemMouseLeave,
		ItemMousePress,
	//	ItemMouseDrag,
	//	ItemMouseRepeat,
	//	ItemMouseRelease,
	//	ItemMouseClick,
	//	ItemMouseDoubleClick,
		ItemsSelect,
		ItemsUnselect,

		PopupOpened,
		PopupSelect,
		PopupClosed,

		ModalMoveOutside,
		ModalBlockedPress,
		ModalBlockedRelease,

		PianoKeyPress,
		PianoKeyRelease,
		
		Activate,
		Deactivate,
		Ping,

		Custom,

	};


	//____ SortOrder ____________________________________________________________

	enum class SortOrder	//. autoExtras
	{
		None,
		Ascending,
		Descending
	};


	//____ SelectMode ___________________________________________________________

	enum class SelectMode : uint8_t	//. autoExtras
	{
		Unselectable,		///< Entries can not be selected.
		SingleEntry,		///< Only a single entry can be selected at a time.
		MultiEntries,		///< Multiple entries can be selected at the same time.
		FlipOnSelect		///< Multiple entries can be selected at the same time and are switched individually.
	};

	//____ TextEditMode _________________________________________________________

	enum class TextEditMode : uint8_t	//. autoExtras
	{
		Static,
		Selectable,
		Editable
	};

	//____ EditCmd _____________________________________________________________

	enum class EditCmd
	{
		None,
		Cut,
		Copy,
		Paste,
		Undo,
		Redo,
		SelectAll,
		Escape,
		Break
	};

	//____ KeyAction _______________________________________________________

	enum class KeyAction
	{
		None,					// Return is ignored
		Insert,					// Return is inserted into the text
		ReleaseFocus,			// Return releases focus, just like Escape
		CycleFocus				// Return cycles focus, just like Tab
	};

	//___  TextDecoration ________________________________________________________

	enum class TextDecoration
	{
		Undefined,
		None,
		Underline
	};


	//____ ComponentNotif ___________________________________________________________

	enum class ComponentNotif
	{
		StartReceiveUpdates,
		StopReceiveUpdates,
		RequestsPreRenderCall,
		SortOrderChanged,
		ValueEdited,
		TextModified,
		SpanModified,
		StepForward,
		StepBackward,
		JumpForward,
		JumpBackward,
		Other
	};




	//____ MaskOp ____________________________________________________________

	enum class MaskOp	//. autoExtras
	{
		Recurse,		///< Recurse through children, let them mask background individually.
		Skip,			///< Do not mask background against container or children.
		Mask			///< Mask background against whole container.
	};

	//____ DropTypes _____________________________________________________________

	enum class DropType
	{
		Undefined,
		TextString,			///< Supplied dataset is a Dataset<std::string> object.
		Widget,				///< Supplied dataset is a Dataset<Widget_p> object.
		WidgetVector		///< Supplied dataset is a Dataset<std::vector<Widget_p>> object.
	};


} // namespace wg

#include <wg_enumextras.h>

#endif // WG_TYPES_DOT_H
