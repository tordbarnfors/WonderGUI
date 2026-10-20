#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>

#include <wondergfx.h>
#include <wg_referencebackend.h>
#include <wg_softsurface.h>

#include "gfxdevicetester.h"

#include "testsuites/a8tests.h"
#include "testsuites/blendtests.h"
#include "testsuites/blendfixedcolortests.h"
#include "testsuites/blitblendtests.h"
#include "testsuites/blitconsistencytest.h"
#include "testsuites/blittests.h"
#include "testsuites/blurtests.h"
#include "testsuites/canvasformattests.h"
#include "testsuites/canvaslayertests.h"
#include "testsuites/colortests.h"
#include "testsuites/filltests.h"
#include "testsuites/linetests.h"
#include "testsuites/mipmaptests.h"
#include "testsuites/paletteblittests.h"
#include "testsuites/piecharttests.h"
#include "testsuites/segmenttests.h"
#include "testsuites/tiletests.h"
#include "testsuites/tintblittests.h"
#include "testsuites/tinttests.h"
#include "testsuites/wavetests.h"
#include "testsuites/edgemaptests.h"
#include "testsuites/edgemapringtests.h"
#include "testsuites/edgemapwavetests.h"
#include "testsuites/samplingtests.h"
#include "testsuites/palettecapacitytests.h"
#include "testsuites/blursourcetests.h"
#include "testsuites/readbacktests.h"
#include "testsuites/edgemaptinttests.h"
#include "testsuites/layerroundtriptests.h"
#include "testsuites/blendruletests.h"
#include "testsuites/roundedboxtests.h"
#include "testsuites/ringtests.h"
#include "testsuites/warptests.h"
#include "testsuites/flattenlayerstests.h"
#include "testsuites/linecaptests.h"


using namespace wg;
using namespace std;


//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return static_cast<WonderApp*>(new GfxDeviceTester());
}

//____ init() _________________________________________________________________

bool GfxDeviceTester::init(wapp::API* pAPI)
{
	m_pAPI = pAPI;

	m_pWindow = wapp::Window::create(pAPI, { .size = {1800,900}, .title = "GfxDevice Tester" });

	pAPI->initDefaultWidgetKit();

	setup_backends();
	setup_tests();

	if (!setup_chrome())
		return false;

	select_view(View::Display);

	// "--accuracy [--dump] [suite ...]" runs the accuracy tests of the given suites (all if none are
	// given) at the first update, prints the results and quits, for runs without anyone at the window.
	// With --dump, testee and reference images of tests that aren't OK are saved as PPM files.

	auto arguments = pAPI->programArguments();
	for (size_t i = 0; i < arguments.size(); i++)
	{
		if (arguments[i] == "--accuracy")
		{
			m_bBatchAccuracy = true;

			for (size_t j = i + 1; j < arguments.size(); j++)
			{
				if (arguments[j] == "--dump")
					m_bBatchDump = true;
				else
					m_batchSuites.push_back(arguments[j]);
			}
			break;
		}
	}

	// Start out showing the first test we can run.

	for (int i = 0; i < (int)m_tests.size(); i++)
	{
		if (m_suites[m_tests[i].suite].bWorking)
		{
			show_in_display(i);
			break;
		}
	}

	return true;
}

//____ update() _______________________________________________________________

bool GfxDeviceTester::update()
{
	if (m_bBatchAccuracy)
	{
		run_batch_accuracy();
		return false;
	}

	if (m_bAccuracyRunning)
		continue_accuracy_run();

	if (m_bPerformanceRunning)
		continue_performance_run();

	return m_pWindow != nullptr;
}

//____ exit() _________________________________________________________________

void GfxDeviceTester::exit()
{
	for (auto route : m_routes)
		Base::msgRouter()->deleteRoute(route);
	m_routes.clear();

	destroy_tests();

	for (auto& viewer : { &m_displayViewer, &m_accuracyViewer, &m_performanceViewer })
	{
		for (auto& p : viewer->pImages)
			p = nullptr;
	}

	for (auto& backend : m_backends)
		backend = Backend();
}

//____ closeWindow() __________________________________________________________

void GfxDeviceTester::closeWindow(wapp::Window* pWindow)
{
	m_pWindow = nullptr;
}

//____ setup_backends() _______________________________________________________

void GfxDeviceTester::setup_backends()
{
	Surface::Blueprint canvasBP = WGBP(Surface,
		_.size = m_canvasSize,
		_.canvas = true);

	// Testee: The native device of the WonderApp framework.

	{
		auto pDevice = Base::defaultGfxDevice();

		string name = string("Native (") + pDevice->typeInfo().className;

		if (pDevice->isInstanceOf(GfxDeviceGen2::TYPEINFO))
		{
			auto pBackend = static_cast<GfxDeviceGen2*>(pDevice.rawPtr())->backend();
			if (pBackend)
				name += string(" + ") + pBackend->typeInfo().className;
		}
		name += ")";

		m_backends[TESTEE] = { name, pDevice, Base::defaultSurfaceFactory()->createSurface(canvasBP) };
	}

	// Reference: GfxDeviceGen2 with ReferenceBackend.

	{
		auto pBackend = ReferenceBackend::create();

		m_backends[REFERENCE] = { "Reference (GfxDeviceGen2 + ReferenceBackend)", GfxDeviceGen2::create(pBackend), SoftSurface::create(canvasBP) };
	}

	// Both canvases are read back to ARGB_8 in the color space of the reference canvas
	// for comparison, so backends with different canvas formats can be compared.

	m_compareColorSpace = m_backends[REFERENCE].pCanvas->colorSpace();
}

//____ setup_tests() __________________________________________________________

void GfxDeviceTester::setup_tests()
{
	destroy_tests();

	auto add = [this](auto factory) { add_testsuite(factory(), factory()); };

	add([]() { return new BlurTests(); });
	add([]() { return new BlitConsistencyTests(); });
	add([]() { return new FillTests(); });
	add([]() { return new BlendTests(); });
	add([]() { return new LineTests(); });
	add([]() { return new CanvasFormatTests(); });
	add([]() { return new PatchBlitTests(); });
	add([]() { return new SegmentTests(); });
	add([]() { return new WaveTests(); });
	add([]() { return new PaletteBlitTests(); });
	add([]() { return new MipmapTests(); });
	add([]() { return new PieChartTests(); });
	add([]() { return new TintBlitTests(); });
	add([]() { return new TintTests(); });
	add([]() { return new BlitBlendTests(); });
	add([]() { return new A8Tests(); });
	add([]() { return new TileTests(); });
	add([]() { return new CanvasLayerTests(); });
	add([]() { return new BlendFixedColorTests(); });
	add([]() { return new EdgemapTests(); });
	add([]() { return new EdgemapRingTests(); });
	add([]() { return new EdgemapWaveTests(); });
	add([]() { return new ColorTests(); });
	add([]() { return new SamplingTests(); });
	add([]() { return new PaletteCapacityTests(); });
	add([]() { return new BlurSourceTests(); });
	add([]() { return new ReadbackTests(); });
	add([]() { return new EdgemapTintTests(); });
	add([]() { return new LayerRoundTripTests(); });
	add([]() { return new BlendRuleTests(); });
	add([]() { return new RoundedBoxTests(); });
	add([]() { return new RingTests(); });
	add([]() { return new WarpTests(); });
	add([]() { return new FlattenLayersTests(); });
	add([]() { return new LineCapTests(); });
}

//____ add_testsuite() ________________________________________________________

void GfxDeviceTester::add_testsuite(TestSuite* pTesteeSuite, TestSuite* pReferenceSuite)
{
	SuiteEntry suite;

	suite.name = pTesteeSuite->name;
	suite.pSuites[TESTEE] = pTesteeSuite;
	suite.pSuites[REFERENCE] = pReferenceSuite;
	suite.firstTest = (int)m_tests.size();
	suite.nbTests = (int)pTesteeSuite->tests.size();

	RectSPX canvas = { 0, 0, m_canvasSize.w * 64, m_canvasSize.h * 64 };

	bool bTesteeOk = pTesteeSuite->init(m_backends[TESTEE].pDevice, canvas, m_pAPI);
	bool bReferenceOk = pReferenceSuite->init(m_backends[REFERENCE].pDevice, canvas, m_pAPI);
	suite.bWorking = bTesteeOk && bReferenceOk;

	int suiteIndex = (int)m_suites.size();

	for (int i = 0; i < suite.nbTests; i++)
	{
		TestEntry test;

		test.name = pTesteeSuite->tests[i].name;
		test.suite = suiteIndex;
		test.pTests[TESTEE] = &pTesteeSuite->tests[i];
		test.pTests[REFERENCE] = &pReferenceSuite->tests[i];

		m_tests.push_back(test);
	}

	m_suites.push_back(suite);
}

//____ destroy_tests() ________________________________________________________

