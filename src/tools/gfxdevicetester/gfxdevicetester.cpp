
#include <cmath>
#include <wg_freetypefont.h>


#include <wondergfx.h>
#include <wondergfxstream.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>
#include <wg_linearbackend.h>

//#include <wg_glbackend.h>


#include "gfxdevicetester.h"


#include "testsuites/testsuite.h"

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
#include "testsuites/rgb565bigendiantests.h"
#include "testsuites/segmenttests.h"
#include "testsuites/tiletests.h"
#include "testsuites/tintblittests.h"
#include "testsuites/tintsegmenttests.h"
#include "testsuites/tinttests.h"
#include "testsuites/wavetests.h"
#include "testsuites/edgemaptests.h"
#include "testsuites/edgemaptoolstests.h"
#include "testsuites/waveformtests.h"
#include "testsuites/samplingtests.h"
#include "testsuites/palettecapacitytests.h"
#include "testsuites/blursourcetests.h"
#include "testsuites/readbacktests.h"
#include "testsuites/edgemaptinttests.h"

#include <wg_softkernels_bgr565srgb_extras.h>


using namespace wg;
using namespace std;


//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return static_cast<WonderApp*>( new GfxDeviceTester() );
}

//____ constructor ____________________________________________________________

GfxDeviceTester::GfxDeviceTester()
{

}

//____ destructor _____________________________________________________________

GfxDeviceTester::~GfxDeviceTester()
{

}

//____ init() _________________________________________________________________

bool GfxDeviceTester::init( wapp::API * pAPI )
{
	m_pAPI = pAPI;

	m_pWindow = wapp::Window::create(pAPI,{ .size = {1332,700}, .title = "GfxDevice Tester"});

	//

	pAPI->initDefaultWidgetKit();
    	
	//
	
	setup_testdevices();

	
	setup_cliplist(ClipList::One);
	setup_tests();

	set_devices(g_testdevices[0], g_testdevices[1]);
	
	if (!setup_chrome())
		return false;

	update_displaymode();

	return true;
}

//____ update() _______________________________________________________________

bool GfxDeviceTester::update()
{
	// Run tests

	if (g_bRefreshPerformance)
	{
		refresh_performance_measurements();
		refresh_performance_display();
		g_bRefreshPerformance = false;
	}

	if (g_bRunComparison)
	{
		run_comparison();
		refresh_compare_display();
		g_bRunComparison = false;
	}

	if (g_displayMode != DisplayMode::Time && g_displayMode != DisplayMode::Compare)
	{
		if( g_pReferenceDevice->needsRedraw() )
			run_tests(g_pReferenceDevice, REFERENCE );
		
		if( g_pTesteeDevice->needsRedraw() )
			run_tests(g_pTesteeDevice, TESTEE );

		
/*
		for( auto pDevice : g_testdevices )
		{
			if( pDevice->needsRedraw() )
				run_tests(pDevice, TESTEE);
		}
*/
	}

	return m_pWindow != nullptr;
}

//____ exit() _________________________________________________________________

void GfxDeviceTester::exit()
{
	teardown_chrome();

	destroy_tests();

	g_pViewScroller = nullptr;
}

//____ closeWindow() __________________________________________________________

void GfxDeviceTester::closeWindow(wapp::Window* pWindow)
{
	m_pWindow = nullptr;
}


//____ setup_testdevices() ___________________________________________________________

