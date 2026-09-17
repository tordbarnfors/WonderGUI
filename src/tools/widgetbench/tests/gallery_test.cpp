// Every widget the glossyblue kit wraps, laid out for poking at.
//
// The whole gallery sits inside a ScrollCapsuleY, which both keeps it in the
// window and gives the vertical scrollbar something long to scroll -- so the
// page you are reading is itself one of the widgets under test.
//
// Captions go straight in the blueprint -- `.label = { .text = "Push me" }` --
// and that is worth a note, because it did NOT work when this file was first
// written. A designated initializer for a nested Blueprint builds a whole
// fresh struct, so it silently discards the kit's defaults for it, including
// `.style`; and a DynamicText with no style has no font and draws nothing at
// all, which is how every button in this gallery came up captionless. The
// widgetkit's wrapper constructors now fill that gap (`if( !label.style() )
// label.setStyle(...)`), with Base::setDefaultStyle() as a second net, so a
// blueprint that mentions only `.text` gets the kit's intended style back.
//
// Still worth knowing: SizeCapsule's defaultSize takes -1, not 0, for "ask the
// child". 0 is a valid forced size, so `{260, 0}` collapses the widget to
// nothing.
//
// Not covered: ListTable (wkit::ListTable / TablePanel) needs columns and rows
// set up to say anything useful, so it wants its own test.

#include "../test.h"

#include <wg_skindisplay.h>     // not pulled in by wondergui.h

#include <string>

using namespace wg;

namespace
{
	const pts kGap = 10;

	Widget_p label(const char* text, TextStyle_p pStyle)
	{
		return TextDisplay::create({ .display = { .style = pStyle, .text = text } });
	}

	// A heading with its row of widgets underneath.
	Widget_p section(const char* heading, Widget_p pContent)
	{
		auto pColumn = PackPanel::create({ .axis = Axis::Y, .spacing = 6 });
		pColumn->slots.pushBack(label(heading, wkit::TextStyles::Heading5), { .weight = 0 });
		pColumn->slots.pushBack(pContent, { .weight = 0 });
		return pColumn;
	}

	Widget_p row(pts spacing = kGap)
	{
		return PackPanel::create({ .axis = Axis::X, .spacing = spacing });
	}

	void add(const Widget_p& pRow, Widget_p pChild)
	{
		static_cast<PackPanel*>(pRow.rawPtr())->slots.pushBack(pChild, { .weight = 0 });
	}

	// -1 means "ask the child" on that axis. Used wherever a widget needs a
	// deliberate size: a scroll capsule's default size is its child's and a
	// split panel's is its children's, so left alone they would either fill the
	// window or collapse, and the handles would have no travel.
	Widget_p sized(Size size, Widget_p pChild)
	{
		return SizeCapsule::create({ .child = pChild, .defaultSize = size });
	}

	Widget_p block(Size size, Skin_p pSkin)
	{
		return Filler::create({ .defaultSize = size, .skin = pSkin });
	}

	//____ sections ___________________________________________________________

	Widget_p buttons()
	{
		auto p = row();
		add(p, wkit::Button::create({ .label = { .text = "Push me" } }));
		add(p, wkit::Button::create({ .disabled = true, .label = { .text = "Disabled" } }));
		add(p, wkit::ToggleButton::create({ .label = { .text = "Toggle me" } }));
		add(p, wkit::ToggleButton::create({ .checked = true, .disabled = true, .label = { .text = "Disabled" } }));
		return p;
	}

	Widget_p checkboxes()
	{
		auto p = row();
		add(p, wkit::Checkbox::create({ .label = { .text = "Unchecked" } }));
		add(p, wkit::Checkbox::create({ .checked = true, .label = { .text = "Checked" } }));
		add(p, wkit::Checkbox::create({ .disabled = true, .label = { .text = "Disabled" } }));
		add(p, wkit::Checkbox::create({ .checked = true, .disabled = true, .label = { .text = "Disabled + checked" } }));
		return p;
	}