void GfxDeviceTester::destroy_tests()
{
	RectSPX canvas = { 0, 0, m_canvasSize.w * 64, m_canvasSize.h * 64 };

	for (auto& suite : m_suites)
	{
		for (int side = 0; side < 2; side++)
		{
			suite.pSuites[side]->exit(m_backends[side].pDevice, canvas);
			delete suite.pSuites[side];
		}
	}

	m_tests.clear();
	m_suites.clear();
}

//____ setup_chrome() _________________________________________________________

bool GfxDeviceTester::setup_chrome()
{
	m_pLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
		.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Weight });

	m_pRightAligned = BasicTextLayout::create({ .placement = Placement::East });

	auto verdictStyle = [](HiColor color) {
		auto bp = wkit::TextStyles::Strong->blueprint();
		bp.color = color;
		return TextStyle::create(bp);
	};

	m_pOkStyle = verdictStyle(Color::DarkGreen);
	m_pSpecksStyle = verdictStyle(Color::DarkOrange);
	m_pDiffStyle = verdictStyle(Color::Red);

	m_pCellSkin = ColorSkin::create({ .color = Color::Transparent, .padding = Border(2, 8) });
	m_pSelectedCellSkin = ColorSkin::create({ .color = Color::Aquamarine, .padding = Border(2, 8) });

	// Navigation bar with a button for each view.

	auto pNavBar = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .skin = wkit::Skins::Plate, .spacing = 4 });

	auto pViewGroup = ToggleGroup::create();

	const char* viewNames[3] = { "Display", "Accuracy", "Performance" };

	for (int i = 0; i < 3; i++)
	{
		auto pButton = WGCREATE(wkit::ToggleButton, _.label.text = viewNames[i]);
		pViewGroup->add(pButton);

		pNavBar->slots.pushBack(pButton, { .weight = 0 });

		View view = View(i);
		m_routes.push_back(Base::msgRouter()->addRoute(pButton, MsgType::Toggle, [this, view](Msg* pMsg) {
			if (static_cast<ToggleMsg*>(pMsg)->isChecked())
				select_view(view);
		}));

		m_pViewButtons[i] = pButton;
	}

	string backendInfo = "Testee: " + m_backends[TESTEE].name + "      Reference: " + m_backends[REFERENCE].name;

	auto pBackendInfo = TextDisplay::create({ .display = {.layout = m_pRightAligned, .style = wkit::TextStyles::Default, .text = backendInfo.c_str() } });
	pNavBar->slots.pushBack(pBackendInfo, { .weight = 1 });

	// The views, of which one is shown at a time.

	m_pViews[int(View::Display)] = create_display_view();
	m_pViews[int(View::Accuracy)] = create_accuracy_view();
	m_pViews[int(View::Performance)] = create_performance_view();

	m_pViewBase = PaddingCapsule::create();

	auto pRoot = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout, .skin = ColorSkin::create(Color::AntiqueWhite) });
	pRoot->slots.pushBack(pNavBar, { .weight = 0 });
	pRoot->slots.pushBack(m_pViewBase, { .weight = 1 });

	m_pWindow->mainCapsule()->slot = pRoot;

	// Clicks on the cells of the accuracy and performance tables select their row.
	// Each cell has the index of its test as id, headings have -1.

	m_routes.push_back(Base::msgRouter()->addRoute(MsgType::MouseClick, [this](Msg* pMsg) {
		auto pSource = pMsg->sourceRawPtr();
		if (!pSource || !pSource->isInstanceOf(Widget::TYPEINFO))
			return;

		auto pWidget = static_cast<Widget*>(pSource);
		if (pWidget->id() < 0)
			return;

		if (pWidget->parent() == m_pAccuracyTable)
			select_accuracy_row(pWidget->id());
		else if (pWidget->parent() == m_pPerformanceTable)
			select_performance_row(pWidget->id());
	}));

	return true;
}

//____ create_display_view() __________________________________________________

Widget_p GfxDeviceTester::create_display_view()
{
	create_viewer(m_displayViewer);

	// Background buttons.

	auto pTopBar = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .skin = wkit::Skins::Plate, .spacing = 4 });

	auto pBgLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = "Background: " } });
	pTopBar->slots.pushBack(pBgLabel, { .weight = 0 });

	auto pBgGroup = ToggleGroup::create();

	const char* bgNames[4] = { "Black", "White", "Grey", "Checkered" };

	for (int i = 0; i < 4; i++)
	{
		auto pButton = WGCREATE(wkit::ToggleButton, _.label.text = bgNames[i]);
		pBgGroup->add(pButton);
		if (i == 0)
			pButton->setChecked(true);

		pTopBar->slots.pushBack(pButton, { .weight = 0 });

		Background background = Background(i);
		m_routes.push_back(Base::msgRouter()->addRoute(pButton, MsgType::Toggle, [this, background](Msg* pMsg) {
			if (static_cast<ToggleMsg*>(pMsg)->isChecked() && m_displayViewer.background != background)
			{
				m_displayViewer.background = background;
				refresh_viewer(m_displayViewer);
			}
		}));
	}

	// Clip list buttons.

	auto pClipLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = "      Clip rects: " } });
	pTopBar->slots.pushBack(pClipLabel, { .weight = 0 });

	auto pClipGroup = ToggleGroup::create();

	const char* clipNames[3] = { "One", "Few", "Many" };

	for (int i = 0; i < 3; i++)
	{
		auto pButton = WGCREATE(wkit::ToggleButton, _.label.text = clipNames[i]);
		pClipGroup->add(pButton);
		if (i == 0)
			pButton->setChecked(true);

		pTopBar->slots.pushBack(pButton, { .weight = 0 });

		ClipList clipList = ClipList(i);
		m_routes.push_back(Base::msgRouter()->addRoute(pButton, MsgType::Toggle, [this, clipList](Msg* pMsg) {
			if (static_cast<ToggleMsg*>(pMsg)->isChecked() && m_displayViewer.clipList != clipList)
			{
				m_displayViewer.clipList = clipList;
				refresh_viewer(m_displayViewer);
			}
		}));
	}

	pTopBar->slots.pushBack(Filler::create(), { .weight = 1 });

	// The three boxes.

	auto pBoxes = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .spacing = 16 });

	for (int i = 0; i < 3; i++)
		pBoxes->slots.pushBack(create_image_box(m_displayViewer, i, 1.f), { .weight = 0 });

	auto pBoxesPadding = PaddingCapsule::create({ .child = pBoxes, .padding = 16 });

	auto pScroller = wkit::ScrollCapsuleXY::create();
	pScroller->slot = pBoxesPadding;

	auto pMain = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout });
	pMain->slots.pushBack(pTopBar, { .weight = 0 });
	pMain->slots.pushBack(m_displayViewer.pTitle, { .weight = 0 });
	pMain->slots.pushBack(pScroller, { .weight = 1 });

	// Test tree to the left.

	auto pBase = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout });
	pBase->slots.pushBack(create_test_tree(), { .weight = 0 });
	pBase->slots.pushBack(pMain, { .weight = 1 });

	return pBase;
}

//____ create_test_tree() _____________________________________________________

Widget_p GfxDeviceTester::create_test_tree()
{
	auto pList = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout });

	for (auto& suite : m_suites)
	{
		auto pDrawer = WGCREATE(wkit::TreeListDrawer, _.selectable = false);

		string title = suite.bWorking ? suite.name : suite.name + " (init failed)";

		auto pHeader = WGCREATE(wkit::TreeListEntry,
			_.child = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = title.c_str() } }),
			_.padding = Border(2, 4, 2, 20),
			_.skin = nullptr);

		auto pContent = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout });

		for (int i = suite.firstTest; i < suite.firstTest + suite.nbTests; i++)
		{
			auto pEntry = WGCREATE(wkit::TreeListEntry,
				_.child = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default, .text = m_tests[i].name.c_str() } }),
				_.id = i,
				_.padding = Border(2, 4, 2, 36),
				_.selectable = suite.bWorking,
				_.disabled = !suite.bWorking);

			pContent->slots.pushBack(pEntry, { .weight = 0 });
			m_tests[i].pTreeEntry = pEntry;
		}

		pDrawer->slots[0] = pHeader;
		pDrawer->slots[1] = pContent;

		pList->slots.pushBack(pDrawer, { .weight = 0 });
		suite.pTreeDrawer = pDrawer;
	}

	auto pSelector = SelectCapsule::create(WGBP(SelectCapsule,
		_.child = pList,
		_.recursive = true,
		_.selectMode = SelectMode::SingleEntry));

	m_pTestTreeSelector = pSelector;

	m_routes.push_back(Base::msgRouter()->addRoute(pSelector, MsgType::Selected, [this](Msg* pMsg) {
		auto p = static_cast<SelectedMsg*>(pMsg);
		if (p->nbSelected() > 0)
			show_test(m_displayViewer, p->selected()[0]->id());
	}));

	auto pScroller = wkit::ScrollCapsuleY::create();
	pScroller->slot = pSelector;

	auto pSidebar = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout, .skin = wkit::Skins::Plate });
	pSidebar->slots.pushBack(TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = "Tests" } }), { .weight = 0 });
	pSidebar->slots.pushBack(pScroller, { .weight = 1 });

	return pSidebar;
}

