// Checkbox state coverage.
//
// Skins::Checkbox carries eight blocks: Default, Hovered, Pressed, Checked,
// CheckedHovered, CheckedPressed, Disabled, DisabledChecked. The checked
// family is deliberately drawn on the UNCHECKED family's background, so the
// surface only ever says what the pointer is doing and the tick says whether
// the box is on. The tick also sinks 1pt down-right while the box is held.
//
// Row 1 forces each state on the skin directly, so all eight are visible at
// once without having to produce them. Row 2 is live, for poking at.

#include "../test.h"

#include <wg_skindisplay.h>   // not pulled in by wondergui.h

using namespace wg;

namespace
{
	Widget_p labelled(const char* text, Widget_p pWidget)
	{
		auto pLabel = TextDisplay::create({
			.display = { .style = wkit::TextStyles::FinePrint, .text = text }
		});

		auto pColumn = PackPanel::create({ .axis = Axis::Y, .spacing = 4 });
		pColumn->slots.pushBack(pWidget, { .weight = 0 });
		pColumn->slots.pushBack(pLabel, { .weight = 0 });
		return pColumn;
	}

	Widget_p stateSample(const char* name, State state)
	{
		return labelled(name, SkinDisplay::create({
			.displaySkin = wkit::Skins::Checkbox,
			.displayState = state
		}));
	}

	void build(TestContext& ctx)
	{
		// --- Row 1: the skin, with each state forced ------------------------

		auto pStates = PackPanel::create({ .axis = Axis::X, .spacing = 12 });
		pStates->slots.pushBack(stateSample("Default", State::Default), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Hovered", State::Hovered), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Pressed", State::Pressed), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Checked", State::Checked), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Chk+Hov", State::CheckedHovered), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Chk+Prs", State::CheckedPressed), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Disabled", State::Disabled), { .weight = 0 });
		pStates->slots.pushBack(stateSample("Dis+Chk", State::DisabledChecked), { .weight = 0 });

		// --- Row 2: live checkboxes -----------------------------------------
		//
		// Skins::Checkbox is the widget's ICON, not its skin -- the checkbox
		// itself is unskinned. Setting it as the skin instead would stretch the
		// 14x14 block, which has no frame, across the whole widget.

		auto pUnchecked = wkit::Checkbox::create({
			.label = { .style = wkit::TextStyles::NormalDark, .text = "unchecked" }
		});

		auto pChecked = wkit::Checkbox::create({
			.checked = true,
			.label = { .style = wkit::TextStyles::NormalDark, .text = "checked" }
		});

		auto pDisabled = wkit::Checkbox::create({
			.disabled = true,
			.label = { .style = wkit::TextStyles::NormalDark, .text = "disabled" }
		});

		auto pDisabledChecked = wkit::Checkbox::create({
			.checked = true,
			.disabled = true,
			.label = { .style = wkit::TextStyles::NormalDark, .text = "disabled + checked" }
		});

		auto pLive = PackPanel::create({ .axis = Axis::X, .spacing = 16 });
		pLive->slots.pushBack(pUnchecked, { .weight = 0 });
		pLive->slots.pushBack(pChecked, { .weight = 0 });
		pLive->slots.pushBack(pDisabled, { .weight = 0 });
		pLive->slots.pushBack(pDisabledChecked, { .weight = 0 });

		// --- Assemble -------------------------------------------------------

		auto pColumn = PackPanel::create({ .axis = Axis::Y, .spacing = 16 });
		pColumn->slots.pushBack(pStates, { .weight = 0 });
		pColumn->slots.pushBack(pLive, { .weight = 0 });

		ctx.pContent->slot = PaddingCapsule::create({ .child = pColumn, .padding = 16 });
	}
}

REGISTER_TEST("Checkbox states", build)
