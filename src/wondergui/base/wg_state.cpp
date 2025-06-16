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

	const uint8_t State::s_weights[NbStates] = {
		0, 32, 8, 40, 16, 48, 24, 56, 1, 33, 9, 41, 17, 49, 25, 57,
		2, 34, 10, 42, 18, 50, 26, 58, 3, 35, 11, 43, 19, 51, 27, 59,
		4, 36, 12, 44, 20, 52, 28, 60, 5, 37, 13, 45, 21, 53, 29, 61,
		64, 96, 72, 104, 80, 112, 88, 120, 65, 97, 73, 105, 81, 113, 89, 121,
		128, 160, 136, 168, 144, 176, 152, 184
		};

	//____ bestMatch() ________________________________________________________

	int State::bestMatch(int nAlternatives, const State* pAlternatives)
	{
		int		bestWeight = -1;
		int		bestIndex = -1;

		uint8_t	unwantedBits = ~s_weights[m_index];

		for (int i = 0; i < nAlternatives; i++)
		{
			// First check that the alternative does NOT have PrimStates that we don't

			int weight = pAlternatives[i].weight();

			if ((pAlternatives[i].weight() & unwantedBits) != 0)
				continue;

			if (weight > bestWeight)
			{
				bestWeight = weight;
				bestIndex = i;
			}
		}

		return bestIndex;
	}
}