	Widget_p radioButtons()
	{
		// Three in one ToggleGroup: checking one unchecks the others, and the
		// group refuses to let the checked one be unchecked by clicking it
		// again (requireChecked defaults true). The fourth is deliberately
		// OUTSIDE the group -- ungrouped, a radio button behaves like a plain
		// toggle and can be switched off, which is the one case where its
		// unchecked-and-pressed state can be reached.
		auto pGroup = ToggleGroup::create();

		auto pA = wkit::RadioButton::create({ .checked = true, .label = { .text = "First" } });
		auto pB = wkit::RadioButton::create({ .label = { .text = "Second" } });
		auto pC = wkit::RadioButton::create({ .label = { .text = "Third" } });

		pGroup->add(pA);
		pGroup->add(pB);
		pGroup->add(pC);

		auto pSelected = TextDisplay::create({
			.display = { .style = wkit::TextStyles::NormalDark, .text = "selected: First" }
		});

		auto watch = [pSelected](const char* name)
			{
				pSelected->display.setText((std::string("selected: ") + name).c_str());
			};
		auto pRouter = Base::msgRouter();
		pRouter->addRoute(pA, MsgType::Toggle, [watch](Msg*) { watch("First"); });
		pRouter->addRoute(pB, MsgType::Toggle, [watch](Msg*) { watch("Second"); });
		pRouter->addRoute(pC, MsgType::Toggle, [watch](Msg*) { watch("Third"); });

		auto p = row(16);
		add(p, pA);
		add(p, pB);
		add(p, pC);
		add(p, wkit::RadioButton::create({ .label = { .text = "(ungrouped)" } }));
		add(p, wkit::RadioButton::create({ .checked = true, .disabled = true, .label = { .text = "disabled" } }));
		add(p, pSelected);
		return p;
	}

	Widget_p selectBox()
	{
		auto pBox = wkit::SelectBox::create();
		const char* entries[] = { "First entry", "Second entry", "Third entry",
								  "Fourth entry", "Fifth entry" };
		int id = 0;
		for (auto text : entries)
			pBox->entries << SelectBoxEntry::Blueprint{ .id = id++, .text = text };
		pBox->selectEntryByIndex(0);

		auto pEmpty = wkit::SelectBox::create({ .disabled = true });

		auto p = row();
		add(p, sized({ 180, -1 }, pBox));
		add(p, sized({ 120, -1 }, pEmpty));
		return p;
	}

	Widget_p textFields()
	{
		// No `.style` here either: the wrappers fill the editor's style the same
		// way they fill a button's label.
		auto p = row();
		add(p, sized({ 200, -1 }, wkit::LineEditor::create({
			.editor = { .text = "Editable line" } })));
		add(p, sized({ 320, 90 }, wkit::TextEditor::create({
			.editor = { .text = "A TextEditor.\nSelect some text to check the selection "
								"colour, and type to check the caret." } })));
		return p;
	}

	Widget_p scrollbars()
	{
		// A child much bigger than the viewport in both directions, so both
		// scrollbars appear with short handles and plenty of travel.
		auto pBoth = wkit::ScrollCapsuleXY::create({
			.child = block({ 1200, 700 }, wkit::Skins::Window)
		});

		// Tall and narrow: one long vertical bar.
		auto pVertical = wkit::ScrollCapsuleY::create({
			.child = block({ 100, 900 }, wkit::Skins::Window)
		});

		auto p = row();
		add(p, sized({ 420, 200 }, pBoth));
		add(p, sized({ 130, 200 }, pVertical));
		return p;
	}

	Widget_p splitPanels()
	{
		auto pX = wkit::SplitPanelX::create();
		pX->slots[0] = block({ 100, 60 }, wkit::Skins::Plate);
		pX->slots[1] = block({ 100, 60 }, wkit::Skins::Canvas);

		auto pY = wkit::SplitPanelY::create();
		pY->slots[0] = block({ 60, 100 }, wkit::Skins::Plate);
		pY->slots[1] = block({ 60, 100 }, wkit::Skins::Canvas);

		auto p = row();
		add(p, sized({ 260, 150 }, pX));
		add(p, sized({ 150, 150 }, pY));
		return p;
	}

