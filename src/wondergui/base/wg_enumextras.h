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

#ifndef	WG_ENUMEXTRAS_DOT_H
#define WG_ENUMEXTRAS_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_key.h>
#include <wg_gfxenumextras.h>

namespace wg
{

//. startAutoSection
/*=========================================================================

				>>> START OF AUTO GENERATED SECTION <<<

				Any modifications here will be overwritten!

=========================================================================*/

	const static PrimState       PrimState_min       = PrimState::Focused;
	const static StateEnum       StateEnum_min       = StateEnum::Default;
	const static CodePage        CodePage_min        = CodePage::Latin1;
	const static PointerStyle    PointerStyle_min    = PointerStyle::Undefined;
	const static MouseButton     MouseButton_min     = MouseButton::None;
	const static PlayMode        PlayMode_min        = PlayMode::Forward;
	const static SearchMode      SearchMode_min      = SearchMode::MarkPolicy;
	const static MarkPolicy      MarkPolicy_min      = MarkPolicy::Undefined;
	const static SizeConstraint  SizeConstraint_min  = SizeConstraint::None;
	const static SizePolicy2D    SizePolicy2D_min    = SizePolicy2D::Original;
	const static EdgePolicy      EdgePolicy_min      = EdgePolicy::Confine;
	const static MsgType         MsgType_min         = MsgType::Dummy;
	const static SortOrder       SortOrder_min       = SortOrder::None;
	const static SelectMode      SelectMode_min      = SelectMode::Unselectable;
	const static TextEditMode    TextEditMode_min    = TextEditMode::Static;
	const static KeyAction       KeyAction_min       = KeyAction::None;
	const static MaskOp          MaskOp_min          = MaskOp::Recurse;

	const static PrimState       PrimState_max       = PrimState::Disabled;
	const static StateEnum       StateEnum_max       = StateEnum::DisabledFlaggedCheckedSelected;
	const static CodePage        CodePage_max        = CodePage::_874;
	const static PointerStyle    PointerStyle_max    = PointerStyle::ResizeBeamWE;
	const static MouseButton     MouseButton_max     = MouseButton::X2;
	const static PlayMode        PlayMode_max        = PlayMode::BackwardPingPong;
	const static SearchMode      SearchMode_max      = SearchMode::ActionTarget;
	const static MarkPolicy      MarkPolicy_max      = MarkPolicy::Geometry;
	const static SizeConstraint  SizeConstraint_max  = SizeConstraint::GreaterOrEqual;
	const static SizePolicy2D    SizePolicy2D_max    = SizePolicy2D::ScaleDown;
	const static EdgePolicy      EdgePolicy_max      = EdgePolicy::Clip;
	const static MsgType         MsgType_max         = MsgType::Custom;
	const static SortOrder       SortOrder_max       = SortOrder::Descending;
	const static SelectMode      SelectMode_max      = SelectMode::FlipOnSelect;
	const static TextEditMode    TextEditMode_max    = TextEditMode::Editable;
	const static KeyAction       KeyAction_max       = KeyAction::CycleFocus;
	const static MaskOp          MaskOp_max          = MaskOp::Mask;

	const static int             PrimState_size      = (int)PrimState::Disabled + 1;
	const static int             StateEnum_size      = (int)StateEnum::DisabledFlaggedCheckedSelected + 1;
	const static int             CodePage_size       = (int)CodePage::_874 + 1;
	const static int             PointerStyle_size   = (int)PointerStyle::ResizeBeamWE + 1;
	const static int             MouseButton_size    = (int)MouseButton::X2 + 1;
	const static int             PlayMode_size       = (int)PlayMode::BackwardPingPong + 1;
	const static int             SearchMode_size     = (int)SearchMode::ActionTarget + 1;
	const static int             MarkPolicy_size     = (int)MarkPolicy::Geometry + 1;
	const static int             SizeConstraint_size = (int)SizeConstraint::GreaterOrEqual + 1;
	const static int             SizePolicy2D_size   = (int)SizePolicy2D::ScaleDown + 1;
	const static int             EdgePolicy_size     = (int)EdgePolicy::Clip + 1;
	const static int             MsgType_size        = (int)MsgType::Custom + 1;
	const static int             SortOrder_size      = (int)SortOrder::Descending + 1;
	const static int             SelectMode_size     = (int)SelectMode::FlipOnSelect + 1;
	const static int             TextEditMode_size   = (int)TextEditMode::Editable + 1;
	const static int             KeyAction_size      = (int)KeyAction::CycleFocus + 1;
	const static int             MaskOp_size         = (int)MaskOp::Mask + 1;

	const char * toString(PrimState);
	const char * toString(StateEnum);
	const char * toString(CodePage);
	const char * toString(PointerStyle);
	const char * toString(MouseButton);
	const char * toString(PlayMode);
	const char * toString(SearchMode);
	const char * toString(MarkPolicy);
	const char * toString(SizeConstraint);
	const char * toString(SizePolicy2D);
	const char * toString(EdgePolicy);
	const char * toString(MsgType);
	const char * toString(SortOrder);
	const char * toString(SelectMode);
	const char * toString(TextEditMode);
	const char * toString(KeyAction);
	const char * toString(MaskOp);

//=========================================================================
//. endAutoSection



	const static ModKeys         ModKeys_min = ModKeys::None;
	const static ModKeys         ModKeys_max = ModKeys::OSKeyCtrlAltShift;
	const static int             ModKeys_size = (int)ModKeys::OSKeyCtrlAltShift + 1;

	const char* toString(ModKeys);


} // namespace wg

#endif //WG_ENUMEXTRAS_DOT_H