//____ create_viewer() ________________________________________________________

void GfxDeviceTester::create_viewer(Viewer& viewer)
{
	Surface::Blueprint bp = WGBP(Surface,
		_.size = m_canvasSize,
		_.format = PixelFormat::ARGB_8,
		_.colorSpace = m_compareColorSpace,
		_.sampleMethod = SampleMethod::Nearest);

	for (int i = 0; i < 3; i++)
	{
		viewer.pImages[i] = Base::defaultSurfaceFactory()->createSurface(bp);
		viewer.pImages[i]->fill(Color::Black);
	}

	viewer.pTitle = TextDisplay::create({ .display = {.style = wkit::TextStyles::Heading5, .text = "No test selected" }, .skin = ColorSkin::create({ .color = Color::Transparent, .padding = Border(8, 16, 0, 16) }) });

	auto pSlider = Slider::create(WGBP(Slider,
		_.handle = wkit::Skins::ScrollbarHandleX,
		_.skin = wkit::Skins::ScrollbarTrack,
		_.defaultSlideLength = 200,
		_.steps = c_maxDiffFactor - 1,
		_.value = 0.f));

	viewer.pDiffSlider = pSlider;

	viewer.pDiffFactorLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = "1x" } });

	Viewer* pViewer = &viewer;
	m_routes.push_back(Base::msgRouter()->addRoute(pSlider, MsgType::ValueUpdate, [this, pViewer](Msg* pMsg) {
		float value = static_cast<ValueUpdateMsg*>(pMsg)->value();
		int factor = 1 + int(std::lround(value * (c_maxDiffFactor - 1)));

		if (factor != pViewer->diffFactor)
		{
			pViewer->diffFactor = factor;
			refresh_diff(*pViewer);
		}
	}));
}

//____ create_image_box() _____________________________________________________
/*
	A label above one of the viewers images. The diff image also gets its slider.
	Images at full size have room for the backend name in their label.
*/

Widget_p GfxDeviceTester::create_image_box(Viewer& viewer, int image, float zoom)
{
	string label = image == TESTEE ? "Testee" : image == REFERENCE ? "Reference" : "Diff";
	if (image != 2 && zoom >= 1.f)
		label += ": " + m_backends[image].name;

	auto pLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = label.c_str() } });

	auto pDisplay = SurfaceDisplay::create(WGBP(SurfaceDisplay,
		_.skin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::Black,
			_.outlineThickness = 1,
			_.outlineColor = Color::DarkGrey,
			_.padding = 1)),
		_.surface = viewer.pImages[image],
		_.zoom = zoom,
		_.minUserZoom = zoom,
		_.maxUserZoom = zoom));

	auto pBox = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout, .spacing = 4 });
	pBox->slots.pushBack(pLabel, { .weight = 0 });
	pBox->slots.pushBack(pDisplay, { .weight = 0 });

	if (image == 2)
	{
		auto pSliderRow = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .spacing = 8 });
		pSliderRow->slots.pushBack(TextDisplay::create({ .display = {.style = wkit::TextStyles::Default, .text = "Amplify:" } }), { .weight = 0 });
		pSliderRow->slots.pushBack(viewer.pDiffSlider, { .weight = 1 });
		pSliderRow->slots.pushBack(viewer.pDiffFactorLabel, { .weight = 0 });

		pBox->slots.pushBack(pSliderRow, { .weight = 0 });
	}

	pBox->slots.pushBack(Filler::create(), { .weight = 1 });
	return pBox;
}

//____ create_progress_bar() __________________________________________________
/*
	A slider the user can't touch makes a progress bar.
*/

Widget_p GfxDeviceTester::create_progress_bar(Slider_p& pBar, TextDisplay_p& pLabel)
{
	pBar = Slider::create(WGBP(Slider,
		_.markPolicy = MarkPolicy::Ignore,
		_.defaultSlideLength = 240,
		_.skin = FillMeterSkin::create(WGBP(FillMeterSkin,
			_.backColor = Color::White,
			_.color = Color::SteelBlue,
			_.defaultSize = Size(240, 16),
			_.padding = 1))));

	pLabel = TextDisplay::create({ .display = {.layout = m_pRightAligned, .style = wkit::TextStyles::Default, .text = "" } });

	// Label first, so the bar stays in place when the length of the text changes.

	auto pPack = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .spacing = 8 });
	pPack->slots.pushBack(pLabel, { .weight = 1 });
	pPack->slots.pushBack(pBar, { .weight = 0 });
	return pPack;
}

//____ create_accuracy_view() _________________________________________________

Widget_p GfxDeviceTester::create_accuracy_view()
{
	create_viewer(m_accuracyViewer);

	// Table

	m_pAccuracyTable = TablePanel::create({ .columnSpacing = 0, .rowSpacing = 0, .skin = ColorSkin::create({ .color = Color::White, .padding = 4 }) });
	m_pAccuracyTable->setRowSkins(ColorSkin::create(Color::White), ColorSkin::create(Color::PapayaWhip));

	auto pTableScroller = wkit::ScrollCapsuleY::create();
	pTableScroller->slot = m_pAccuracyTable;

	// Preview of selected test: testee and reference at half size, diff below at full size.

	auto pPreview = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout, .spacing = 8 });

	auto pTopRow = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .spacing = 8 });
	pTopRow->slots.pushBack(create_image_box(m_accuracyViewer, TESTEE, 0.5f), { .weight = 0 });
	pTopRow->slots.pushBack(create_image_box(m_accuracyViewer, REFERENCE, 0.5f), { .weight = 0 });

	auto pShowButton = WGCREATE(wkit::Button, _.label.text = "Show in Display view");
	m_routes.push_back(Base::msgRouter()->addRoute(pShowButton, MsgType::Select, [this](Msg* pMsg) {
		if (m_accuracySelected >= 0)
			show_in_display(m_accuracySelected);
	}));

	auto pTitleRow = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout });
	pTitleRow->slots.pushBack(m_accuracyViewer.pTitle, { .weight = 1 });
	pTitleRow->slots.pushBack(pShowButton, { .weight = 0 });

	pPreview->slots.pushBack(pTitleRow, { .weight = 0 });
	pPreview->slots.pushBack(pTopRow, { .weight = 0 });
	pPreview->slots.pushBack(create_image_box(m_accuracyViewer, 2, 1.f), { .weight = 0 });

	auto pPreviewScroller = wkit::ScrollCapsuleY::create();
	pPreviewScroller->slot = PaddingCapsule::create({ .child = pPreview, .padding = 8 });

	auto pMid = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout });
	pMid->slots.pushBack(pTableScroller, { .weight = 1 });
	pMid->slots.pushBack(pPreviewScroller, { .weight = 0 });

	// Bottom bar

	auto pBottom = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .skin = wkit::Skins::Plate, .spacing = 8 });

	m_pAccuracySummary = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default } });

	m_pAccuracyRunButton = WGCREATE(wkit::Button, _.label.text = "RUN ALL");
	m_routes.push_back(Base::msgRouter()->addRoute(m_pAccuracyRunButton, MsgType::Select, [this](Msg* pMsg) {
		if (m_bAccuracyRunning)
		{
			m_bAccuracyRunning = false;
			m_pAccuracyRunButton->label.setText("RUN ALL");
			m_pAccuracyProgressLabel->display.setText("Stopped");
			refresh_accuracy_table();
		}
		else
			start_accuracy_run();
	}));

	pBottom->slots.pushBack(m_pAccuracySummary, { .weight = 1 });
	pBottom->slots.pushBack(create_progress_bar(m_pAccuracyProgress, m_pAccuracyProgressLabel), { .weight = 0 });
	pBottom->slots.pushBack(m_pAccuracyRunButton, { .weight = 0 });

	auto pBase = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout });
	pBase->slots.pushBack(pMid, { .weight = 1 });
	pBase->slots.pushBack(pBottom, { .weight = 0 });

	refresh_accuracy_table();

	return pBase;
}

//____ create_performance_view() ______________________________________________

