
// Demonstrates an interactive widget and hooking up its messages -
// a reasonable template for testing anything clickable.

#include "../test.h"

#include <memory>
#include <string>

using namespace wg;

namespace
{
	void build(TestContext& ctx)
	{
		auto pCountDisplay = TextDisplay::create({
			.display = { .style = wkit::TextStyles::Default, .text = "Clicks: 0" }
		});

		auto pButton = Button::create({
			.label = { .style = wkit::TextStyles::Default, .text = "Click me" },
			.skin = wkit::Skins::Button
		});

		auto pDisabledButton = Button::create({
			.disabled = true,
			.label = { .style = wkit::TextStyles::Default, .text = "Disabled" },
			.skin = wkit::Skins::Button
		});

		auto pClicks = std::make_shared<int>(0);

		Base::msgRouter()->addRoute(pButton, MsgType::Select, [pCountDisplay, pClicks](Msg*)
			{
				(*pClicks)++;
				pCountDisplay->display.setText(("Clicks: " + std::to_string(*pClicks)).c_str());
			});

		auto pRow = PackPanel::create({ .axis = Axis::X, .spacing = 8 });
		pRow->slots.pushBack(pButton, { .weight = 0 });
		pRow->slots.pushBack(pDisabledButton, { .weight = 0 });
		pRow->slots.pushBack(pCountDisplay, { .weight = 0 });

		ctx.pContent->slot = PaddingCapsule::create({ .child = pRow, .padding = 16 });
	}
}

REGISTER_TEST("Buttons", build)
