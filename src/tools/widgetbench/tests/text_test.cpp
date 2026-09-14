
// Shortest possible test - a starting point to copy for new ones.

#include "../test.h"

using namespace wg;

namespace
{
	void build(TestContext& ctx)
	{
		auto pDisplay = TextDisplay::create({
			.display = {
				.layout = wkit::TextLayouts::CenteredNoWrap,
				.style = wkit::TextStyles::Heading2,
				.text = "The quick brown fox jumps over the lazy dog."
			},
			.skin = wkit::Skins::Canvas
		});

		ctx.pContent->slot = pDisplay;
	}
}

REGISTER_TEST("Text Display", build)