Widget_p GfxDeviceTester::create_performance_view()
{
	create_viewer(m_performanceViewer);

	// Table

	const int nbColumns = 5;

	auto pTable = TablePanel::create({ .columnSpacing = 0, .rowSpacing = 0, .skin = ColorSkin::create({ .color = Color::White, .padding = 4 }) });
	pTable->setRowSkins(ColorSkin::create(Color::White), ColorSkin::create(Color::PapayaWhip));

	m_pPerformanceTable = pTable;

	pTable->resize(1 + (int)m_tests.size(), nbColumns);

	const char* headings[nbColumns] = { "SUITE", "TEST", "TIME PER RUN", "LOADED", "CHANGE" };
	for (int col = 0; col < nbColumns; col++)
		pTable->slots[0][col] = create_cell(-1, col < 2 ? nullptr : m_pRightAligned, wkit::TextStyles::Heading6, headings[col]);

	for (int i = 0; i < (int)m_tests.size(); i++)
	{
		auto& test = m_tests[i];

		test.pPerfTimeCell = create_cell(i, m_pRightAligned);
		test.pPerfLoadedCell = create_cell(i, m_pRightAligned);
		test.pPerfChangeCell = create_cell(i, m_pRightAligned);

		pTable->slots[1 + i][0] = create_cell(i, nullptr, nullptr, m_suites[test.suite].name);
		pTable->slots[1 + i][1] = create_cell(i, nullptr, nullptr, test.name);
		pTable->slots[1 + i][2] = test.pPerfTimeCell;
		pTable->slots[1 + i][3] = test.pPerfLoadedCell;
		pTable->slots[1 + i][4] = test.pPerfChangeCell;

		refresh_performance_row(i);
	}

	auto pTableScroller = wkit::ScrollCapsuleY::create();
	pTableScroller->slot = pTable;

	// Selected test, as rendered by the testee.

	auto pPreview = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout, .spacing = 8 });
	pPreview->slots.pushBack(m_performanceViewer.pTitle, { .weight = 0 });
	pPreview->slots.pushBack(create_image_box(m_performanceViewer, TESTEE, 1.f), { .weight = 0 });

	m_pPerformanceDetails = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default, .text = "Select a test to see it here." } });

	auto pRunSelectedButton = WGCREATE(wkit::Button, _.label.text = "RUN SELECTED TEST");
	m_routes.push_back(Base::msgRouter()->addRoute(pRunSelectedButton, MsgType::Select, [this](Msg* pMsg) {
		if (!m_bPerformanceRunning && m_performanceSelected >= 0)
			start_performance_run({ m_performanceSelected });
	}));

	auto pDetailsRow = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .spacing = 8 });
	pDetailsRow->slots.pushBack(m_pPerformanceDetails, { .weight = 1 });
	pDetailsRow->slots.pushBack(pRunSelectedButton, { .weight = 0 });

	pPreview->slots.pushBack(pDetailsRow, { .weight = 0 });

	auto pPreviewScroller = wkit::ScrollCapsuleY::create();
	pPreviewScroller->slot = PaddingCapsule::create({ .child = pPreview, .padding = 8 });

	auto pMid = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout });
	pMid->slots.pushBack(pTableScroller, { .weight = 1 });
	pMid->slots.pushBack(pPreviewScroller, { .weight = 0 });

	// Bar for saving and loading results.

	auto pFileBar = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .skin = wkit::Skins::Plate, .spacing = 8 });

	auto pSaveButton = WGCREATE(wkit::Button, _.label.text = "SAVE RESULTS...");
	m_routes.push_back(Base::msgRouter()->addRoute(pSaveButton, MsgType::Select, [this](Msg* pMsg) { save_performance_results(); }));

	auto pLoadButton = WGCREATE(wkit::Button, _.label.text = "LOAD RESULTS...");
	m_routes.push_back(Base::msgRouter()->addRoute(pLoadButton, MsgType::Select, [this](Msg* pMsg) { load_performance_results(); }));

	m_pPerformanceLoadedLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default, .text = "No results loaded to compare with." } });

	pFileBar->slots.pushBack(pSaveButton, { .weight = 0 });
	pFileBar->slots.pushBack(pLoadButton, { .weight = 0 });
	pFileBar->slots.pushBack(m_pPerformanceLoadedLabel, { .weight = 1 });

	// Number of passes through the tests. Each test keeps its best result.

	auto pPassesLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Strong, .text = "Passes: " } });
	pFileBar->slots.pushBack(pPassesLabel, { .weight = 0 });

	auto pPassesGroup = ToggleGroup::create();

	for (int passes : { 1, 3, 5, 10, 20 })
	{
		auto pButton = WGCREATE(wkit::ToggleButton, _.label.text = std::to_string(passes).c_str());
		pPassesGroup->add(pButton);
		if (passes == m_performancePasses)
			pButton->setChecked(true);

		pFileBar->slots.pushBack(pButton, { .weight = 0 });

		m_routes.push_back(Base::msgRouter()->addRoute(pButton, MsgType::Toggle, [this, passes](Msg* pMsg) {
			if (static_cast<ToggleMsg*>(pMsg)->isChecked())
				m_performancePasses = passes;
		}));
	}

	// Bottom bar

	auto pBottom = PackPanel::create({ .axis = Axis::X, .layout = m_pLayout, .skin = wkit::Skins::Plate, .spacing = 8 });

	m_pPerformanceSummary = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default,
		.text = "Times each test on the testee only. Init and exit are not included." } });

	m_pPerformanceRunButton = WGCREATE(wkit::Button, _.label.text = "RUN ALL TESTS");
	m_routes.push_back(Base::msgRouter()->addRoute(m_pPerformanceRunButton, MsgType::Select, [this](Msg* pMsg) {
		if (m_bPerformanceRunning)
		{
			m_bPerformanceRunning = false;
			m_pPerformanceRunButton->label.setText("RUN ALL TESTS");
			m_pPerformanceProgressLabel->display.setText("Stopped");
		}
		else
		{
			vector<int> all(m_tests.size());
			for (int i = 0; i < (int)all.size(); i++)
				all[i] = i;

			start_performance_run(all);
		}
	}));

	pBottom->slots.pushBack(m_pPerformanceSummary, { .weight = 1 });
	pBottom->slots.pushBack(create_progress_bar(m_pPerformanceProgress, m_pPerformanceProgressLabel), { .weight = 0 });
	pBottom->slots.pushBack(m_pPerformanceRunButton, { .weight = 0 });

	auto pBase = PackPanel::create({ .axis = Axis::Y, .layout = m_pLayout });
	pBase->slots.pushBack(pFileBar, { .weight = 0 });
	pBase->slots.pushBack(pMid, { .weight = 1 });
	pBase->slots.pushBack(pBottom, { .weight = 0 });

	return pBase;
}

//____ create_cell() __________________________________________________________
/*
	A cell of the accuracy or performance table. The id is the index of the test
	of its row, or -1 for headings. Clicking a cell selects its row.
*/

TextDisplay_p GfxDeviceTester::create_cell(int test, TextLayout* pLayout, TextStyle* pStyle, const string& text)
{
	return TextDisplay::create({
		.display = {.layout = pLayout, .style = pStyle ? pStyle : wkit::TextStyles::Default.rawPtr(), .text = text.c_str() },
		.id = test,
		.markPolicy = MarkPolicy::Geometry,
		.skin = m_pCellSkin });
}

//____ highlight_row() ________________________________________________________

void GfxDeviceTester::highlight_row(TablePanel* pTable, int row, bool bHighlight)
{
	if (row < 0 || row >= pTable->rows.size())
		return;

	for (int col = 0; col < pTable->columns.size(); col++)
	{
		Widget* pCell = pTable->slots[row][col]._widget();
		if (pCell)
			pCell->setSkin(bHighlight ? m_pSelectedCellSkin : m_pCellSkin);
	}
}

//____ select_view() __________________________________________________________

void GfxDeviceTester::select_view(View view)
{
	m_pViewBase->slot = m_pViews[int(view)];

	if (!m_pViewButtons[int(view)]->isChecked())
		m_pViewButtons[int(view)]->setChecked(true);
}

//____ show_in_display() ______________________________________________________
/*
	Switches to the display view, showing the given test selected in the tree.
	Selecting from code doesn't post a message for the selected entry, so the
	test is shown from here.
*/

void GfxDeviceTester::show_in_display(int test)
{
	auto& suite = m_suites[m_tests[test].suite];

	suite.pTreeDrawer->openImmediately();
	m_pTestTreeSelector->select(m_tests[test].pTreeEntry);

	show_test(m_displayViewer, test);
	select_view(View::Display);
}

//____ render_test() __________________________________________________________
/*
	Renders a test, including its init and exit, on one of the backends.
*/

bool GfxDeviceTester::render_test(Side side, int test, Background background, ClipList clipList)
{
	auto& entry = m_tests[test];
	if (!m_suites[entry.suite].bWorking)
		return false;

	auto& backend = m_backends[side];
	GfxDevice* pDevice = backend.pDevice;
	Test* pTest = entry.pTests[side];

	RectSPX canvas = { 0, 0, m_canvasSize.w * 64, m_canvasSize.h * 64 };

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(backend.pCanvas);

	fill_background(pDevice, background);

	vector<RectSPX> clipRects;
	if (clipList != ClipList::One)
	{
		clipRects = clip_rects(clipList);
		pDevice->setClipList((int)clipRects.size(), clipRects.data());
	}

	if (pTest->init)
		pTest->init(pDevice, canvas);

	pTest->run(pDevice, canvas);

	if (pTest->exit)
		pTest->exit(pDevice, canvas);

	if (clipList != ClipList::One)
		pDevice->resetClipList();

	pDevice->endCanvasUpdate();
	pDevice->endRender();
	return true;
}

