
#include <cmath>
#include <wg_freetypefont.h>

#include "gfxdevicetester.h"

#include <wondergfx.h>
#include <wondergfxstream.h>
#include <wg_softbackend.h>
#include <wg_softbackend_kernels.h>
#include <wg_linearbackend.h>

#include <wg_glbackend.h>

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
#include "testsuites/wavetests.h"
#include "testsuites/edgemaptests.h"
#include "testsuites/edgemaptoolstests.h"
#include "testsuites/waveformtests.h"

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

bool GfxDeviceTester::init( WonderApp::Visitor * pVisitor )
{
	m_pVisitor = pVisitor;

	m_pWindow = pVisitor->createWindow({ .size = {1332,700}, .title = "GfxDevice Tester" });

	//
    
	// Init textmappers

	auto pMapper = BasicTextLayout::create({ .placement = Placement::Center });
	g_pButtonLabelMapper = pMapper;

	// Init fonts

	Blob_p pFontFile = pVisitor->loadBlob("resources/DroidSans.ttf");
	FreeTypeFont_p pFont = FreeTypeFont::create(pFontFile);

	TextStyle_p pStyle = TextStyle::create({ .color = HiColor::Black, .font = pFont, .size = 16 });
	Base::setDefaultStyle(pStyle);

	g_pButtonLabelStyle = TextStyle::create({ .color = HiColor::Black, .font = pFont, .size = 16 });

	m_pSmallTextStyle = TextStyle::create({ .color = HiColor::Black, .font = pFont, .size = 10 });

	
	//

	setup_theme();
	
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

	if (g_displayMode != DisplayMode::Time)
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

	return true;
}

//____ exit() _________________________________________________________________

void GfxDeviceTester::exit()
{
	teardown_chrome();

	destroy_tests();

	g_pButtonLabelStyle = nullptr;
	g_pViewPanel = nullptr;
}

//____ setup_testdevices() ___________________________________________________________

void GfxDeviceTester::setup_testdevices()
{
	Surface::Blueprint canvasBP = WGBP(Surface,
									   _.size = {512,512},
									   _.canvas = true );
	
	// Reference
	
	auto pSoftGfxDevice = SoftGfxDevice::create();
	addDefaultSoftKernels( pSoftGfxDevice );
	auto pCanvasSurface = SoftSurface::create( canvasBP );
	
	auto pReferenceDevice = Device::create( "Reference (SoftGfxDevice)", pSoftGfxDevice, CanvasRef::None, pCanvasSurface, this );

//	g_testdevices.push_back(pReferenceDevice);

	// Gen2 with SoftBackend

	{
/*
		auto pSoftBackend = SoftBackend::create();
		addDefaultSoftKernels(pSoftBackend);

		auto pBackendLogger = BackendLogger::create( std::cout, pSoftBackend );
	
		auto pGen2GfxDevice = GfxDeviceGen2::create( pBackendLogger );

		auto pGen2CanvasSurface = SoftSurface::create(canvasBP);
		auto pGen2SoftDevice = Device::create("Gen2 Software (SoftBackend)", pGen2GfxDevice, CanvasRef::None, pGen2CanvasSurface, this);

		g_testdevices.push_back(pGen2SoftDevice);
 */
	}

	// Gen2 with GlBackend

	{
/*
		auto pBackend = GlBackend::create();

//		auto pBackendLogger = BackendLogger::create(std::cout, pBackend);
//		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackendLogger);

		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackend);

		auto pCanvasSurface = GlSurface::create(canvasBP);
		auto pGen2GlDevice = Device::create("Gen2 OpenGL (GlBackend)", pGen2GfxDevice, CanvasRef::None, pCanvasSurface, this);

//		g_testdevices.push_back(pGen2GlDevice);
 */

 }

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

		auto pLinearDevice = Device::create("Gen2 Linear (LinearBackend)", pGfxDevice, CanvasRef::Default, nullptr, this );

		m_pLinearBackendSurface = pLinearDevice->displaySurface();

		g_testdevices.push_back(pLinearDevice);
	}


	// Native

	auto pNativeGfxDevice = Base::defaultGfxDevice();
	string nativeDeviceName = string("Native (" + string(pNativeGfxDevice->typeInfo().className) + ")" );
	
	auto pNativeDevice = Device::create(nativeDeviceName, pNativeGfxDevice, CanvasRef::None, Base::defaultSurfaceFactory()->createSurface(canvasBP), this );
	
	g_testdevices.push_back(pNativeDevice);

	// Linear
	
	auto pLinearOutputBlob = Blob::create(512*512*4);
	m_pSavedBlob = pLinearOutputBlob;
		
	auto pLinearGfxDevice = LinearGfxDevice::create(

		[this](CanvasRef ref, int bytes)
		{
			assert( bytes <= 512*512*4 );
			
			return m_pSavedBlob->data();
		},
		[this](CanvasRef ref, int nSegments, const LinearGfxDevice::Segment * pSegments )
		{
			auto buffer = m_pLinearDeviceSurface->allocPixelBuffer();
			
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

			m_pLinearDeviceSurface->pullPixels(buffer);
			m_pLinearDeviceSurface->freePixelBuffer(buffer);
		}
	);
	
	addDefaultSoftKernels( pLinearGfxDevice );
	pLinearGfxDevice->defineCanvas(CanvasRef::Default, g_canvasSize*64, PixelFormat::BGRA_8_sRGB);
	
	auto pLinearDevice = Device::create( pLinearGfxDevice->typeInfo().className, pLinearGfxDevice, CanvasRef::Default, nullptr, this );
	
	m_pLinearDeviceSurface = pLinearDevice->displaySurface();
	