void GfxDeviceTester::setup_testdevices()
{
	Surface::Blueprint canvasBP = WGBP(Surface,
									   _.size = {512,512},
									   _.canvas = true );

	// Gen2 with SoftBackend

	{

		auto pSoftBackend = SoftBackend::create();
		addDefaultSoftKernels(pSoftBackend);

//		auto pBackendLogger = BackendLogger::create( &std::cout, pSoftBackend );
	
		auto pGen2GfxDevice = GfxDeviceGen2::create( pSoftBackend );

		auto pGen2CanvasSurface = SoftSurface::create(canvasBP);
		auto pGen2SoftDevice = Device::create("Gen2 Software (SoftBackend)", pGen2GfxDevice, CanvasRef::None, pGen2CanvasSurface );

		g_testdevices.push_back(pGen2SoftDevice);

	}

	// Gen2 with GlBackend
/*
	{

		auto pBackend = GlBackend::create();

//		auto pBackendLogger = BackendLogger::create(std::cout, pBackend);
//		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackendLogger);

		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackend);

		auto pCanvasSurface = GlSurface::create(canvasBP);
		auto pGen2GlDevice = Device::create("Gen2 OpenGL (GlBackend)", pGen2GfxDevice, CanvasRef::None, pCanvasSurface, this);

		g_testdevices.push_back(pGen2GlDevice);

 }
*/
	// Gen2 with LinearBackend

	{
		auto pLinearOutputBlob = Blob::create(512*512*4);
		m_pSavedBlob = pLinearOutputBlob;

		auto pLinearBackend = LinearBackend::create(

			[this](CanvasRef ref, int bytes)
			{
				assert( bytes <= 512*512*4 );

				return m_pSavedBlob->data();
			},
			[this](CanvasRef ref, int nSegments, const LinearBackend::Segment * pSegments )
			{
				auto buffer = m_pLinearBackendSurface->allocPixelBuffer();

				int pixelBytes = Util::pixelFormatToDescription(buffer.format).bits/8;

				for( int i = 0 ; i < nSegments ; i++ )
				{
					auto& seg = pSegments[i];

					uint8_t * pDest = ((uint8_t*)buffer.pixels) + seg.rect.y * buffer.pitch + seg.rect.x * pixelBytes;
					uint8_t * pSrc = seg.pBuffer;

					for( int y = 0 ; y < seg.rect.h ; y++ )
					{
						memcpy( pDest, pSrc, seg.rect.w*pixelBytes );
						pDest += buffer.pitch;
						pSrc += seg.pitch;
					}
				}

				m_pLinearBackendSurface->pullPixels(buffer);
				m_pLinearBackendSurface->freePixelBuffer(buffer);
			}
			);

		addDefaultSoftKernels( pLinearBackend );
		pLinearBackend->defineCanvas(CanvasRef::Default, g_canvasSize*64, PixelFormat::BGRA_8_sRGB);

		auto pGfxDevice = GfxDeviceGen2::create(pLinearBackend);

		auto pLinearDevice = Device::create("Gen2 Linear (LinearBackend)", pGfxDevice, CanvasRef::Default, nullptr );

		m_pLinearBackendSurface = pLinearDevice->displaySurface();

//		g_testdevices.push_back(pLinearDevice);
	}


	// Native

	auto pNativeGfxDevice = Base::defaultGfxDevice();
	string nativeDeviceName = string("Native (" + string(pNativeGfxDevice->typeInfo().className) + ")" );
	
	auto pNativeDevice = Device::create(nativeDeviceName, pNativeGfxDevice, CanvasRef::None, Base::defaultSurfaceFactory()->createSurface(canvasBP));
	
	g_testdevices.push_back(pNativeDevice);


	// Gen2 Software BGR_565_sRGB

	{
		Surface::Blueprint canvasBP565srgbBP = WGBP(Surface,
												  _.size = {512,512},
												  _.format = PixelFormat::BGR_565_sRGB,
												  _.canvas = true );

		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels( pBackend );
		addExtraSoftKernelsForBGR565sRGBCanvas( pBackend );

		auto pSoftGfxDevice = GfxDeviceGen2::create(pBackend);
		auto pCanvasSurface = SoftSurface::create( canvasBP565srgbBP );

		pCanvasSurface->fill( Color::Green );

		auto pReferenceDevice = Device::create( "Gen2 Software BGR565sRGB (SoftBackend)", pSoftGfxDevice, CanvasRef::None, pCanvasSurface);

//		g_testdevices.push_back(pReferenceDevice);
	}

	// Stream to Gen2 Software

	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);

		auto pBackendLogger = BackendLogger::create(&std::cout, pBackend);
//		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackendLogger);

		auto pGen2CanvasSurface = SoftSurface::create(canvasBP);

		pBackend->defineCanvas(CanvasRef::Default, pGen2CanvasSurface);

		auto pStreamPlayer = StreamPlayer::create(pBackendLogger, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());

		auto pStreamEncoder = StreamEncoder::create({ pStreamPlayer, pStreamPlayer->input });

		auto pStreamBackend = StreamBackend::create(pStreamEncoder);
		pStreamBackend->defineCanvas(CanvasRef::Default, { 512,512 }, PixelFormat::BGRA_8_sRGB);

		auto pStreamGfxDevice = GfxDeviceGen2::create(pStreamBackend);

		auto pStreamDevice = Device::create("Stream to Gen 2 Software", pStreamGfxDevice, CanvasRef::Default, pGen2CanvasSurface);

//		g_testdevices.push_back(pStreamDevice);
	}

	// Gen2 Metal
/*
	{
		auto pDevice = Base::defaultGfxDeviceFactory()->createGfxDevice();

		auto pCanvasSurface = Base::defaultSurfaceFactory()->createSurface(canvasBP);
		auto pGen2GlDevice = Device::create("Gen2 Metal (MetalBackend)", pDevice, CanvasRef::None, pCanvasSurface, this);

//		g_testdevices.push_back(pGen2GlDevice);

	}
*/


}

//____ destroy_testdevices() ____________________________________________________

void GfxDeviceTester::destroy_testdevices()
{
	g_testdevices.clear();

	g_pReferenceDevice = nullptr;
}

//____ update_displaymode() ______________________________________________________

void GfxDeviceTester::update_displaymode()
{

	// Setup view panel

	Widget_p pContent;

	switch (g_displayMode)
	{
		default:
		{
			auto pPack = PackPanel::create();
			pPack->setAxis(Axis::X);

			for( auto pDevice : g_testdevices )
				pPack->slots << pDevice;

			pContent = pPack;
			break;
		}
		case DisplayMode::Diff:

			pContent = Filler::create();		// Not implemented, just avoid a crash.
			break;
		case DisplayMode::Time:
			refresh_performance_display();
			g_pViewScroller->slot = g_pPerformanceDisplay;
			pContent = g_pPerformanceDisplay;
			break;
		case DisplayMode::Compare:
			refresh_compare_display();
			g_pViewScroller->slot = g_pCompareDisplay;
			pContent = g_pCompareDisplay;
			break;
	}

	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pContent);
	pFlex->setDefaultSize(pContent->defaultSize());

	g_pViewScroller->slot = pFlex;
}

