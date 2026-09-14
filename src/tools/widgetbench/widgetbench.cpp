
#include "widgetbench.h"

#include <algorithm>
#include <string>

using namespace wg;
using namespace wapp;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(API* pAPI)
{
	m_pAPI = pAPI;

	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return m_pWindow != nullptr;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(Window* pWindow)
{
	if (pWindow == m_pWindow)
		m_pWindow = nullptr;
}

//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(API* pAPI)
{
	m_pWindow = Window::create(pAPI, { .size = {1100,750}, .title = "Widget Bench" });

	if (!pAPI->initDefaultWidgetKit())
		return false;

	m_pWindow->root()->setSkin(ColorSkin::create(wkit::Colors::Canvas));

	// Collect and alphabetically sort all self-registered tests.

	for (auto& test : TestRegistry::instance().tests())
		m_sortedTests.push_back(&test);

	sort(m_sortedTests.begin(), m_sortedTests.end(),
		[](const Test* pA, const Test* pB) { return pA->name < pB->name; });

	// Picker, sits at the top of the window.

	m_pPicker = SelectBox::create({
		.entrySkin = wkit::Skins::SelectBoxEntry,
		.entryTextStyle = wkit::TextStyles::Default,
		.listSkin = wkit::Skins::Plate,
		.skin = wkit::Skins::SelectBox
	});

	int id = 0;
	for (auto pTest : m_sortedTests)
		m_pPicker->entries << SelectBoxEntry::Blueprint{ .id = id++, .text = pTest->name.c_str() };

	Base::msgRouter()->addRoute(m_pPicker, MsgType::Select, [this](Msg*)
		{
			_selectTest(m_pPicker->selectedEntryIndex());
		});

	// Content area below the picker, holds whatever the active test builds.

	m_pContent = Capsule::create();

	auto pLayout = PackPanel::create({ .axis = Axis::Y, .spacing = 8 });
	pLayout->slots << m_pPicker;
	pLayout->slots << m_pContent;
	pLayout->slots[0].setWeight(0);
	pLayout->slots[1].setWeight(1);

	m_pWindow->mainCapsule()->slot = PaddingCapsule::create({ .child = pLayout, .padding = 8 });

	if (!m_sortedTests.empty())
	{
		m_pPicker->selectEntryByIndex(0);
		_selectTest(0);
	}

	return true;
}

//____ _selectTest() __________________________________________________________

void MyApp::_selectTest(int index)
{
	if (index < 0 || index >= (int) m_sortedTests.size())
		return;

	m_pContent->slot = nullptr;

	TestContext ctx{ m_pAPI, m_pContent };
	m_sortedTests[index]->build(ctx);
}
