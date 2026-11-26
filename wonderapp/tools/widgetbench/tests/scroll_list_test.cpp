
// Demonstrates a scrollable panel full of items - handy as a starting point
// for testing layout, skins or list-like widgets against real content.

#include <string>

#include "../test.h"

using namespace wg;

namespace
{
	void build(TestContext& ctx)
	{
		auto pItemSkin = BoxSkin::create({
			.color = Color::LightYellow,
			.outlineColor = Color::Black,
			.outlineThickness = 1,
			.padding = 8,
			.states = {
				{State::Hovered, Color::LightCyan}
			}
		});

		auto pList = PackPanel::create({ .axis = Axis::Y, .spacing = 4 });

		for (int i = 1; i <= 30; i++)
		{
			auto pLabel = TextDisplay::create({
				.display = { .style = wkit::TextStyles::Default, .text = ("Item " + std::to_string(i)).c_str() },
				.skin = pItemSkin
			});

			pList->slots.pushBack(pLabel, { .weight = 0 });
		}

		auto pScroll = ScrollPanel::create({
			.scrollbarY = {
				.back = wkit::Skins::ScrollbarTrack,
				.backwardButton = wkit::Skins::ScrollbarButtonUp,
				.bar = wkit::Skins::ScrollbarHandleY,
				.forwardButton = wkit::Skins::ScrollbarButtonDown
			}
		});

		pScroll->slot = pList;

		ctx.pContent->slot = PaddingCapsule::create({ .child = pScroll, .padding = 16 });
	}
}

REGISTER_TEST("Scroll List", build)