//____ refresh_performance_display() __________________________________________

void GfxDeviceTester::refresh_performance_display()
{
//	g_pPerformanceTable->slots.clear();

	g_pPerformanceTable->resize( 1, 3 );

	g_pPerformanceTable->slots[0][0] = TextDisplay::create( { .display = { .layout = g_pPerformanceValueMapper, .style = wkit::TextStyles::Heading6, .text = "TEST" }});
	g_pPerformanceTable->slots[0][1] = TextDisplay::create( { .display = { .layout = g_pPerformanceValueMapper, .style = wkit::TextStyles::Heading6, .text = "REFERENCE" }});
	g_pPerformanceTable->slots[0][2] = TextDisplay::create( { .display = { .layout = g_pPerformanceValueMapper, .style = wkit::TextStyles::Heading6, .text = "TESTEE" }});


	for (TestEntry t : g_tests)
	{
		if (t.bActive)
		{

			auto pLabel = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default } });
			pLabel->display.setText(t.name);

			char value[128];
			snprintf(value, 128, " %.1f + %.1f = %.1f ms", t.devices[TESTEE].render_time * 1000, t.devices[TESTEE].stalling_time * 1000, (t.devices[TESTEE].render_time + t.devices[TESTEE].stalling_time) * 1000);

			auto pValueTestee = TextDisplay::create({ .display = {.layout = g_pPerformanceValueMapper, .style = wkit::TextStyles::Default, .text = value } });


			snprintf(value, 128, " %.1f + %.1f = %.1f ms", t.devices[REFERENCE].render_time * 1000, t.devices[REFERENCE].stalling_time * 1000, (t.devices[REFERENCE].render_time + t.devices[REFERENCE].stalling_time) * 1000 );

			auto pValueRef = TextDisplay::create({ .display = {.layout = g_pPerformanceValueMapper, .style = wkit::TextStyles::Default, .text = value } });

			auto it = g_pPerformanceTable->rows.pushBack();

			int rowIdx = g_pPerformanceTable->rows.size()-1;

			g_pPerformanceTable->slots[rowIdx][0] = pLabel;
			g_pPerformanceTable->slots[rowIdx][1] = pValueRef;
			g_pPerformanceTable->slots[rowIdx][2] = pValueTestee;
		}
 	}

}


//____ create_canvas() _______________________________________________________

SurfaceDisplay_p GfxDeviceTester::create_canvas()
{
	auto pCanvas = SurfaceDisplay::create();
	
	auto pSurface = Base::defaultSurfaceFactory()->createSurface( { .size = g_canvasSize } );
	pCanvas->setSurface(pSurface);
//	pCanvas->canvas.setBackColor(Color::Black);
	return pCanvas;
}

//____ setup_cliplist() _________________________________________________________

void GfxDeviceTester::setup_cliplist(ClipList list)
{
	g_clipList.clear();

	SizeSPX rectSize;
	SizeSPX canvasSize = g_canvasSize * 64;

	switch (list)
	{
	case ClipList::One:
		rectSize = canvasSize;
		break;
	case ClipList::Few:
		rectSize = canvasSize / 8;
		break;
	case ClipList::Many:
		rectSize = canvasSize / 32;
		break;
	}

	CoordSPX rectOfs;

	while (rectOfs.y + rectSize.h <= canvasSize.h)
	{
		while (rectOfs.x + rectSize.w <= canvasSize.w)
		{
			g_clipList.push_back({ rectOfs, rectSize });
			rectOfs.x += rectSize.w * 2;
		}

		if ((rectOfs.x / rectSize.w) % 2 == 1)
			rectOfs.x = 0;
		else
			rectOfs.x = rectSize.w;

		rectOfs.y += rectSize.h;
	}

	if( g_pReferenceDevice )
		g_pReferenceDevice->setNeedsRedraw();

	if( g_pTesteeDevice )
		g_pTesteeDevice->setNeedsRedraw();
}


//____ run_tests() ____________________________________________________________

void GfxDeviceTester::run_tests(Device* pDevice, DeviceEnum device)
{
	if (!pDevice)
		return;

	auto pGfxDevice = pDevice->beginRender();

	pGfxDevice->fill(g_canvasSize*64, HiColor::Black);
	pGfxDevice->setClipList((int)g_clipList.size(), &g_clipList[0]);

	for (auto& test : g_tests)
	{
		if (test.bActive)
			run_test(pGfxDevice, test.devices[device].pTest);
	}

	pGfxDevice->resetClipList();
	pDevice->endRender();
}

//____ run_test() _____________________________________________________________

void GfxDeviceTester::run_test(GfxDevice* pGfxDevice, Test* pTest)
{
	if (pTest->init != nullptr)
		pTest->init(pGfxDevice, g_canvasSize*64);

	pTest->run(pGfxDevice, g_canvasSize*64);

	if (pTest->exit != nullptr)
		pTest->exit(pGfxDevice, g_canvasSize*64);
}


//____ refresh_performance_measurements() ___________________________________________________