//____ clip_rects() ___________________________________________________________
/*
	Rectangles laid out like the black squares of a checkerboard. For the few
	and many lists, each side of a rectangle is 1/8 or 1/32 of the canvas.
*/

vector<RectSPX> GfxDeviceTester::clip_rects(ClipList clipList)
{
	SizeSPX canvasSize = m_canvasSize * 64;
	SizeSPX rectSize = clipList == ClipList::Few ? canvasSize / 8 : clipList == ClipList::Many ? canvasSize / 32 : canvasSize;

	vector<RectSPX> rects;

	for (spx y = 0; y + rectSize.h <= canvasSize.h; y += rectSize.h)
	{
		spx x = (y / rectSize.h) % 2 == 0 ? 0 : rectSize.w;

		for (; x + rectSize.w <= canvasSize.w; x += rectSize.w * 2)
			rects.push_back({ x, y, rectSize.w, rectSize.h });
	}

	return rects;
}

//____ fill_background() ______________________________________________________

void GfxDeviceTester::fill_background(GfxDevice* pDevice, Background background)
{
	RectSPX canvas = { 0, 0, m_canvasSize.w * 64, m_canvasSize.h * 64 };

	pDevice->setBlendMode(BlendMode::Replace);

	switch (background)
	{
		case Background::Black:
			pDevice->fill(canvas, Color::Black);
			break;
		case Background::White:
			pDevice->fill(canvas, Color::White);
			break;
		case Background::Grey:
			pDevice->fill(canvas, Color8(128, 128, 128));
			break;
		case Background::Checkered:
		{
			const int squareSize = 16;

			pDevice->fill(canvas, Color8(160, 160, 160));

			for (int y = 0; y < m_canvasSize.h; y += squareSize)
			{
				for (int x = ((y / squareSize) % 2) * squareSize; x < m_canvasSize.w; x += squareSize * 2)
					pDevice->fill(RectSPX(x, y, squareSize, squareSize) * 64, Color8(96, 96, 96));
			}
			break;
		}
	}

	pDevice->setBlendMode(BlendMode::Blend);
}

//____ read_canvas() __________________________________________________________
/*
	Reads back the canvas of a backend as ARGB_8 in the color space we compare in.
*/

bool GfxDeviceTester::read_canvas(Side side, vector<uint8_t>& pixels)
{
	Surface* pSurface = m_backends[side].pCanvas;
	if (!pSurface || pSurface->pixelSize() != m_canvasSize)
		return false;

	pixels.resize(m_canvasSize.w * m_canvasSize.h * 4);

	auto buffer = pSurface->allocPixelBuffer();
	pSurface->pushPixels(buffer);

	int srcPitchAdd = buffer.pitch - m_canvasSize.w * Util::pixelFormatToDescription(buffer.format).bits / 8;
	int dstPaletteEntries = 0;

	bool bOk = PixelTools::copyPixels(m_canvasSize.w, m_canvasSize.h, buffer.pixels, buffer.format, buffer.colorSpace, buffer.bigEndian, srcPitchAdd,
		buffer.palette, pSurface->paletteSize(),
		pixels.data(), PixelFormat::ARGB_8, m_compareColorSpace, (WG_IS_BIG_ENDIAN == 1), 0, nullptr, dstPaletteEntries, 0);

	pSurface->freePixelBuffer(buffer);
	return bOk;
}

//____ write_image() __________________________________________________________
/*
	Writes ARGB_8 pixels, in the color space we compare in, to one of the images
	we display.
*/

void GfxDeviceTester::write_image(Surface* pSurface, const vector<uint8_t>& pixels)
{
	auto buffer = pSurface->allocPixelBuffer();

	int dstPitchAdd = buffer.pitch - m_canvasSize.w * Util::pixelFormatToDescription(buffer.format).bits / 8;
	int dstPaletteEntries = 0;

	PixelTools::copyPixels(m_canvasSize.w, m_canvasSize.h, pixels.data(), PixelFormat::ARGB_8, m_compareColorSpace, (WG_IS_BIG_ENDIAN == 1), 0,
		nullptr, 0,
		buffer.pixels, buffer.format, buffer.colorSpace, buffer.bigEndian, dstPitchAdd, nullptr, dstPaletteEntries, 0);

	pSurface->pullPixels(buffer);
	pSurface->freePixelBuffer(buffer);
}

//____ compare() ______________________________________________________________

void GfxDeviceTester::compare(const vector<uint8_t>& testee, const vector<uint8_t>& reference, AccuracyResult& result)
{
	const uint8_t* pTestee = testee.data();
	const uint8_t* pRef = reference.data();
	size_t nbValues = reference.size();
	int64_t sum = 0;

	result = AccuracyResult();

	for (size_t i = 0; i < nbValues; i++)
	{
		int diff = std::abs(int(pRef[i]) - int(pTestee[i]));
		sum += diff;

		if (diff > c_diffThreshold)
			result.nbOverThreshold++;

		if (diff > result.maxDiff)
		{
			result.maxDiff = diff;
			int pixel = int(i / 4);
			result.worst = { pixel % m_canvasSize.w, pixel / m_canvasSize.w };
		}
	}

	result.meanDiff = double(sum) / nbValues;
	result.bDone = true;
}

//____ show_test() ____________________________________________________________

void GfxDeviceTester::show_test(Viewer& viewer, int test)
{
	viewer.test = test;
	refresh_viewer(viewer);
}

//____ refresh_viewer() _______________________________________________________
/*
	Renders the viewers test on both backends and updates its images.
*/

void GfxDeviceTester::refresh_viewer(Viewer& viewer)
{
	if (viewer.test < 0)
		return;

	auto& test = m_tests[viewer.test];

	string title = m_suites[test.suite].name + " / " + test.name;

	bool bOk = true;
	for (int side = 0; side < 2; side++)
	{
		if (render_test(Side(side), viewer.test, viewer.background, viewer.clipList) && read_canvas(Side(side), viewer.pixels[side]))
			write_image(viewer.pImages[side], viewer.pixels[side]);
		else
		{
			viewer.pixels[side].clear();
			viewer.pImages[side]->fill(Color::Black);
			bOk = false;
		}
	}

	if (!bOk)
		title += "   (could not be rendered)";

	viewer.pTitle->display.setText(title);

	refresh_diff(viewer);
}

//____ refresh_diff() _________________________________________________________
/*
	The diff image shows the difference of each color channel, multiplied by the
	diff factor. It is opaque, alpha differences are not shown.
*/

void GfxDeviceTester::refresh_diff(Viewer& viewer)
{
	char label[32];
	snprintf(label, sizeof(label), "%dx", viewer.diffFactor);
	viewer.pDiffFactorLabel->display.setText(label);

	auto& testee = viewer.pixels[TESTEE];
	auto& reference = viewer.pixels[REFERENCE];

	if (testee.empty() || testee.size() != reference.size())
	{
		viewer.pImages[2]->fill(Color::Black);
		return;
	}

	const int alphaOfs = WG_IS_BIG_ENDIAN ? 0 : 3;

	vector<uint8_t> diff(testee.size());

	for (size_t i = 0; i < diff.size(); i += 4)
	{
		for (int c = 0; c < 4; c++)
		{
			if (c == alphaOfs)
				diff[i + c] = 255;
			else
				diff[i + c] = (uint8_t) std::min(255, std::abs(int(testee[i + c]) - int(reference[i + c])) * viewer.diffFactor);
		}
	}

	write_image(viewer.pImages[2], diff);
}

//____ start_accuracy_run() ___________________________________________________

void GfxDeviceTester::start_accuracy_run()
{
	for (auto& test : m_tests)
		test.accuracy = AccuracyResult();

	m_bAccuracyRunning = true;
	m_accuracyNext = 0;

	m_pAccuracyRunButton->label.setText("STOP");
	m_pAccuracyProgress->setValue(0.f);

	refresh_accuracy_table();
}

//____ continue_accuracy_run() ________________________________________________
/*
	Runs tests until our time budget for this update is spent, so the progress
	bar keeps moving and the window stays responsive.
*/

void GfxDeviceTester::continue_accuracy_run()
{
	auto start = Clock::now();
	int nbTests = (int)m_tests.size();

	while (m_accuracyNext < nbTests && std::chrono::duration<double>(Clock::now() - start).count() < c_progressBudget)
		run_accuracy_test(m_accuracyNext++);

	char label[256];

	if (m_accuracyNext < nbTests)
	{
		auto& next = m_tests[m_accuracyNext];
		snprintf(label, sizeof(label), "%d / %d   %s / %s", m_accuracyNext, nbTests, m_suites[next.suite].name.c_str(), next.name.c_str());
	}
	else
	{
		snprintf(label, sizeof(label), "Done, %d tests", nbTests);

		m_bAccuracyRunning = false;
		m_pAccuracyRunButton->label.setText("RUN ALL");
		refresh_accuracy_table();
	}

	m_pAccuracyProgress->setValue(nbTests > 0 ? m_accuracyNext / float(nbTests) : 1.f);
	m_pAccuracyProgressLabel->display.setText(label);
}

