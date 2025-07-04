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

#ifndef	WG_STATE_DOT_H
#define WG_STATE_DOT_H
#pragma once

#include <wg_types.h>

#include <algorithm>

namespace wg
{

	class State
	{
	public:

		//.____ Creation _______________________________________________________

		constexpr State() : m_index(0) {}

		constexpr State(StateEnum value) : m_enum(value) {}

		//.____ State __________________________________________________________

		bool	setDisabled(bool bDisabled)
		{
			if (!bDisabled)
				m_index &= ~DISABLED_MASK;
			else
			{
				m_index &= ~FOCUSED_MASK;
				m_index = (m_index & ~MOUSE_INTERACTION_MASK) |
					(static_cast<uint8_t>(MouseInteraction::None) << MOUSE_INTERACTION_BIT);
				m_index |= DISABLED_MASK;
			}
			return true;
		}

		bool	setSelected(bool bSelected)
		{
			if (bSelected)
				m_index |= SELECTED_MASK;
			else
				m_index &= ~SELECTED_MASK;
			return true;
		}

		bool	setChecked(bool bChecked)
		{
			if (bChecked)
				m_index |= CHECKED_MASK;
			else
				m_index &= ~CHECKED_MASK;
			return true;
		}

		bool	setFlagged(bool bFlagged)
		{
			if (bFlagged)
				m_index |= FLAGGED_MASK;
			else
				m_index &= ~FLAGGED_MASK;
			return true;
		}

		bool	setFocused(bool bFocused)
		{
			if (m_index & DISABLED_MASK)
				return false;

			if (bFocused)
				m_index |= FOCUSED_MASK;
			else
				m_index &= ~FOCUSED_MASK;
			return true;
		}

		bool	setHovered(bool bHovered)
		{
			if (m_index & DISABLED_MASK)
				return false;

			if (!bHovered)
				setMouseInteraction(MouseInteraction::None);
			else if (getMouseInteraction() == MouseInteraction::None)
				setMouseInteraction(MouseInteraction::Hovered);

			return true;
		}

		bool	setPressed(bool bPressed)
		{
			if (m_index & DISABLED_MASK)
				return false;

			if (bPressed)
				setMouseInteraction(MouseInteraction::Pressed);
			else if (getMouseInteraction() == MouseInteraction::Pressed)
				setMouseInteraction(MouseInteraction::Hovered);

			return true;
		}

		bool	setTargeted(bool bTargeted)
		{
			if (m_index & DISABLED_MASK)
				return false;

			if (bTargeted)
				setMouseInteraction(MouseInteraction::Targeted);
			else if (getMouseInteraction() == MouseInteraction::Targeted)
				setMouseInteraction(MouseInteraction::Hovered);

			return true;
		}

		constexpr bool	isChecked() const { return m_index & CHECKED_MASK; }
		constexpr bool	isFlagged() const { return m_index & FLAGGED_MASK; }
		constexpr bool	isDisabled() const { return m_index & DISABLED_MASK; }
		constexpr bool	isSelected() const { return m_index & SELECTED_MASK; }
		constexpr bool	isFocused() const { return m_index & FOCUSED_MASK; }
		constexpr bool	isHovered() const { return getMouseInteraction() != MouseInteraction::None; }
		constexpr bool	isPressed() const { return getMouseInteraction() == MouseInteraction::Pressed; }
		constexpr bool	isTargeted() const { return getMouseInteraction() == MouseInteraction::Targeted; }

		//._____ Operators _____________________________________________________

		constexpr bool operator==(State state) const { return m_index == state.m_index; }
		constexpr bool operator!=(State state) const { return m_index != state.m_index; }

		constexpr operator int() const { return m_index; }

		State operator+(State state) const
		{
			State s;
			s.m_index = m_index | state.m_index;

			if (s.m_index & DISABLED_MASK)
			{
				s.m_index &= ~FOCUSED_MASK;
				s.setMouseInteraction(MouseInteraction::None);
			}
			else
			{
				MouseInteraction myMouse = getMouseInteraction();
				MouseInteraction otherMouse = state.getMouseInteraction();
				s.setMouseInteraction(std::max(myMouse, otherMouse));
			}

			return s;
		}

		State operator-(State state) const
		{
			State s;

			s.m_index = m_index & ~state.m_index;

			if (s.m_index & DISABLED_MASK)
			{
				s.m_index &= ~FOCUSED_MASK;
				s.setMouseInteraction(MouseInteraction::None);
			}
			else
			{
				MouseInteraction myMouse = getMouseInteraction();
				MouseInteraction stateMouse = state.getMouseInteraction();

				if (stateMouse == MouseInteraction::Hovered)
					s.setMouseInteraction(MouseInteraction::None);
				else if (stateMouse > MouseInteraction::Hovered && stateMouse != myMouse)
					s.setMouseInteraction(myMouse);
			}
			return s;
		}

		State& operator+=(State state)
		{
			MouseInteraction oldMouseInteraction = getMouseInteraction();

			m_index |= state.m_index;

			if (m_index & DISABLED_MASK)
			{
				m_index &= ~FOCUSED_MASK;
				setMouseInteraction(MouseInteraction::None);
			}
			else
			{
				setMouseInteraction(std::max(state.getMouseInteraction(), oldMouseInteraction));
			}

			return *this;
		}

		State& operator-=(State state)
		{
			MouseInteraction oldMouseInteraction = getMouseInteraction();

			m_index = m_index & ~state.m_index;

			if (m_index & DISABLED_MASK)
			{
				m_index &= ~FOCUSED_MASK;
				setMouseInteraction(MouseInteraction::None);
			}
			else
			{
				MouseInteraction stateMouse = state.getMouseInteraction();

				if (stateMouse == MouseInteraction::Hovered)
					setMouseInteraction(MouseInteraction::None);
				else if (stateMouse > MouseInteraction::Hovered && stateMouse != oldMouseInteraction)
					setMouseInteraction(oldMouseInteraction);
			}

			return *this;
		}