void GfxDeviceTester::refresh_performance_measurements()
{
	for (auto& test : g_tests)
	{
		if (test.bActive)
		{
			clock_test(&test.devices[TESTEE], 1000, g_pTesteeDevice);
			clock_test(&test.devices[REFERENCE], 1000, g_pReferenceDevice);
		}
	}
}


//____ clock_test() ___________________________________________________________

void GfxDeviceTester::clock_test(DeviceTest* pDeviceTest, int rounds, Device* pDevice)
{
	uint64_t first = m_pAPI->time();

	uint64_t start;
	uint64_t end;
	uint64_t stallBegin;
	uint64_t stallEnd;

	pDevice->beginRender();
	pDevice->gfxDevice()->setClipList((int)g_clipList.size(), &g_clipList[0]);

	if (pDeviceTest->pTest->init != nullptr)
		pDeviceTest->pTest->init(pDevice->gfxDevice(), g_canvasSize*64);

	do { start = m_pAPI->time(); } while (start == first);

	for (int i = 0; i < rounds; i++)
		pDeviceTest->pTest->run(pDevice->gfxDevice(), g_canvasSize*64);

	end = m_pAPI->time();

	if (pDeviceTest->pTest->exit != nullptr)
		pDeviceTest->pTest->exit(pDevice->gfxDevice(), g_canvasSize*64);

	stallBegin = m_pAPI->time();

	pDevice->endRender();

	stallEnd = m_pAPI->time();


	pDeviceTest->render_time = (end - start)/1000000.0;
	pDeviceTest->stalling_time = (stallEnd - stallBegin)/1000000.0;
}

//____ run_comparison() _______________________________________________________
/*
	Renders every test on its own on both devices, reads back the results and
	compares them. Both are converted to 8 bits per channel in the color space of
	the reference canvas, so devices with different canvas formats can be compared.
*/

void GfxDeviceTester::run_comparison()
{
	Surface_p pRefSurface = g_pReferenceDevice->canvas() ? g_pReferenceDevice->canvas() : g_pReferenceDevice->displaySurface();
	PixelFormat format = Util::pixelFormatToDescription(pRefSurface->pixelFormat()).colorSpace == ColorSpace::Linear ? PixelFormat::BGRA_8_linear : PixelFormat::BGRA_8_sRGB;

	vector<uint8_t> pixels[2];

	for (auto& test : g_tests)
	{
		CompareResult& result = test.compare;
		result = CompareResult();

		bool bOk = true;
		for (int device = 0; device < 2; device++)
		{
			Device* pDevice = device == REFERENCE ? g_pReferenceDevice : g_pTesteeDevice;

			auto pGfxDevice = pDevice->beginRender();
			pGfxDevice->fill(g_canvasSize*64, HiColor::Black);
			pGfxDevice->setClipList((int)g_clipList.size(), &g_clipList[0]);
			run_test(pGfxDevice, test.devices[device].pTest);
			pGfxDevice->resetClipList();
			pDevice->endRender();

			bOk &= read_canvas(pDevice, format, pixels[device]);
		}

		if (!bOk)
			continue;

		const uint8_t* pRef = pixels[REFERENCE].data();
		const uint8_t* pTestee = pixels[TESTEE].data();
		size_t nbValues = pixels[REFERENCE].size();
		int64_t sum = 0;

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
				result.worst = { pixel % g_canvasSize.w, pixel / g_canvasSize.w };
			}
		}

		result.meanDiff = double(sum) / nbValues;
		result.bDone = true;
	}

	// The canvases now show the last test, so get them back to what is selected.

	g_pReferenceDevice->setNeedsRedraw();
	g_pTesteeDevice->setNeedsRedraw();
}

//____ read_canvas() __________________________________________________________

bool GfxDeviceTester::read_canvas(Device* pDevice, PixelFormat format, vector<uint8_t>& pixels)
{
	Surface_p pSurface = pDevice->canvas() ? pDevice->canvas() : pDevice->displaySurface();
	if (!pSurface || pSurface->pixelSize() != g_canvasSize)
		return false;

	pixels.resize(g_canvasSize.w * g_canvasSize.h * 4);

	auto buffer = pSurface->allocPixelBuffer();
	pSurface->pushPixels(buffer);

	int srcPitchAdd = buffer.pitch - g_canvasSize.w * Util::pixelFormatToDescription(buffer.format).bits / 8;
	int dstPaletteEntries = 0;

	bool bOk = PixelTools::copyPixels(g_canvasSize.w, g_canvasSize.h, buffer.pixels, buffer.format, srcPitchAdd,
									  pixels.data(), format, 0, buffer.palette, nullptr, 256, dstPaletteEntries, 0);

	pSurface->freePixelBuffer(buffer);
	return bOk;
}

//____ refresh_compare_display() ______________________________________________