//	g_testdevices.push_back(pLinearDevice);

	// Stream to Software

	{
		auto pSoftGfxDevice = SoftGfxDevice::create();
		addDefaultSoftKernels( pSoftGfxDevice );

		auto pCanvasSurface = SoftSurface::create(canvasBP);		
		pSoftGfxDevice->defineCanvas(CanvasRef::Default, pCanvasSurface);
		
		auto pStreamPlayer = StreamPlayer::create(pSoftGfxDevice, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
		
		auto pStreamEncoder = StreamFastEncoder::create( {pStreamPlayer, pStreamPlayer->input} );
		auto pStreamGfxDevice = StreamDevice::create(pStreamEncoder);
		pStreamGfxDevice->defineCanvas(CanvasRef::Default, {512,512}, PixelFormat::BGRA_8_sRGB);
		
		auto pStreamDevice = Device::create( "Stream to Software", pStreamGfxDevice, CanvasRef::Default, pCanvasSurface, this );

//		g_testdevices.push_back(pStreamDevice);
	}

	// Software BGR_565_sRGB

	{
		Surface::Blueprint canvasBP565srgbBP = WGBP(Surface,
												  _.size = {512,512},
												  _.format = PixelFormat::BGR_565_sRGB,
												  _.canvas = true );

		auto pSoftGfxDevice = SoftGfxDevice::create();
		addDefaultSoftKernels( pSoftGfxDevice );
		addExtraSoftKernelsForBGR565sRGBCanvas( pSoftGfxDevice );
		auto pCanvasSurface = SoftSurface::create( canvasBP565srgbBP );

		pCanvasSurface->fill( Color::Green );

		auto pReferenceDevice = Device::create( "Software BGR565sRGB (SoftGfxDevice)", pSoftGfxDevice, CanvasRef::None, pCanvasSurface, this );

//		g_testdevices.push_back(pReferenceDevice);
	}

	// Stream to Gen2 Software

	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);

		auto pBackendLogger = BackendLogger::create(std::cout, pBackend);
		auto pGen2GfxDevice = GfxDeviceGen2::create(pBackendLogger);

		auto pGen2CanvasSurface = SoftSurface::create(canvasBP);
		auto pGen2SoftDevice = Device::create("Gen2 Software (SoftBackend)", pGen2GfxDevice, CanvasRef::None, pGen2CanvasSurface, this);

		pBackend->defineCanvas(CanvasRef::Default, pGen2CanvasSurface);

		auto pStreamPlayer = StreamPlayer::create(pGen2GfxDevice, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());

		auto pStreamEncoder = StreamFastEncoder::create({ pStreamPlayer, pStreamPlayer->input });
		auto pStreamGfxDevice = StreamDevice::create(pStreamEncoder);
		pStreamGfxDevice->defineCanvas(CanvasRef::Default, { 512,512 }, PixelFormat::BGRA_8_sRGB);

		auto pStreamDevice = Device::create("Stream to Gen 2 Software", pStreamGfxDevice, CanvasRef::Default, pGen2CanvasSurface, this);