		//.____ Misc ________________________________________________

		constexpr uint8_t	index() const { return m_index; }
		constexpr StateEnum value() const { return m_enum; }

		constexpr uint8_t	weight() const { return s_weights[m_index]; }
		Bitmask<uint8_t>	primStates() const { return s_weights[m_index]; }

		int					bestMatch(int nAlternatives, const State* pAlternatives);


		//.____ Definitions _________________________________________

		static const State DisabledFlaggedCheckedSelected;
		static const State DisabledFlaggedChecked;
		static const State DisabledFlaggedSelected;
		static const State DisabledFlagged;
		static const State DisabledCheckedSelected;
		static const State DisabledChecked;
		static const State DisabledSelected;
		static const State Disabled;
		static const State TargetedFlaggedCheckedSelectedFocused;
		static const State TargetedFlaggedCheckedSelected;
		static const State TargetedFlaggedCheckedFocused;
		static const State TargetedFlaggedChecked;
		static const State TargetedFlaggedSelectedFocused;
		static const State TargetedFlaggedSelected;
		static const State TargetedFlaggedFocused;
		static const State TargetedFlagged;
		static const State TargetedCheckedSelectedFocused;
		static const State TargetedCheckedSelected;
		static const State TargetedCheckedFocused;
		static const State TargetedChecked;
		static const State TargetedSelectedFocused;
		static const State TargetedSelected;
		static const State TargetedFocused;
		static const State Targeted;
		static const State FlaggedCheckedSelectedPressedFocused;
		static const State FlaggedCheckedSelectedPressed;
		static const State FlaggedCheckedSelectedHoveredFocused;
		static const State FlaggedCheckedSelectedHovered;
		static const State FlaggedCheckedSelectedFocused;
		static const State FlaggedCheckedSelected;
		static const State FlaggedCheckedPressedFocused;
		static const State FlaggedCheckedPressed;
		static const State FlaggedCheckedHoveredFocused;
		static const State FlaggedCheckedHovered;
		static const State FlaggedCheckedFocused;
		static const State FlaggedChecked;
		static const State FlaggedSelectedPressedFocused;
		static const State FlaggedSelectedPressed;
		static const State FlaggedSelectedHoveredFocused;
		static const State FlaggedSelectedHovered;
		static const State FlaggedSelectedFocused;
		static const State FlaggedSelected;
		static const State FlaggedPressedFocused;
		static const State FlaggedPressed;
		static const State FlaggedHoveredFocused;
		static const State FlaggedHovered;
		static const State FlaggedFocused;
		static const State Flagged;
		static const State CheckedSelectedPressedFocused;
		static const State CheckedSelectedPressed;
		static const State CheckedSelectedHoveredFocused;
		static const State CheckedSelectedHovered;
		static const State CheckedSelectedFocused;
		static const State CheckedSelected;
		static const State CheckedPressedFocused;
		static const State CheckedPressed;
		static const State CheckedHoveredFocused;
		static const State CheckedHovered;
		static const State CheckedFocused;
		static const State Checked;
		static const State SelectedPressedFocused;
		static const State SelectedPressed;
		static const State SelectedHoveredFocused;
		static const State SelectedHovered;
		static const State SelectedFocused;
		static const State Selected;
		static const State PressedFocused;
		static const State Pressed;
		static const State HoveredFocused;
		static const State Hovered;
		static const State Focused;
		static const State Default;


		static const int	NbStates = 72;			// Number of state combinations
		static const int	MaxValue = 71;				// Highest value for m_index

		static const uint8_t	s_weights[NbStates];

	private:

		enum class MouseInteraction : uint8_t
		{
			None = 0,
			Hovered = 1,
			Pressed = 2,
			Targeted = 3,
		};

		// Bit positions (LSB to MSB)
		static constexpr uint8_t FLAGGED_BIT = 0;
		static constexpr uint8_t SELECTED_BIT = 1;
		static constexpr uint8_t CHECKED_BIT = 2;
		static constexpr uint8_t FOCUSED_BIT = 3;
		static constexpr uint8_t MOUSE_INTERACTION_BIT = 4;  // 2 bits: positions 4-5
		static constexpr uint8_t DISABLED_BIT = 6;

		// Bit masks
		static constexpr uint8_t FOCUSED_MASK = 1 << FOCUSED_BIT;
		static constexpr uint8_t MOUSE_INTERACTION_MASK = 0b11 << MOUSE_INTERACTION_BIT;
		static constexpr uint8_t SELECTED_MASK = 1 << SELECTED_BIT;
		static constexpr uint8_t CHECKED_MASK = 1 << CHECKED_BIT;
		static constexpr uint8_t FLAGGED_MASK = 1 << FLAGGED_BIT;
		static constexpr uint8_t DISABLED_MASK = 1 << DISABLED_BIT;

		// Helper functions for mouse interaction
		constexpr MouseInteraction getMouseInteraction() const {
			return static_cast<MouseInteraction>((m_index & MOUSE_INTERACTION_MASK) >> MOUSE_INTERACTION_BIT);
		}

		void setMouseInteraction(MouseInteraction value) {
			m_index = (m_index & ~MOUSE_INTERACTION_MASK) |
				(static_cast<uint8_t>(value) << MOUSE_INTERACTION_BIT);
		}

		union
		{
			uint8_t		m_index;
			StateEnum	m_enum;
		};
	};

}

#endif	//WG_STATE_DOT_H