void GfxDeviceTester::refresh_compare_display()
{
	for (auto route : g_compareRoutes)
		Base::msgRouter()->deleteRoute(route);
	g_compareRoutes.clear();

	g_pCompareTable->resize(1, 6);

	const char* headings[6] = { "TEST", "MAX", "MEAN", "OVER", "WORST AT", "RESULT" };
	for (int col = 0; col < 6; col++)
		g_pCompareTable->slots[0][col] = TextDisplay::create({ .display = {.layout = col == 0 ? nullptr : g_pPerformanceValueMapper, .style = wkit::TextStyles::Heading6, .text = headings[col] } });

	// Worst first, so what needs looking at is at the top.

	vector<TestEntry*> sorted;
	for (auto& test : g_tests)
	{
		if (test.compare.bDone)
			sorted.push_back(&test);
	}

	std::stable_sort(sorted.begin(), sorted.end(), [](const TestEntry* a, const TestEntry* b) {
		if (a->compare.nbOverThreshold != b->compare.nbOverThreshold)
			return a->compare.nbOverThreshold > b->compare.nbOverThreshold;
		return a->compare.maxDiff > b->compare.maxDiff;
	});

	int nbOk = 0, nbSpecks = 0, nbDiff = 0;

	for (TestEntry* pTest : sorted)
	{
		const CompareResult& r = pTest->compare;

		const char* pVerdict;
		TextStyle_p pStyle;

		if (r.nbOverThreshold == 0)
		{
			pVerdict = "OK";
			pStyle = g_pOkStyle;
			nbOk++;
		}
		else if (r.nbOverThreshold <= c_maxSpecks)
		{
			pVerdict = "SPECKS";
			pStyle = g_pSpecksStyle;
			nbSpecks++;
		}
		else
		{
			pVerdict = "DIFF";
			pStyle = g_pDiffStyle;
			nbDiff++;
		}

		char values[5][32];
		snprintf(values[0], 32, "%d", r.maxDiff);
		snprintf(values[1], 32, "%.2f", r.meanDiff);
		snprintf(values[2], 32, "%d", r.nbOverThreshold);
		if (r.maxDiff > 0)
			snprintf(values[3], 32, "%d, %d", r.worst.x, r.worst.y);
		else
			values[3][0] = 0;
		snprintf(values[4], 32, "%s", pVerdict);

		g_pCompareTable->rows.pushBack();
		int row = g_pCompareTable->rows.size() - 1;

		// The test name is a button that shows the test on its own.

		auto pButton = WGCREATE(wkit::Button, _.label.text = pTest->name.c_str());
		int index = int(pTest - &g_tests[0]);
		g_compareRoutes.push_back(Base::msgRouter()->addRoute(pButton, MsgType::Select, [this, index](Msg* pMsg) { show_test(index); }));
		g_pCompareTable->slots[row][0] = pButton;
		for (int col = 1; col < 6; col++)
			g_pCompareTable->slots[row][col] = TextDisplay::create({ .display = {.layout = g_pPerformanceValueMapper,
				.style = col == 5 ? pStyle : wkit::TextStyles::Default, .text = values[col - 1] } });
	}

	char summary[128];
	if (sorted.empty())
		snprintf(summary, 128, "Press RUN ALL to render every test on both devices and compare.");
	else
		snprintf(summary, 128, "%d tests: %d OK, %d with specks, %d differ. Click a test to view it.", (int)sorted.size(), nbOk, nbSpecks, nbDiff);
	g_pCompareSummary->display.setText(summary);
}

//____ show_test() ____________________________________________________________
/*
	Makes the test the only one selected and shows both devices rendering it.
	Selecting from code doesn't post a message for the selected entry, so the
	test entries are updated here.
*/

void GfxDeviceTester::show_test(int index)
{
	g_pTestSelector->unselectAll();
	g_pTestSelector->select(g_testListEntries[index]);

	for (auto& test : g_tests)
		test.bActive = false;
	g_tests[index].bActive = true;

	g_pReferenceDevice->setNeedsRedraw();
	g_pTesteeDevice->setNeedsRedraw();

	g_displayMode = DisplayMode::Both;
	update_displaymode();
}

//____ setup_tests() ____________________________________________________

void GfxDeviceTester::setup_tests()
{
	destroy_tests();

	add_testsuite([](){ return new BlurTests();});
	add_testsuite([](){ return new BlitConsistencyTests();});
	add_testsuite([](){ return new FillTests();});
	add_testsuite([](){ return new BlendTests();});
	add_testsuite([](){ return new LineTests();});
	add_testsuite([](){ return new CanvasFormatTests();});
	add_testsuite([](){ return new PatchBlitTests();});
	add_testsuite([](){ return new SegmentTests();});
	add_testsuite([](){ return new WaveTests();});
	add_testsuite([](){ return new PaletteBlitTests();});
	add_testsuite([](){ return new MipmapTests();});
	add_testsuite([](){ return new PieChartTests();});
	add_testsuite([](){ return new TintBlitTests();});
	add_testsuite([](){ return new TintTests();});
	add_testsuite([](){ return new BlitBlendTests();});
	add_testsuite([](){ return new A8Tests();});
	add_testsuite([](){ return new TileTests();});
	add_testsuite([](){ return new CanvasLayerTests();});
//	add_testsuite([](){ return new RGB565BigEndianTests();});
	add_testsuite([](){ return new BlendFixedColorTests();});
	add_testsuite([](){ return new EdgemapTests();});
	add_testsuite([](){ return new EdgemapToolsTests();});
	add_testsuite([](){ return new WaveformTests();});
	add_testsuite([]() { return new ColorTests(); });
	add_testsuite([]() { return new SamplingTests(); });
	add_testsuite([]() { return new PaletteCapacityTests(); });
	add_testsuite([]() { return new BlurSourceTests(); });
	add_testsuite([]() { return new ReadbackTests(); });
	add_testsuite([]() { return new EdgemapTintTests(); });

	regen_testentries();
}