//____ run_accuracy_test() ____________________________________________________
/*
	Renders a test on both backends, on black, reads back the results and compares them.
*/

void GfxDeviceTester::run_accuracy_test(int test)
{
	auto& result = m_tests[test].accuracy;

	vector<uint8_t> pixels[2];

	for (int side = 0; side < 2; side++)
	{
		if (!render_test(Side(side), test, Background::Black) || !read_canvas(Side(side), pixels[side]))
		{
			result = AccuracyResult();
			result.bDone = true;
			result.bFailed = true;
			return;
		}
	}

	compare(pixels[TESTEE], pixels[REFERENCE], result);
}

//____ run_batch_accuracy() ___________________________________________________
/*
	Runs the accuracy tests of the suites asked for on the command line and prints a line per
	test, with the verdict as in the accuracy table.
*/

void GfxDeviceTester::run_batch_accuracy()
{
	int nbRun = 0, nbDiffering = 0, nbFailed = 0;

	for (int i = 0; i < (int)m_tests.size(); i++)
	{
		auto& test = m_tests[i];
		auto& suite = m_suites[test.suite];

		if (!m_batchSuites.empty() && std::find(m_batchSuites.begin(), m_batchSuites.end(), suite.name) == m_batchSuites.end())
			continue;

		const char* pVerdict;
		auto& result = test.accuracy;

		if (!suite.bWorking)
		{
			pVerdict = "NOT WORKING";
			nbFailed++;
		}
		else
		{
			run_accuracy_test(i);
			nbRun++;

			if (result.bFailed)
			{
				pVerdict = "FAILED";
				nbFailed++;
			}
			else if (result.nbOverThreshold == 0)
				pVerdict = "OK";
			else if (result.nbOverThreshold <= c_maxSpecks)
				pVerdict = "SPECKS";
			else
			{
				pVerdict = "DIFFERS";
				nbDiffering++;
			}
		}

		printf("%-24s %-36s %-11s max %3d  mean %.3f  over %6d  worst %d,%d\n", suite.name.c_str(), test.name.c_str(), pVerdict,
			   result.maxDiff, result.meanDiff, result.nbOverThreshold, result.worst.x, result.worst.y);

		if (m_bBatchDump && suite.bWorking && result.nbOverThreshold > 0)
		{
			for (int side = 0; side < 2; side++)
			{
				vector<uint8_t> pixels;
				if (!render_test(Side(side), i, Background::Black) || !read_canvas(Side(side), pixels))
					continue;

				string filename = suite.name + "_" + test.name + (side == TESTEE ? "_testee.ppm" : "_reference.ppm");
				std::replace(filename.begin(), filename.end(), ' ', '_');

				FILE* pFile = fopen(filename.c_str(), "wb");
				if (!pFile)
					continue;

				const int r = WG_IS_BIG_ENDIAN ? 1 : 2, g = WG_IS_BIG_ENDIAN ? 2 : 1, b = WG_IS_BIG_ENDIAN ? 3 : 0;

				fprintf(pFile, "P6 %d %d 255\n", m_canvasSize.w, m_canvasSize.h);
				for (size_t p = 0; p + 3 < pixels.size(); p += 4)
				{
					fputc(pixels[p + r], pFile);
					fputc(pixels[p + g], pFile);
					fputc(pixels[p + b], pFile);
				}
				fclose(pFile);
			}
		}
	}

	printf("%d tests run, %d differ, %d failed or not working.\n", nbRun, nbDiffering, nbFailed);
	fflush(stdout);
}

//____ refresh_accuracy_table() _______________________________________________
/*
	Lists the results with the worst first, so what needs looking at is at the top.
	Tests not yet compared are last, in suite order.
*/

void GfxDeviceTester::refresh_accuracy_table()
{
	const int nbColumns = 7;

	m_accuracyRowOrder.clear();
	for (int i = 0; i < (int)m_tests.size(); i++)
		m_accuracyRowOrder.push_back(i);

	std::stable_sort(m_accuracyRowOrder.begin(), m_accuracyRowOrder.end(), [this](int a, int b) {
		const AccuracyResult& ra = m_tests[a].accuracy;
		const AccuracyResult& rb = m_tests[b].accuracy;

		if (ra.bDone != rb.bDone)
			return ra.bDone;
		if (!ra.bDone)
			return false;
		if (ra.bFailed != rb.bFailed)
			return ra.bFailed;
		if (ra.nbOverThreshold != rb.nbOverThreshold)
			return ra.nbOverThreshold > rb.nbOverThreshold;
		return ra.maxDiff > rb.maxDiff;
	});

	auto pTable = m_pAccuracyTable;
	pTable->resize(1 + (int)m_tests.size(), nbColumns);

	const char* headings[nbColumns] = { "SUITE", "TEST", "MAX", "MEAN", "OVER", "WORST AT", "RESULT" };
	for (int col = 0; col < nbColumns; col++)
		pTable->slots[0][col] = create_cell(-1, col < 2 ? nullptr : m_pRightAligned, wkit::TextStyles::Heading6, headings[col]);

	int nbDone = 0, nbOk = 0, nbSpecks = 0, nbDiff = 0, nbFailed = 0;

	for (int row = 0; row < (int)m_accuracyRowOrder.size(); row++)
	{
		int testIndex = m_accuracyRowOrder[row];
		const TestEntry& test = m_tests[testIndex];
		const AccuracyResult& r = test.accuracy;

		string values[nbColumns];
		TextStyle_p pVerdictStyle = wkit::TextStyles::Default;

		values[0] = m_suites[test.suite].name;
		values[1] = test.name;

		if (r.bDone)
		{
			nbDone++;

			if (r.bFailed)
			{
				values[6] = "FAILED";
				pVerdictStyle = m_pDiffStyle;
				nbFailed++;
			}
			else
			{
				char buffer[64];

				snprintf(buffer, sizeof(buffer), "%d", r.maxDiff);
				values[2] = buffer;
				snprintf(buffer, sizeof(buffer), "%.2f", r.meanDiff);
				values[3] = buffer;
				snprintf(buffer, sizeof(buffer), "%d", r.nbOverThreshold);
				values[4] = buffer;
				if (r.maxDiff > 0)
				{
					snprintf(buffer, sizeof(buffer), "%d, %d", r.worst.x, r.worst.y);
					values[5] = buffer;
				}

				if (r.nbOverThreshold == 0)
				{
					values[6] = "OK";
					pVerdictStyle = m_pOkStyle;
					nbOk++;
				}
				else if (r.nbOverThreshold <= c_maxSpecks)
				{
					values[6] = "SPECKS";
					pVerdictStyle = m_pSpecksStyle;
					nbSpecks++;
				}
				else
				{
					values[6] = "DIFF";
					pVerdictStyle = m_pDiffStyle;
					nbDiff++;
				}
			}
		}
		else
			values[6] = "-";

		for (int col = 0; col < nbColumns; col++)
			pTable->slots[1 + row][col] = create_cell(testIndex, col < 2 ? nullptr : m_pRightAligned, col == 6 ? pVerdictStyle : wkit::TextStyles::Default, values[col]);
	}

	// Restore selection, the row might have moved.

	int selected = m_accuracySelected;
	m_accuracySelected = -1;
	if (selected >= 0)
		select_accuracy_row(selected);

	char summary[256];
	if (nbDone == 0)
		snprintf(summary, sizeof(summary), "Press RUN ALL to render every test on both backends, on black, and compare.");
	else
		snprintf(summary, sizeof(summary), "%d tests compared: %d OK, %d with specks, %d differ, %d failed. Click a row to view the test.", nbDone, nbOk, nbSpecks, nbDiff, nbFailed);

	m_pAccuracySummary->display.setText(summary);
}

//____ select_accuracy_row() __________________________________________________

void GfxDeviceTester::select_accuracy_row(int test)
{
	auto rowOf = [this](int test) {
		auto it = std::find(m_accuracyRowOrder.begin(), m_accuracyRowOrder.end(), test);
		return it == m_accuracyRowOrder.end() ? -1 : 1 + int(it - m_accuracyRowOrder.begin());
	};

	if (m_accuracySelected >= 0)
		highlight_row(m_pAccuracyTable, rowOf(m_accuracySelected), false);

	m_accuracySelected = test;
	highlight_row(m_pAccuracyTable, rowOf(test), true);

	if (m_accuracyViewer.test != test)
		show_test(m_accuracyViewer, test);
}

//____ start_performance_run() ________________________________________________
/*
	Times the given tests, one per update, in as many passes as set. Results of
	other tests are kept.
*/

