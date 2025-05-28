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

#include <wg_state.h>

namespace wg
{
	const State State::Default = State(StateEnum::Default);
	const State State::Focused = State(StateEnum::Focused);
	const State State::Hovered = State(StateEnum::Hovered);
	const State State::Pressed = State(StateEnum::Pressed);
	const State State::Targeted = State(StateEnum::Targeted);
	const State State::Selected = State(StateEnum::Selected);
	const State State::Checked = State(StateEnum::Checked);
	const State State::Flagged = State(StateEnum::Flagged);
	const State State::Disabled = State(StateEnum::Disabled);
}