	Widget_p drawer()
	{
		// The only place PlusMinusToggle shows up: DrawerPanel draws it as its
		// button. slots[0] is the always-visible header, slots[1] the drawer.
		// The header is padded left to clear the 14pt button.
		auto pDrawer = wkit::TreeListDrawer::create();

		pDrawer->slots[0] = PaddingCapsule::create({
			.child = label("Click the +/- to fold", wkit::TextStyles::NormalDark),
			.padding = { 2, 2, 2, 20 }
		});

		auto pContents = PackPanel::create({ .axis = Axis::Y, .spacing = 4 });
		pContents->slots.pushBack(label("First drawer line", wkit::TextStyles::NormalDark), { .weight = 0 });
		pContents->slots.pushBack(label("Second drawer line", wkit::TextStyles::NormalDark), { .weight = 0 });
		pContents->slots.pushBack(label("Third drawer line", wkit::TextStyles::NormalDark), { .weight = 0 });

		pDrawer->slots[1] = PaddingCapsule::create({
			.child = pContents,
			.padding = { 4, 4, 4, 20 },
			.skin = wkit::Skins::Canvas
		});

		// Starts closed otherwise, which looks like the drawer is missing.
		pDrawer->openImmediately();

		return sized({ 260, -1 }, pDrawer);
	}

	Widget_p capsules()
	{
		auto p = row();
		add(p, wkit::LabelAndFrameCapsule::create({
			.child = block({ 120, 50 }, wkit::Skins::Canvas),
			.label = { .text = "Label and frame" } }));
		add(p, wkit::LabeledSectionCapsule::create({
			.child = block({ 120, 50 }, wkit::Skins::Canvas),
			.label = { .text = "Labeled section" } }));
		return p;
	}

	Widget_p surfaces()
	{
		// The passive skins, forced onto SkinDisplays so they can be compared
		// side by side rather than hunted for behind other widgets.
		auto p = row();
		struct { const char* name; Skin_p skin; } items[] = {
			{ "Plate",        wkit::Skins::Plate },
			{ "PlateNoBevel", wkit::Skins::PlateNoBevel },
			{ "Canvas",       wkit::Skins::Canvas },
			{ "Field",        wkit::Skins::Field },
			{ "Window",       wkit::Skins::Window },
			{ "Titlebar",     wkit::Skins::Titlebar },
			{ "ScrollbarTrack", wkit::Skins::ScrollbarTrack },
		};
		for (auto& item : items)
		{
			auto pColumn = PackPanel::create({ .axis = Axis::Y, .spacing = 4 });
			pColumn->slots.pushBack(
				sized({ 90, 44 }, SkinDisplay::create({ .displaySkin = item.skin })),
				{ .weight = 0 });
			pColumn->slots.pushBack(label(item.name, wkit::TextStyles::FinePrint), { .weight = 0 });
			add(p, pColumn);
		}
		return p;
	}

	//____ build ______________________________________________________________

	void build(TestContext& ctx)
	{
		auto pColumn = PackPanel::create({ .axis = Axis::Y, .spacing = 18 });

		pColumn->slots.pushBack(
			wkit::WindowTitleBar::create({ .display = { .text = "glossyblue widget gallery" } }),
			{ .weight = 0 });

		pColumn->slots.pushBack(section("Buttons", buttons()), { .weight = 0 });
		pColumn->slots.pushBack(section("Checkboxes", checkboxes()), { .weight = 0 });
		pColumn->slots.pushBack(section("Radio buttons (first three share a ToggleGroup)",
										radioButtons()), { .weight = 0 });
		pColumn->slots.pushBack(section("Select box", selectBox()), { .weight = 0 });
		pColumn->slots.pushBack(section("Text fields", textFields()), { .weight = 0 });
		pColumn->slots.pushBack(section("Scrollbars - drag the handles", scrollbars()), { .weight = 0 });
		pColumn->slots.pushBack(section("Split panels - drag the handles", splitPanels()), { .weight = 0 });
		pColumn->slots.pushBack(section("Drawer", drawer()), { .weight = 0 });
		pColumn->slots.pushBack(section("Capsules", capsules()), { .weight = 0 });
		pColumn->slots.pushBack(section("Surfaces", surfaces()), { .weight = 0 });

		auto pScroll = wkit::ScrollCapsuleY::create({
			.child = PaddingCapsule::create({ .child = pColumn, .padding = 16 })
		});

		ctx.pContent->slot = pScroll;
	}
}

REGISTER_TEST("Gallery - all widgets", build)