void GfxDeviceTester::start_performance_run(const vector<int>& tests)
{
	for (int test : tests)
	{
		m_tests[test].performance = PerformanceResult();
		refresh_performance_row(test);
	}

	refresh_performance_details();

	m_bPerformanceRunning = true;
	m_performanceQueue = tests;
	m_performanceRunPasses = m_performancePasses;
	m_performancePass = 0;
	m_performanceNext = 0;

	m_pPerformanceRunButton->label.setText("STOP");
	m_pPerformanceProgress->setValue(0.f);
	m_pPerformanceSummary->display.setText("Times each test on the testee only. Init and exit are not included.");
}

//____ continue_performance_run() _____________________________________________
/*
	Times tests until our time budget for this update is spent, then lets the
	progress bar and window update.
*/

void GfxDeviceTester::continue_performance_run()
{
	int nbTests = (int)m_performanceQueue.size();

	auto start = Clock::now();

	while (m_performancePass < m_performanceRunPasses && std::chrono::duration<double>(Clock::now() - start).count() < c_perfUpdateBudget)
	{
		int test = m_performanceQueue[m_performanceNext];

		if (m_performancePass == 0 && m_performanceNext == 0)
			warm_up_testee(test);

		m_performanceNext++;
		run_performance_pass(test);
		refresh_performance_row(test);

		if (test == m_performanceSelected)
			refresh_performance_details();

		if (m_performanceNext == nbTests)
		{
			m_performancePass++;
			m_performanceNext = 0;
		}
	}

	char label[256];

	if (m_performancePass < m_performanceRunPasses)
	{
		auto& next = m_tests[m_performanceQueue[m_performanceNext]];
		snprintf(label, sizeof(label), "Pass %d / %d   %d / %d   %s / %s", m_performancePass + 1, m_performanceRunPasses,
			m_performanceNext, nbTests, m_suites[next.suite].name.c_str(), next.name.c_str());
	}
	else
	{
		snprintf(label, sizeof(label), nbTests == 1 ? "Done, %d test" : "Done, %d tests", nbTests);

		m_bPerformanceRunning = false;
		m_pPerformanceRunButton->label.setText("RUN ALL TESTS");

		double total = 0;
		int nbTimed = 0;
		int nbFailed = 0;
		for (auto& test : m_tests)
		{
			if (!test.performance.bDone)
				continue;

			if (test.performance.bFailed)
				nbFailed++;
			else
			{
				total += test.performance.secondsPerRun;
				nbTimed++;
			}
		}

		string summary = std::to_string(nbTimed) + (nbTimed == 1 ? " test timed" : " tests timed") + ", one run of each adds up to " + format_time(total) + ".";
		if (nbFailed > 0)
			summary += " " + std::to_string(nbFailed) + (nbFailed == 1 ? " test" : " tests") + " could not be run.";
		summary += " Init and exit are not included.";

		m_pPerformanceSummary->display.setText(summary.c_str());
	}

	int nbSteps = nbTests * m_performanceRunPasses;
	int nbStepsDone = std::min(nbSteps, m_performancePass * nbTests + m_performanceNext);

	m_pPerformanceProgress->setValue(nbSteps > 0 ? nbStepsDone / float(nbSteps) : 1.f);
	m_pPerformanceProgressLabel->display.setText(label);
}

//____ run_performance_pass() _________________________________________________
/*
	Times a test on the testee, once per pass.

	Each measurement times a number of calls to run() including the endRender()
	and a read back of a pixel, which makes sure the backend has finished its work,
	also when it is done asynchronously, as by a GPU. The same without any calls
	to run() is measured as a baseline and subtracted. That way neither init(),
	exit() nor the overhead of a render pass are included.

	The first pass calibrates the number of calls to run() so that a measurement
	takes long enough to be accurate. Each pass then warms up by running the test
	for a while, so caches and clock speeds have settled, and keeps the fastest
	of a number of measurements, as it is the one least disturbed by anything else
	going on.

	Timings are noisy in bursts, from other processes, power management and such,
	which can make all measurements of one pass slow. Making several passes through
	all tests spreads the measurements of each test over time, so the fastest of
	them is much more likely to be undisturbed.
*/

void GfxDeviceTester::run_performance_pass(int test)
{
	auto& r = m_tests[test].performance;

	if (!m_suites[m_tests[test].suite].bWorking)
	{
		r.bDone = true;
		r.bFailed = true;
		return;
	}

	Test* pTest = m_tests[test].pTests[TESTEE];

	if (r.loops == 0)
	{
		r.baseline = std::numeric_limits<double>::max();
		r.loops = calibrate_test(pTest, r.baseline);
	}

	// Warm up.

	double fastest = time_test(pTest, r.loops);

	auto start = Clock::now();
	while (std::chrono::duration<double>(Clock::now() - start).count() < c_perfWarmupTime)
		fastest = std::min(fastest, time_test(pTest, r.loops));

	// Measure. Tests that are slow even without looping get fewer rounds,
	// noise is small in comparison and we don't want to wait forever.

	int rounds = fastest > c_perfTargetTime * 10 ? c_perfSlowRounds : c_perfRounds;

	for (int round = 0; round < rounds; round++)
	{
		r.baseline = std::min(r.baseline, time_test(pTest, 0));
		fastest = std::min(fastest, time_test(pTest, r.loops));
	}

	r.passes.push_back(fastest);

	r.secondsPerRun = std::max(0.0, *std::min_element(r.passes.begin(), r.passes.end()) - r.baseline) / r.loops;
	r.bDone = true;
}

//____ warm_up_testee() _______________________________________________________
/*
	Keeps the testee busy with a test for c_perfRunWarmupTime, so a GPU has its
	clocks up before we calibrate and time anything.
*/

void GfxDeviceTester::warm_up_testee(int test)
{
	if (!m_suites[m_tests[test].suite].bWorking)
		return;

	Test* pTest = m_tests[test].pTests[TESTEE];

	double baseline = std::numeric_limits<double>::max();
	int loops = calibrate_test(pTest, baseline);

	auto start = Clock::now();
	while (std::chrono::duration<double>(Clock::now() - start).count() < c_perfRunWarmupTime)
		time_test(pTest, loops);
}

//____ calibrate_test() _______________________________________________________
/*
	Returns the number of calls to run() needed for a measurement to take at least
	c_perfTargetTime. Lowers baseline to the fastest measurement without calls.
*/

int GfxDeviceTester::calibrate_test(Test* pTest, double& baseline)
{
	// A first run includes any first time setup (caches, shaders, buffers), making
	// it slower than later ones. That gives us a low first guess.

	double first = time_test(pTest, 1);
	baseline = std::min({ baseline, time_test(pTest, 0), time_test(pTest, 0) });

	int loops = 1;
	if (first - baseline > 0)
		loops = (int) std::clamp(std::floor(c_perfTargetTime / (first - baseline)), 1.0, double(c_perfMaxLoops));

	while (true)
	{
		double time = time_test(pTest, loops) - baseline;

		if (time >= c_perfTargetTime || loops >= c_perfMaxLoops)
			return loops;

		// Aim a bit over the target, but grow at least 2x and at most 100x each step.

		double factor = time > 0 ? c_perfTargetTime * 1.25 / time : 100.0;
		factor = std::clamp(factor, 2.0, 100.0);

		loops = (int) std::min(double(c_perfMaxLoops), std::ceil(loops * factor));
		baseline = std::min(baseline, time_test(pTest, 0));
	}
}

//____ PerformanceResult::spread() ____________________________________________

double GfxDeviceTester::PerformanceResult::spread() const
{
	if (passes.size() < 2 || loops == 0)
		return 0;

	auto [pFastest, pSlowest] = std::minmax_element(passes.begin(), passes.end());

	double fastest = *pFastest - baseline;
	return fastest > 0 ? (*pSlowest - baseline) / fastest - 1.0 : 0;
}

//____ time_test() ____________________________________________________________
/*
	Seconds from just before the first call to run() until the testee has finished
	rendering. See run_performance_pass().
*/

double GfxDeviceTester::time_test(Test* pTest, int loops)
{
	auto& backend = m_backends[TESTEE];
	GfxDevice* pDevice = backend.pDevice;

	RectSPX canvas = { 0, 0, m_canvasSize.w * 64, m_canvasSize.h * 64 };

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(backend.pCanvas);

	fill_background(pDevice, Background::Black);

	if (pTest->init)
		pTest->init(pDevice, canvas);

	auto start = Clock::now();

	for (int i = 0; i < loops; i++)
		pTest->run(pDevice, canvas);

	if (pTest->exit)
		pTest->exit(pDevice, canvas);

	pDevice->endCanvasUpdate();
	pDevice->endRender();

	sync_testee();

	auto end = Clock::now();

	return std::chrono::duration<double>(end - start).count();
}

//____ sync_testee() __________________________________________________________
/*
	Reads back a pixel from the testee canvas, which forces it to finish rendering.
*/

void GfxDeviceTester::sync_testee()
{
	Surface* pCanvas = m_backends[TESTEE].pCanvas;

	auto buffer = pCanvas->allocPixelBuffer(RectI(0, 0, 1, 1));
	pCanvas->pushPixels(buffer);
	pCanvas->freePixelBuffer(buffer);
}

//____ refresh_performance_row() ______________________________________________