//		g_testdevices.push_back(pStreamDevice);
	}


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


	switch (g_displayMode)
	{
		default:
		{
			auto pPack = PackPanel::create();
			pPack->setAxis(Axis::X);

			for( auto pDevice : g_testdevices )
				pPack->slots << pDevice;

			g_pViewPanel->slot = pPack;

			break;
		}
		case DisplayMode::Diff:
			break;
		case DisplayMode::Time:
			refresh_performance_display();
			g_pViewPanel->slot = g_pPerformanceDisplay;
			break;
	}


}

//____ refresh_performance_display() __________________________________________

void GfxDeviceTester::refresh_performance_display()
{
	g_pPerformanceList->slots.clear();

	for (TestEntry t : g_tests)
	{
		if (t.bActive)
		{
			auto pEntry = PackPanel::create();
			pEntry->setAxis(Axis::X);
			pEntry->setLayout(g_pPerformanceEntryLayout);

			auto pLabel = TextDisplay::create();
			pLabel->display.setText(t.name);

			//			auto pValue = NumberDisplay::create();
			//			pValue->value.set(t.devices[TESTEE].render_time*1000);

			auto pValuePacker = PackPanel::create();
			pValuePacker->setAxis(Axis::Y);

			char value[128];

			auto pValueTestee = TextDisplay::create();

			snprintf(value, 128, " %.1f ms (+ %1.f stalling)", t.devices[TESTEE].render_time * 1000, t.devices[TESTEE].stalling_time * 1000);

			pValueTestee->display.setText(value);
			pValueTestee->display.setLayout(g_pPerformanceValueMapper);

			auto pValueRef = TextDisplay::create();

			snprintf(value, 128, " %.1f ms (+ %1.f stalling)", t.devices[REFERENCE].render_time * 1000, t.devices[REFERENCE].stalling_time * 1000);
			pValueRef->display.setText(value);
			pValueRef->display.setLayout(g_pPerformanceValueMapper);

			pValuePacker->slots << pValueTestee;
			pValuePacker->slots << pValueRef;

			pEntry->slots << pLabel;
			pEntry->slots << pValuePacker;

			//			std::map<int,Widget_p> v;
			//			v[1] = Button::create();

			std::vector<Widget_p> v;
			v.push_back(Button::create());

			//			pEntry->slots.add(1, 2);
			pEntry->slots.pushBack(v.begin(), v.end());

			pEntry->setSlotWeight(0, 2, { 0.f,1.f });
			//			pEntry->setSlotPadding(1, { 0,0,0,10 });

			g_pPerformanceList->slots << pEntry;
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
		{
			if (test.devices[device].pTest->init != nullptr)
				test.devices[device].pTest->init(pGfxDevice, g_canvasSize*64);

			test.devices[device].pTest->run(pGfxDevice, g_canvasSize*64);

			if (test.devices[device].pTest->exit != nullptr)
				test.devices[device].pTest->exit(pGfxDevice, g_canvasSize*64);
		}
	}

	pGfxDevice->resetClipList();
	pDevice->endRender();
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
	uint64_t first = m_pVisitor->time();

	uint64_t start;
	uint64_t end;
	uint64_t stallBegin;
	uint64_t stallEnd;

	pDevice->beginRender();
	pDevice->gfxDevice()->setClipList((int)g_clipList.size(), &g_clipList[0]);

	if (pDeviceTest->pTest->init != nullptr)
		pDeviceTest->pTest->init(pDevice->gfxDevice(), g_canvasSize*64);

	do { start = m_pVisitor->time(); } while (start == first);

	for (int i = 0; i < rounds; i++)
		pDeviceTest->pTest->run(pDevice->gfxDevice(), g_canvasSize*64);

	end = m_pVisitor->time();

	if (pDeviceTest->pTest->exit != nullptr)
		pDeviceTest->pTest->exit(pDevice->gfxDevice(), g_canvasSize*64);

	stallBegin = m_pVisitor->time();

	pDevice->endRender();

	stallEnd = m_pVisitor->time();


	pDeviceTest->render_time = (end - start)/1000000.0;
	pDeviceTest->stalling_time = (stallEnd - stallBegin)/1000000.0;
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

			se.pReferenceSuite->init(pReference->gfxDevice(), g_canvasSize*64, m_pVisitor);
			se.pTesteeSuite->init(pTestee->gfxDevice(), g_canvasSize*64, m_pVisitor);
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

	auto pTempDevice = SoftGfxDevice::create();
	bool bWorking = se.pReferenceSuite->init(pTempDevice, g_canvasSize*64, m_pVisitor);
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

//____ setup_theme() __________________________________________________________

bool GfxDeviceTester::setup_theme()
{
	// Load resources

	auto pPlateSurface = m_pVisitor->loadSurface("resources/grey_plate.bmp");
	assert(pPlateSurface);
	BlockSkin_p pPlateSkin = BlockSkin::create({ .frame = 3, .padding = 5, .surface = pPlateSurface } );
	m_pBackPlateSkin = pPlateSkin;
	
	
	auto pButtonSurface = m_pVisitor->loadSurface("resources/simple_button.bmp");
	assert(pButtonSurface);
	BlockSkin_p pSimpleButtonSkin = BlockSkin::create({ .axis = Axis::X, .frame = Border(3), .padding = Border(5), .states = { State::Hovered, {}, State::Pressed, {}, State::Disabled, {} }, .surface = pButtonSurface });
	m_pButtonSkin = pSimpleButtonSkin;

	return true;
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
	m_pWindow->rootPanel()->slot = pLayerStack;

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
	pSidebar->setSkin( m_pBackPlateSkin );
	pSidebar->setLayout(pUniformLayout);

	auto pCanvasPanel = PackPanel::create();
	pCanvasPanel->setAxis(Axis::Y);
	pCanvasPanel->setLayout(pUniformLayout);

	auto pViewNav = PackPanel::create();
	pViewNav->setAxis(Axis::X);
	pViewNav->setLayout(pUniformLayout);
	pViewNav->setSkin( m_pBackPlateSkin );

	auto pViewPanel = ScrollPanel::create();
	pViewPanel->setSkin( ColorSkin::create(Color8::SlateGrey) );
	g_pViewPanel = pViewPanel;

	pMidSection->slots << pCanvasPanel;
	pMidSection->slots << pSidebar;

	pMidSection->slots[0].setWeight(1.f);
	pMidSection->slots[1].setWeight(0.f);


	pCanvasPanel->slots << pViewNav;
	pCanvasPanel->slots << pViewPanel;

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
	pClipLabel->display.setStyle(g_pButtonLabelStyle);


	auto pNoClipButton = Button::create();
	pNoClipButton->setSkin( m_pButtonSkin );
	pNoClipButton->label.setText("One");
	pNoClipButton->label.setStyle(g_pButtonLabelStyle);
	pNoClipButton->label.setLayout(g_pButtonLabelMapper);

	auto pFewButton = Button::create();
	pFewButton->setSkin( m_pButtonSkin );
	pFewButton->label.setText("Few");
	pFewButton->label.setStyle(g_pButtonLabelStyle);
	pFewButton->label.setLayout(g_pButtonLabelMapper);

	auto pManyButton = Button::create();
	pManyButton->setSkin( m_pButtonSkin );
	pManyButton->label.setText("Many");
	pManyButton->label.setStyle(g_pButtonLabelStyle);
	pManyButton->label.setLayout(g_pButtonLabelMapper);

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


	auto pTesteeButton = Button::create();
	pTesteeButton->setSkin( m_pButtonSkin );
	pTesteeButton->label.setText("Testee");
	pTesteeButton->label.setStyle(g_pButtonLabelStyle);
	pTesteeButton->label.setLayout(g_pButtonLabelMapper);

	auto pRefButton = Button::create();
	pRefButton->setSkin( m_pButtonSkin );
	pRefButton->label.setText("Reference");
	pRefButton->label.setStyle(g_pButtonLabelStyle);
	pRefButton->label.setLayout(g_pButtonLabelMapper);

	auto pBothButton = Button::create();
	pBothButton->setSkin( m_pButtonSkin );
//	pBothButton->text = { "Both", g_pButtonLabelStyle, g_pButtonLabelMapper }
	pBothButton->label.setText("Both");
	pBothButton->label.setStyle(g_pButtonLabelStyle);
	pBothButton->label.setLayout(g_pButtonLabelMapper);

	auto pDiffButton = Button::create();
	pDiffButton->setSkin( m_pButtonSkin );
	pDiffButton->label.setText("Diff");
	pDiffButton->label.setStyle(g_pButtonLabelStyle);
	pDiffButton->label.setLayout(g_pButtonLabelMapper);

	auto pTimeButton = Button::create();
	pTimeButton->setSkin( m_pButtonSkin );
	pTimeButton->label.setText("Time");
	pTimeButton->label.setStyle(g_pButtonLabelStyle);
	pTimeButton->label.setLayout(g_pButtonLabelMapper);

	pDispModeSection->slots << pTesteeButton;
	pDispModeSection->slots << pRefButton;
	pDispModeSection->slots << pBothButton;
	pDispModeSection->slots << pDiffButton;
	pDispModeSection->slots << pTimeButton;

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

	// Setup display zoom section

	pDispModeSection->setAxis(Axis::X);
	pDispModeSection->setLayout(pUniformLayout);

	auto pX1Button = Button::create();
	pX1Button->setSkin( m_pButtonSkin );
	pX1Button->label.setText(" X1 ");
	pX1Button->label.setStyle(g_pButtonLabelStyle);
	pX1Button->label.setLayout(g_pButtonLabelMapper);

	auto pX2Button = Button::create();
	pX2Button->setSkin( m_pButtonSkin );
	pX2Button->label.setText(" X2 ");
	pX2Button->label.setStyle(g_pButtonLabelStyle);
	pX2Button->label.setLayout(g_pButtonLabelMapper);

	auto pX4Button = Button::create();
	pX4Button->setSkin( m_pButtonSkin );
	pX4Button->label.setText(" X4 ");
	pX4Button->label.setStyle(g_pButtonLabelStyle);
	pX4Button->label.setLayout(g_pButtonLabelMapper);

	auto pX8Button = Button::create();
	pX8Button->setSkin( m_pButtonSkin );
	pX8Button->label.setText(" X8 ");
	pX8Button->label.setStyle(g_pButtonLabelStyle);
	pX8Button->label.setLayout(g_pButtonLabelMapper);

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

	auto pTestList = PackList::create();
	pTestList->setSelectMode(SelectMode::FlipOnSelect);

	Base::msgRouter()->addRoute(pTestList, MsgType::ItemsSelect, [&](Msg* _pMsg) {

		auto pMsg = static_cast<ItemsSelectMsg*>(_pMsg);
		auto p = pMsg->items();
		for (int x = 0; x < pMsg->nbItems(); x++)
			g_tests[p[x].id].bActive = true;


		g_pReferenceDevice->setNeedsRedraw();
		g_pTesteeDevice->setNeedsRedraw();
	});

	Base::msgRouter()->addRoute(pTestList, MsgType::ItemsUnselect, [&](Msg* _pMsg) {

		auto pMsg = static_cast<ItemsUnselectMsg*>(_pMsg);
		auto p = pMsg->items();
		for (int x = 0; x < pMsg->nbItems(); x++)
			g_tests[p[x].id].bActive = false;

		g_pReferenceDevice->setNeedsRedraw();
		g_pTesteeDevice->setNeedsRedraw();
	});


	auto pSkin = BoxSkin::create(1, Color::White, Color::Black, 8);

	auto pEntrySkin = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Black, .outlineThickness = 1, .padding = 8,
			.states = {
				{State::Hovered, {.color = Color::AntiqueWhite } },
				{State::SelectedHovered, {.color = Color::AntiqueWhite } },
				{State::Selected, {.color = Color::Aquamarine }}
			}
	});

	pTestList->setSkin( pSkin );
	pTestList->setEntrySkin(pEntrySkin);

	int id = 0;
	for (TestEntry& test : g_tests)
	{
		auto pEntry = TextDisplay::create();
		pEntry->setId(id++);
		pEntry->display.setText(test.name);
		pTestList->slots.pushBack(pEntry);
	}

	auto pTestScrollPanel = ScrollPanel::create();
	pTestScrollPanel->setSizeConstraints( SizeConstraint::Equal, SizeConstraint::None );
	pTestScrollPanel->slot = pTestList;

	pTestScrollPanel->scrollbarY.setBackground(ColorSkin::create(Color::Green));
	pTestScrollPanel->scrollbarY.setBar(m_pButtonSkin);

	pSidebar->slots << pTestScrollPanel;


	// Setup performance display

	{
		auto pMapper = BasicTextLayout::create({ .placement = Placement::East });
		g_pPerformanceValueMapper = pMapper;

		g_pPerformanceEntryLayout = PackLayout::create( { .expandFactor = PackLayout::Factor::One } );


		auto pBase = PackPanel::create();
		pBase->setAxis(Axis::Y);

		auto pList = PackList::create();
		pList->setSkin( ColorSkin::create(Color::White) );

		auto pOddEntrySkin = BoxSkin::create(0, Color::White, Color::White);
		auto pEvenEntrySkin = BoxSkin::create(0, Color::PaleGreen, Color::PaleGreen);

		pList->setEntrySkin(pOddEntrySkin, pEvenEntrySkin);


		// Create the bottom section

		auto pBottom = PackPanel::create();
		pBottom->setSkin( m_pBackPlateSkin );
		pBottom->setAxis(Axis::X);
		pBottom->setLayout(pUniformLayout);

		auto pRefresh = Button::create();
		pRefresh->setSkin( m_pButtonSkin );
		pRefresh->label.setText("REFRESH");
		pRefresh->label.setStyle(g_pButtonLabelStyle);
		pRefresh->label.setLayout(g_pButtonLabelMapper);

		Base::msgRouter()->addRoute(pRefresh, MsgType::Select, [this](Msg* pMsg) {
			g_bRefreshPerformance = true;
		});


		auto pFiller = Filler::create();

		pBottom->slots << pFiller;
		pBottom->slots << pRefresh;

		//

		pBase->slots << pList;
		pBase->slots << pBottom;

		g_pPerformanceDisplay = pBase;
		g_pPerformanceList = pList;
	}

	return true;
}

//____ teardown_chrome() ______________________________________________________

void GfxDeviceTester::teardown_chrome()
{
}
