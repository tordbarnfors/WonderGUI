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
	const State State::DisabledFlaggedCheckedSelected			= State(StateEnum::DisabledFlaggedCheckedSelected);
	const State State::DisabledFlaggedChecked					= State(StateEnum::DisabledFlaggedChecked);
	const State State::DisabledFlaggedSelected					= State(StateEnum::DisabledFlaggedSelected);
	const State State::DisabledFlagged							= State(StateEnum::DisabledFlagged);
	const State State::DisabledCheckedSelected					= State(StateEnum::DisabledCheckedSelected);
	const State State::DisabledChecked							= State(StateEnum::DisabledChecked);
	const State State::DisabledSelected							= State(StateEnum::DisabledSelected);
	const State State::Disabled									= State(StateEnum::Disabled);
	const State State::TargetedFlaggedCheckedSelectedFocused	= State(StateEnum::TargetedFlaggedCheckedSelectedFocused);
	const State State::TargetedFlaggedCheckedSelected			= State(StateEnum::TargetedFlaggedCheckedSelected);
	const State State::TargetedFlaggedCheckedFocused			= State(StateEnum::TargetedFlaggedCheckedFocused);
	const State State::TargetedFlaggedChecked					= State(StateEnum::TargetedFlaggedChecked);
	const State State::TargetedFlaggedSelectedFocused			= State(StateEnum::TargetedFlaggedSelectedFocused);
	const State State::TargetedFlaggedSelected					= State(StateEnum::TargetedFlaggedSelected);
	const State State::TargetedFlaggedFocused					= State(StateEnum::TargetedFlaggedFocused);
	const State State::TargetedFlagged							= State(StateEnum::TargetedFlagged);
	const State State::TargetedCheckedSelectedFocused			= State(StateEnum::TargetedCheckedSelectedFocused);
	const State State::TargetedCheckedSelected					= State(StateEnum::TargetedCheckedSelected);
	const State State::TargetedCheckedFocused					= State(StateEnum::TargetedCheckedFocused);
	const State State::TargetedChecked							= State(StateEnum::TargetedChecked);
	const State State::TargetedSelectedFocused					= State(StateEnum::TargetedSelectedFocused);
	const State State::TargetedSelected							= State(StateEnum::TargetedSelected);
	const State State::TargetedFocused							= State(StateEnum::TargetedFocused);
	const State State::Targeted									= State(StateEnum::Targeted);
	const State State::FlaggedCheckedSelectedPressedFocused		= State(StateEnum::FlaggedCheckedSelectedPressedFocused);
	const State State::FlaggedCheckedSelectedPressed			= State(StateEnum::FlaggedCheckedSelectedPressed);
	const State State::FlaggedCheckedSelectedHoveredFocused		= State(StateEnum::FlaggedCheckedSelectedHoveredFocused);
	const State State::FlaggedCheckedSelectedHovered			= State(StateEnum::FlaggedCheckedSelectedHovered);
	const State State::FlaggedCheckedSelectedFocused			= State(StateEnum::FlaggedCheckedSelectedFocused);
	const State State::FlaggedCheckedSelected					= State(StateEnum::FlaggedCheckedSelected);
	const State State::FlaggedCheckedPressedFocused				= State(StateEnum::FlaggedCheckedPressedFocused);
	const State State::FlaggedCheckedPressed					= State(StateEnum::FlaggedCheckedPressed);
	const State State::FlaggedCheckedHoveredFocused				= State(StateEnum::FlaggedCheckedHoveredFocused);
	const State State::FlaggedCheckedHovered					= State(StateEnum::FlaggedCheckedHovered);
	const State State::FlaggedCheckedFocused					= State(StateEnum::FlaggedCheckedFocused);
	const State State::FlaggedChecked							= State(StateEnum::FlaggedChecked);
	const State State::FlaggedSelectedPressedFocused			= State(StateEnum::FlaggedSelectedPressedFocused);
	const State State::FlaggedSelectedPressed					= State(StateEnum::FlaggedSelectedPressed);
	const State State::FlaggedSelectedHoveredFocused			= State(StateEnum::FlaggedSelectedHoveredFocused);
	const State State::FlaggedSelectedHovered					= State(StateEnum::FlaggedSelectedHovered);
	const State State::FlaggedSelectedFocused					= State(StateEnum::FlaggedSelectedFocused);
	const State State::FlaggedSelected							= State(StateEnum::FlaggedSelected);
	const State State::FlaggedPressedFocused					= State(StateEnum::FlaggedPressedFocused);
	const State State::FlaggedPressed							= State(StateEnum::FlaggedPressed);
	const State State::FlaggedHoveredFocused					= State(StateEnum::FlaggedHoveredFocused);
	const State State::FlaggedHovered							= State(StateEnum::FlaggedHovered);
	const State State::FlaggedFocused							= State(StateEnum::FlaggedFocused);
	const State State::Flagged									= State(StateEnum::Flagged);
	const State State::CheckedSelectedPressedFocused			= State(StateEnum::CheckedSelectedPressedFocused);
	const State State::CheckedSelectedPressed					= State(StateEnum::CheckedSelectedPressed);
	const State State::CheckedSelectedHoveredFocused			= State(StateEnum::CheckedSelectedHoveredFocused);
	const State State::CheckedSelectedHovered					= State(StateEnum::CheckedSelectedHovered);
	const State State::CheckedSelectedFocused					= State(StateEnum::CheckedSelectedFocused);
	const State State::CheckedSelected							= State(StateEnum::CheckedSelected);
	const State State::CheckedPressedFocused					= State(StateEnum::CheckedPressedFocused);
	const State State::CheckedPressed							= State(StateEnum::CheckedPressed);
	const State State::CheckedHoveredFocused					= State(StateEnum::CheckedHoveredFocused);
	const State State::CheckedHovered							= State(StateEnum::CheckedHovered);
	const State State::CheckedFocused							= State(StateEnum::CheckedFocused);
	const State State::Checked									= State(StateEnum::Checked);
	const State State::SelectedPressedFocused					= State(StateEnum::SelectedPressedFocused);
	const State State::SelectedPressed							= State(StateEnum::SelectedPressed);
	const State State::SelectedHoveredFocused					= State(StateEnum::SelectedHoveredFocused);
	const State State::SelectedHovered							= State(StateEnum::SelectedHovered);
	const State State::SelectedFocused							= State(StateEnum::SelectedFocused);
	const State State::Selected									= State(StateEnum::Selected);
	const State State::PressedFocused							= State(StateEnum::PressedFocused);
	const State State::Pressed									= State(StateEnum::Pressed);
	const State State::HoveredFocused							= State(StateEnum::HoveredFocused);
	const State State::Hovered									= State(StateEnum::Hovered);
	const State State::Focused									= State(StateEnum::Focused);
	const State State::Default									= State(StateEnum::Default);


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