void GfxDeviceTester::refresh_performance_row(int test)
{
	auto& entry = m_tests[test];
	auto& r = entry.performance;

	TextStyle_p pTimeStyle = wkit::TextStyles::Default;
	TextStyle_p pChangeStyle = wkit::TextStyles::Default;
	string time, loaded, change;

	if (!r.bDone)
		time = "-";
	else if (r.bFailed)
	{
		time = "FAILED";
		pTimeStyle = m_pDiffStyle;
	}
	else
		time = format_time(r.secondsPerRun);

	if (entry.loadedSecondsPerRun >= 0)
		loaded = format_time(entry.loadedSecondsPerRun);

	double ratio;
	if (performance_change(test, ratio))
	{
		change = format_change(ratio);

		if (ratio > c_perfChangeThreshold)
			pChangeStyle = m_pDiffStyle;
		else if (ratio < -c_perfChangeThreshold)
			pChangeStyle = m_pOkStyle;
	}

	entry.pPerfTimeCell->display.setText(time.c_str());
	entry.pPerfTimeCell->display.setStyle(pTimeStyle);
	entry.pPerfLoadedCell->display.setText(loaded.c_str());
	entry.pPerfChangeCell->display.setText(change.c_str());
	entry.pPerfChangeCell->display.setStyle(pChangeStyle);
}

//____ select_performance_row() _______________________________________________

void GfxDeviceTester::select_performance_row(int test)
{
	if (m_performanceSelected >= 0)
		highlight_row(m_pPerformanceTable, 1 + m_performanceSelected, false);

	m_performanceSelected = test;
	highlight_row(m_pPerformanceTable, 1 + test, true);

	if (m_performanceViewer.test != test)
		show_test(m_performanceViewer, test);

	refresh_performance_details();
}

//____ refresh_performance_details() __________________________________________

void GfxDeviceTester::refresh_performance_details()
{
	if (m_performanceSelected < 0)
		return;

	auto& test = m_tests[m_performanceSelected];
	auto& r = test.performance;

	string details;

	if (!r.bDone)
		details = "Not timed yet.";
	else if (r.bFailed)
		details = "Could not be run.";
	else
	{
		char spread[64] = "";
		if (r.passes.size() > 1)
			snprintf(spread, sizeof(spread), ", passes within %.1f %%", r.spread() * 100.0);

		details = "Time per run: " + format_time(r.secondsPerRun) + " (best of " + std::to_string(r.passes.size())
			+ (r.passes.size() == 1 ? " pass" : " passes") + spread + ")\nLoops per measurement: " + std::to_string(r.loops);
	}

	if (test.loadedSecondsPerRun >= 0)
	{
		details += "\nLoaded: " + format_time(test.loadedSecondsPerRun);

		double ratio;
		if (performance_change(m_performanceSelected, ratio))
			details += "   Change: " + format_change(ratio);
	}

	m_pPerformanceDetails->display.setText(details.c_str());
}

//____ save_performance_results() _____________________________________________
/*
	Saves the results as text, one test per line:

		<suite> TAB <test> TAB <seconds per run> TAB <loops>

	Lines starting with '#' are comments. Tests are identified by suite and
	name, so results can be compared after tests have been added or removed.
*/

void GfxDeviceTester::save_performance_results()
{
	int nbResults = 0;
	for (auto& test : m_tests)
	{
		if (test.performance.bDone && !test.performance.bFailed)
			nbResults++;
	}

	if (nbResults == 0)
	{
		m_pPerformanceLoadedLabel->display.setText("No results to save, run the performance test first.");
		return;
	}

	string path = m_pAPI->saveFileDialog("Save Performance Results", "", "performance.txt", { "*.txt" }, "Performance results");
	if (path.empty())
		return;

	// Add an extension if none was given.

	size_t nameStart = path.find_last_of("/\\");
	if (path.find('.', nameStart == string::npos ? 0 : nameStart) == string::npos)
		path += ".txt";

	std::ofstream file(path);
	if (!file)
	{
		m_pPerformanceLoadedLabel->display.setText(("Could not save to " + path).c_str());
		return;
	}

	char date[64] = "";
	std::time_t now = std::time(nullptr);
	std::strftime(date, sizeof(date), "%Y-%m-%d %H:%M", std::localtime(&now));

	file << "# GfxDeviceTester performance results\n";
	file << "# Testee: " << m_backends[TESTEE].name << "\n";
	file << "# Saved: " << date << "\n";
	file << "# Suite\tTest\tSeconds per run\tLoops\n";

	char seconds[64];
	for (auto& test : m_tests)
	{
		auto& r = test.performance;
		if (r.bDone && !r.bFailed)
		{
			snprintf(seconds, sizeof(seconds), "%.9g", r.secondsPerRun);
			file << m_suites[test.suite].name << '\t' << test.name << '\t' << seconds << '\t' << r.loops << '\n';
		}
	}

	file.close();

	if (!file)
		m_pPerformanceLoadedLabel->display.setText(("Could not save to " + path).c_str());
	else
		m_pPerformanceLoadedLabel->display.setText(("Saved " + std::to_string(nbResults) + " results to " + path).c_str());
}

//____ load_performance_results() _____________________________________________
/*
	Loads results saved by save_performance_results() into the LOADED column.
	Tests are matched by suite and name. Results for tests we don't have are
	ignored, our tests without results get none.
*/

void GfxDeviceTester::load_performance_results()
{
	string path = m_pAPI->openFileDialog("Load Performance Results", "", "", { "*.txt" }, "Performance results");
	if (path.empty())
		return;

	std::ifstream file(path);
	if (!file)
	{
		m_pPerformanceLoadedLabel->display.setText(("Could not open " + path).c_str());
		return;
	}

	std::map<string, double> results;
	string testee;
	string line;

	while (std::getline(file, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (line.empty())
			continue;

		if (line[0] == '#')
		{
			const string testeePrefix = "# Testee: ";
			if (line.compare(0, testeePrefix.size(), testeePrefix) == 0)
				testee = line.substr(testeePrefix.size());
			continue;
		}

		std::istringstream fields(line);
		string suite, name, seconds;
		if (!std::getline(fields, suite, '\t') || !std::getline(fields, name, '\t') || !std::getline(fields, seconds, '\t'))
			continue;

		char* pEnd = nullptr;
		double value = std::strtod(seconds.c_str(), &pEnd);
		if (pEnd == seconds.c_str() || value < 0)
			continue;

		results[suite + '\t' + name] = value;
	}

	if (results.empty())
	{
		m_pPerformanceLoadedLabel->display.setText(("No performance results found in " + path).c_str());
		return;
	}

	int nbMatched = 0;
	for (int i = 0; i < (int)m_tests.size(); i++)
	{
		auto& test = m_tests[i];
		auto it = results.find(m_suites[test.suite].name + '\t' + test.name);

		test.loadedSecondsPerRun = it != results.end() ? it->second : -1;
		if (it != results.end())
			nbMatched++;

		refresh_performance_row(i);
	}

	string label = "Loaded " + path;
	if (!testee.empty())
		label += " (testee: " + testee + ")";
	label += ". " + std::to_string(nbMatched) + " of our " + std::to_string(m_tests.size()) + " tests matched";
	int nbUnknown = (int)results.size() - nbMatched;
	if (nbUnknown > 0)
		label += ", " + std::to_string(nbUnknown) + (nbUnknown == 1 ? " result for a test we don't have" : " results for tests we don't have");
	label += ".";

	m_pPerformanceLoadedLabel->display.setText(label.c_str());

	refresh_performance_details();
}

//____ performance_change() ___________________________________________________
/*
	Relative change in time per run from the loaded result, positive if slower.
	Returns false if there is nothing to compare.
*/

bool GfxDeviceTester::performance_change(int test, double& ratio) const
{
	auto& entry = m_tests[test];
	auto& r = entry.performance;

	if (!r.bDone || r.bFailed || entry.loadedSecondsPerRun <= 0)
		return false;

	ratio = r.secondsPerRun / entry.loadedSecondsPerRun - 1.0;
	return true;
}

//____ format_change() ________________________________________________________

string GfxDeviceTester::format_change(double ratio)
{
	// No sign on changes that round to zero.

	if (std::abs(ratio) < 0.0005)
		return "0.0 %";

	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%+.1f %%", ratio * 100.0);
	return buffer;
}

//____ format_time() __________________________________________________________

string GfxDeviceTester::format_time(double seconds)
{
	char buffer[64];

	if (seconds < 0.000001)
		snprintf(buffer, sizeof(buffer), "%.1f ns", seconds * 1000000000.0);
	else if (seconds < 0.001)
		snprintf(buffer, sizeof(buffer), "%.2f us", seconds * 1000000.0);
	else if (seconds < 1.0)
		snprintf(buffer, sizeof(buffer), "%.3f ms", seconds * 1000.0);
	else
		snprintf(buffer, sizeof(buffer), "%.3f s", seconds);

	return buffer;
}