//____ set_devices() ___________________________________________________________

bool GfxDeviceTester::set_devices( Device_p pReference, Device_p pTestee )
{
	for( auto& se : g_testsuites )
	{
		if( se.bActive )
		{
			if( g_pReferenceDevice )
				se.pReferenceSuite->exit( g_pReferenceDevice->gfxDevice(), g_canvasSize*64 );
			if( g_pTesteeDevice )
				se.pTesteeSuite->exit( g_pTesteeDevice->gfxDevice(), g_canvasSize*64 );

			se.pReferenceSuite->init(pReference->gfxDevice(), g_canvasSize*64, m_pAPI);
			se.pTesteeSuite->init(pTestee->gfxDevice(), g_canvasSize*64, m_pAPI);
		}
	}
	
	g_pReferenceDevice = pReference;
	g_pTesteeDevice = pTestee;
	
	return true;
}


//____ add_testsuite() _____________________________________________________________

bool GfxDeviceTester::add_testsuite( const std::function<TestSuite*()>& testSuiteFactory)
{
	SuiteEntry se;
	
	se.factory 			= testSuiteFactory;
	se.pReferenceSuite 	= testSuiteFactory();
	se.pTesteeSuite		= testSuiteFactory();
	se.nbTests 			= (int)se.pReferenceSuite->tests.size();

	auto pBackend = SoftBackend::create();
	auto pTempDevice = GfxDeviceGen2::create(pBackend);
	bool bWorking = se.pReferenceSuite->init(pTempDevice, g_canvasSize*64, m_pAPI);
	se.pReferenceSuite->exit(pTempDevice, g_canvasSize*64);
	
	se.bWorking = bWorking;
	se.bActive	= bWorking;
	
	g_testsuites.push_back(se);

	return bWorking;
}

//____ regen_testentries() ____________________________________________________

void GfxDeviceTester::regen_testentries()
{
	g_tests.clear();

	for (auto& suite : g_testsuites)
	{
		if (suite.bActive)
		{
			for (int i = 0; i < suite.nbTests; i++)
			{
				TestEntry t;

				t.bWorking = suite.bWorking;
				t.bActive = false;
				t.name = suite.pReferenceSuite->tests[i].name;

				t.devices[TESTEE].pTest = &suite.pTesteeSuite->tests[i];
				t.devices[REFERENCE].pTest = &suite.pReferenceSuite->tests[i];

				g_tests.push_back(t);
			}
		}
	}
}


//____ destroy_tests() ____________________________________________________

void GfxDeviceTester::destroy_tests()
{
	for (auto& suite : g_testsuites)
	{
		delete suite.pReferenceSuite;
		delete suite.pTesteeSuite;
	}

	g_tests.clear();
	g_testsuites.clear();
}


//____ setup_chrome() _________________________________________________________

