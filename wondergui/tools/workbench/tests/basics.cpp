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

// A few simple tests that show how tests are written.

#include "../workbench.h"

#include <widgetkits/wg_oldskool.h>

#include <string>

using namespace wg;

//____ Buttons ________________________________________________________________

WORKBENCH_TEST("Basics", "Buttons")
{
	auto pPanel = PackPanel::create(WGBP(PackPanel,
		_.axis = Axis::Y,
		_.slotAlignment = Alignment::Begin,
		_.spacing = 8,
		_.spacingBefore = 16
	));

	auto pStatus = TextDisplay::create(WGBP(TextDisplay, _.display.text = "Nothing clicked yet."));

	auto pButton = oldskool::Button::create(WGBP(oldskool::Button, _.label.text = "Click me"));
	auto pToggle = oldskool::ToggleButton::create(WGBP(oldskool::ToggleButton, _.label.text = "Toggle me"));
	auto pCheckbox = oldskool::Checkbox::create(WGBP(oldskool::Checkbox, _.label.text = "Check me"));

	// Routes added through ctx are removed when the test is left.

	ctx.addRoute(pButton, MsgType::Select, [pStatus, clicks = 0](Msg* pMsg) mutable
	{
		pStatus->display.setText(("Button clicked " + std::to_string(++clicks) + " times.").c_str());
	});

	ctx.addRoute(pToggle, MsgType::Toggle, [pStatus](Msg* pMsg)
	{
		bool bChecked = static_cast<ToggleMsg*>(pMsg)->isChecked();
		pStatus->display.setText(bChecked ? "Toggle button checked." : "Toggle button unchecked.");
	});

	ctx.addRoute(pCheckbox, MsgType::Toggle, [pStatus](Msg* pMsg)
	{
		bool bChecked = static_cast<ToggleMsg*>(pMsg)->isChecked();
		pStatus->display.setText(bChecked ? "Checkbox checked." : "Checkbox unchecked.");
	});

	pPanel->slots.pushBack({ pButton, pToggle, pCheckbox, pStatus });
	pPanel->setSlotWeight(pPanel->slots.begin(), pPanel->slots.end(), 0.f);
	return pPanel;
}

//____ Update callback ________________________________________________________

WORKBENCH_TEST("Basics", "Update callback")
{
	auto pDisplay = TextDisplay::create(WGBP(TextDisplay,
		_.display.layout = oldskool::TextLayouts::CenteredNoWrap,
		_.display.style = oldskool::TextStyles::Heading1
	));

	// Called every frame while the test runs.

	ctx.onUpdate([pDisplay, start = int64_t(-1)](int64_t microsec) mutable
	{
		if (start < 0)
			start = microsec;

		char buffer[64];
		snprintf(buffer, sizeof(buffer), "%.1f seconds", (microsec - start) / 1000000.0);
		pDisplay->display.setText(buffer);
	});

	return pDisplay;
}

//____ Text editor ____________________________________________________________

WORKBENCH_TEST("Text", "Text editor")
{
	auto pEditor = oldskool::TextEditor::create();
	pEditor->editor.setText("Type here.\n\nThis is a TextEditor from the oldskool widget kit.");

	auto pCapsule = PaddingCapsule::create(WGBP(PaddingCapsule, _.padding = 16));
	pCapsule->slot = pEditor;
	return pCapsule;
}

//____ Box skins ______________________________________________________________

WORKBENCH_TEST("Skins", "Box skins")
{
	auto pPanel = PackPanel::create(WGBP(PackPanel,
		_.axis = Axis::X,
		_.slotAlignment = Alignment::Begin,
		_.spacing = 16,
		_.spacingAfter = 16,
		_.spacingBefore = 16
	));

	auto addBox = [&](const BoxSkin::Blueprint& bp)
	{
		pPanel->slots.pushBack(Filler::create(WGBP(Filler,
			_.defaultSize = { 120, 120 },
			_.skin = BoxSkin::create(bp)
		)), { .weight = 0.f });
	};

	addBox({ .color = Color::LightBlue, .outlineColor = Color::DarkBlue, .outlineThickness = 2 });
	addBox({ .color = Color::LightGreen, .cornerRadius = 20, .outlineColor = Color::DarkGreen, .outlineThickness = 4 });
	addBox({ .color = Color::Transparent, .cornerRadius = 60, .outlineColor = Color::Red, .outlineThickness = 8 });

	return pPanel;
}