bool GfxDeviceTester::setup_chrome()
{

	//    BoxStateSkin_p pPlateSkin = BoxSkin::create( Border(2), Color::Red, Color::Blue );
	//    BoxStateSkin_p pPressablePlateSkin = pPlateSkin;
	//    BoxStateSkin_p pSimpleButtonSkin = pPlateSkin;

		// Setup base layers

	auto pLayerStack = StackPanel::create();
	pLayerStack->setSkin( ColorSkin::create(Color::AntiqueWhite) );
	m_pWindow->mainCapsule()->slot = pLayerStack;

//	m_pWindow->rootPanel()->setDebugMode(true);

	auto pUniformLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
		.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Weight });

	

	// Divid screen into sidebar and canvaspanel with top section

	auto pMidSection = PackPanel::create();
	pMidSection->setAxis(Axis::X);
	pMidSection->setLayout(pUniformLayout);
	pLayerStack->slots << pMidSection;
	pLayerStack->slots.back().setSizePolicy(SizePolicy2D::Stretch);

	auto pSidebar = PackPanel::create();
	pSidebar->setAxis(Axis::Y);
	pSidebar->setSkin( wkit::Skins::Plate );
	pSidebar->setLayout(pUniformLayout);

	auto pCanvasPanel = PackPanel::create();
	pCanvasPanel->setAxis(Axis::Y);
	pCanvasPanel->setLayout(pUniformLayout);

	auto pViewNav = PackPanel::create();
	pViewNav->setAxis(Axis::X);
	pViewNav->setLayout(pUniformLayout);
	pViewNav->setSkin(wkit::Skins::Plate );

	auto pViewScroller = wkit::ScrollCapsuleXY::create();
	g_pViewScroller = pViewScroller;

	pMidSection->slots << pCanvasPanel;
	pMidSection->slots << pSidebar;

	pMidSection->slots[0].setWeight(1.f);
	pMidSection->slots[1].setWeight(0.f);


	pCanvasPanel->slots << pViewNav;
	pCanvasPanel->slots << pViewScroller;

	pCanvasPanel->slots[0].setWeight(0.f);
	pCanvasPanel->slots[1].setWeight(1.f);


	// Setup view navigator

	auto pClipSection = PackPanel::create();
	auto pLeftFiller = Filler::create();
	auto pDispModeSection = PackPanel::create();
	auto pRightFiller = Filler::create();
	auto pDispZoomSection = PackPanel::create();

	pViewNav->slots << pClipSection;
	pViewNav->slots << pLeftFiller;
	pViewNav->slots << pDispModeSection;
	pViewNav->slots << pRightFiller;
	pViewNav->slots << pDispZoomSection;

	pViewNav->slots[0].setWeight(0.f);
	pViewNav->slots[1].setWeight(1.f);
	pViewNav->slots[2].setWeight(0.f);
	pViewNav->slots[3].setWeight(1.f);
	pViewNav->slots[4].setWeight(0.f);

	// Setup clip mode section

	pClipSection->setAxis(Axis::X);
	pClipSection->setLayout(pUniformLayout);

	auto pClipLabel = TextDisplay::create();
	pClipLabel->display.setText("ClipRects: ");
	pClipLabel->display.setStyle( wkit::TextStyles::Strong );


	auto pNoClipButton = WGCREATE( wkit::Button, _.label.text = "One");
	auto pFewButton = WGCREATE( wkit::Button, _.label.text = "Few");
	auto pManyButton = WGCREATE( wkit::Button, _.label.text = "Many");

	pClipSection->slots << pClipLabel;
	pClipSection->slots << pNoClipButton;
	pClipSection->slots << pFewButton;
	pClipSection->slots << pManyButton;

	Base::msgRouter()->addRoute(pNoClipButton, MsgType::Select, [this](Msg* pMsg) { setup_cliplist(ClipList::One); });
	Base::msgRouter()->addRoute(pFewButton, MsgType::Select, [this](Msg* pMsg) { setup_cliplist(ClipList::Few); });
	Base::msgRouter()->addRoute(pManyButton, MsgType::Select, [this](Msg* pMsg) { setup_cliplist(ClipList::Many); });

	// Setup display mode section

	pDispModeSection->setAxis(Axis::X);
	pDispModeSection->setLayout(pUniformLayout);


	auto pTesteeButton = WGCREATE( wkit::Button, _.label.text = "Testee");
	auto pRefButton = WGCREATE(wkit::Button, _.label.text = "Reference");
	auto pBothButton = WGCREATE(wkit::Button, _.label.text = "Both");
	auto pDiffButton = WGCREATE(wkit::Button, _.label.text = "Diff");
	auto pTimeButton = WGCREATE(wkit::Button, _.label.text = "Time");
	auto pCompareButton = WGCREATE(wkit::Button, _.label.text = "Compare");

	pDispModeSection->slots << pTesteeButton;
	pDispModeSection->slots << pRefButton;
	pDispModeSection->slots << pBothButton;
	pDispModeSection->slots << pDiffButton;
	pDispModeSection->slots << pTimeButton;
	pDispModeSection->slots << pCompareButton;

	Base::msgRouter()->addRoute(pTesteeButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Testee;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pRefButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Reference;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pBothButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Both;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pDiffButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Diff;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pTimeButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Time;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pCompareButton, MsgType::Select, [this](Msg* pMsg) {
		g_displayMode = DisplayMode::Compare;
		update_displaymode();
	});

	// Setup display zoom section

	pDispModeSection->setAxis(Axis::X);
	pDispModeSection->setLayout(pUniformLayout);

	auto pX1Button = WGCREATE(wkit::Button, _.label.text = "X1");
	auto pX2Button = WGCREATE(wkit::Button, _.label.text = "X2");
	auto pX4Button = WGCREATE(wkit::Button, _.label.text = "X4");
	auto pX8Button = WGCREATE(wkit::Button, _.label.text = "X8");

	pDispZoomSection->slots << pX1Button;
	pDispZoomSection->slots << pX2Button;
	pDispZoomSection->slots << pX4Button;
	pDispZoomSection->slots << pX8Button;

	Base::msgRouter()->addRoute(pX1Button, MsgType::Select, [this](Msg* pMsg) {
		g_zoomFactor = 1.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX2Button, MsgType::Select, [this](Msg* pMsg) {
		g_zoomFactor = 2.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX4Button, MsgType::Select, [this](Msg* pMsg) {
		g_zoomFactor = 4.f;
		update_displaymode();
	});

	Base::msgRouter()->addRoute(pX8Button, MsgType::Select, [this](Msg* pMsg) {
		g_zoomFactor = 8.f;
		update_displaymode();
	});

	// Setup sidebar

	auto pTestList = PackPanel::create({ .axis = Axis::Y });

	auto pSelectCapsule = SelectCapsule::create({ .child = pTestList, .selectMode = SelectMode::MultiEntries });
	g_pTestSelector = pSelectCapsule;



	Base::msgRouter()->addRoute(pSelectCapsule, MsgType::Selected, [&](Msg* _pMsg) {

		auto pMsg = static_cast<SelectedMsg*>(_pMsg);
		
		auto p = pMsg->selected();
		for (int x = 0; x < pMsg->nbSelected(); x++)
			g_tests[p[x]->id()].bActive = true;

		g_pReferenceDevice->setNeedsRedraw();
		g_pTesteeDevice->setNeedsRedraw();
	});

	Base::msgRouter()->addRoute(pSelectCapsule, MsgType::Unselected, [&](Msg* _pMsg) {

		auto pMsg = static_cast<UnselectedMsg*>(_pMsg);

		auto p = pMsg->unselected();
		for (int x = 0; x < pMsg->nbUnselected(); x++)
			g_tests[p[x]->id()].bActive = false;

		g_pReferenceDevice->setNeedsRedraw();
		g_pTesteeDevice->setNeedsRedraw();
		});


	auto pSkin = BoxSkin::create(1, Color::White, Color::Black, 8);

	auto pEntrySkin = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Black, .outlineThickness = 1, .padding = 8,
			.states = {
				{State::Hovered, {.color = Color::AntiqueWhite } },
				{State::Selekted + State::Hovered, {.color = Color::AntiqueWhite } },
				{State::Selekted, {.color = Color::Aquamarine }}
			}
	});

	pTestList->setSkin( pSkin );

	int id = 0;
	for (TestEntry& test : g_tests)
	{
		auto pEntry = TextDisplay::create( {
			.display = {.style = wkit::TextStyles::Default, .text = test.name.c_str() },
			.id = id++,
			.selectable = true,
			.skin = pEntrySkin });
		pTestList->slots.pushBack(pEntry);
		g_testListEntries.push_back(pEntry);
	}

	auto pTestScrollPanel = wkit::ScrollCapsuleY::create();
	pTestScrollPanel->slot = pSelectCapsule;

	pSidebar->slots << pTestScrollPanel;


	// Setup performance display

	{
		auto pMapper = BasicTextLayout::create({ .placement = Placement::East });
		g_pPerformanceValueMapper = pMapper;

		g_pPerformanceEntryLayout = PackLayout::create( { .expandFactor = PackLayout::Factor::One } );


		auto pBase = PackPanel::create();
		pBase->setAxis(Axis::Y);

		auto pTable = TablePanel::create( { .columnSpacing = 10, .rowSpacing = 4, .skin = ColorSkin::create( { .color = Color::White, .padding = 4 }) });

		auto pOddEntrySkin = BoxSkin::create(0, Color::White, Color::White);
		auto pEvenEntrySkin = BoxSkin::create(0, Color::PapayaWhip, Color::PapayaWhip);

		pTable->setRowSkins(pOddEntrySkin, pEvenEntrySkin);


		// Create the bottom section

		auto pBottom = PackPanel::create();
		pBottom->setSkin(wkit::Skins::Plate);
		pBottom->setAxis(Axis::X);
		pBottom->setLayout(pUniformLayout);

		auto pRefresh = WGCREATE( wkit::Button, _.label.text = "REFRESH");

		Base::msgRouter()->addRoute(pRefresh, MsgType::Select, [this](Msg* pMsg) {
			g_bRefreshPerformance = true;
		});


		auto pFiller = Filler::create();

		pBottom->slots << pFiller;
		pBottom->slots << pRefresh;

		//
  
		pBase->slots << pTable;
		pBase->slots << pBottom;

		g_pPerformanceDisplay = pBase;
		g_pPerformanceTable = pTable;
	}

	// Setup compare display

	{
		auto verdictStyle = [](HiColor color) {
			auto bp = wkit::TextStyles::Strong->blueprint();
			bp.color = color;
			return TextStyle::create(bp);
		};

		g_pOkStyle = verdictStyle(Color::DarkGreen);
		g_pSpecksStyle = verdictStyle(Color::DarkOrange);
		g_pDiffStyle = verdictStyle(Color::Red);

		auto pBase = PackPanel::create();
		pBase->setAxis(Axis::Y);

		auto pTable = TablePanel::create( { .columnSpacing = 10, .rowSpacing = 4, .skin = ColorSkin::create( { .color = Color::White, .padding = 4 }) });

		pTable->setRowSkins(BoxSkin::create(0, Color::White, Color::White), BoxSkin::create(0, Color::PapayaWhip, Color::PapayaWhip));

		auto pBottom = PackPanel::create();
		pBottom->setSkin(wkit::Skins::Plate);
		pBottom->setAxis(Axis::X);
		pBottom->setLayout(pUniformLayout);

		auto pSummary = TextDisplay::create({ .display = {.style = wkit::TextStyles::Default } });

		auto pRunAll = WGCREATE( wkit::Button, _.label.text = "RUN ALL");

		Base::msgRouter()->addRoute(pRunAll, MsgType::Select, [this](Msg* pMsg) {
			g_bRunComparison = true;
		});

		pBottom->slots << pSummary;
		pBottom->slots << pRunAll;
		pBottom->slots[0].setWeight(1.f);
		pBottom->slots[1].setWeight(0.f);

		pBase->slots << pTable;
		pBase->slots << pBottom;

		g_pCompareDisplay = pBase;
		g_pCompareTable = pTable;
		g_pCompareSummary = pSummary;
	}

	return true;
}

//____ teardown_chrome() ______________________________________________________

void GfxDeviceTester::teardown_chrome()
{
}
