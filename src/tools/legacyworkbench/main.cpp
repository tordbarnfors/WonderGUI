#include <cstdlib>

//#include <dwmapi.h>
#undef min
#undef max


#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wdocumentation"
#		include <SDL2/SDL.h>
#		include <SDL2_image/SDL_image.h>
#       pragma clang diagnostic pop
#	else
#		include <SDL2/SDL.h>
#		include <SDL2/SDL_image.h>
#	endif
#endif

#include <wglegacygui.h>

#include "sdl_wglib.h"

#include <wg2_knob.h>
#include <iostream>

/*
#include <wg_streamsurface.h>
#include <wg_streamsurfacefactory.h>
#include <wg_streamplayer.h>
*/

#include <wg_gfxdevice_gen2.h>

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>
#include <wg_boxskin.h>
#include <wg_bitmapfont.h>
#include <wg_freetypefont.h>
#include <wg_textstyle.h>
#include <wg_blockskin.h>
#include <wg_tileskin.h>

#include <wg2_volumemeter.h>
#include <wg2_simplevolumemeter.h>
#include <wg2_chart.h>
#include <wg2_scrollchart.h>
#include <wg2_canvascapsule.h>

#include <wg2_popupopener.h>
#include <wg2_popuplayer.h>
#include <wg2_blockset.h>
#include <wg2_shadowlayer.h>
#include <wg2_pianokeyboard.h>
#include <wg2_tooltiplayer.h>
#include <wg2_mouseblocker.h>

#include <wg2_chart2.h>


void manuBlendTest();
void shadowLayerTest( WgRootPanel * pRoot );
void scrollPanelTest( WgRootPanel * pRoot );
void baselineTest( WgRootPanel * pRoot );
void packPanelStressTest( WgRootPanel * pRoot );
void cursorInViewTest( WgRootPanel * pRoot );
void flexHookGrowthTest( WgRootPanel * pRoot );
void packPanelPaddingTest( WgRootPanel * pRoot );
void packPanelTextWrapTest( WgRootPanel * pRoot );
bool pianoKeyboardTest(WgRootPanel * pRoot);
bool rangeSliderTest(WgRootPanel* pRoot);
bool canvasCapsuleTest(WgRootPanel* pRoot);
bool tooltipLayerTest(WgRootPanel* pRoot);
bool flowPanelTest(WgRootPanel* pRoot);
bool fullStateSupportTest(WgRootPanel* pRoot);
bool gfxStreamingTest(WgRootPanel* pRoot);
bool blendFixedColorTest(WgRootPanel* pRoot);
bool multiSliderClickThroughTest(WgRootPanel* pRoot);
bool scrollChartTest(WgRootPanel* pRoot);
bool flipCanvasTest(WgRootPanel* pRoot);
bool mouseOverMovingObjectsTest(WgRootPanel* pRoot);





//#define USE_OPEN_GL
/*
 wg::GfxStreamPlug_p         g_pGfxStreamPlug;
wg::StreamPlayer_p       g_pStreamPlayer;
wg::StreamGfxDevice_p       g_pStreamGfxDevice;
wg::StreamSurfaceFactory_p  g_pStreamSurfaceFactory;
 */
wg::SoftSurfaceFactory_p    g_pSoftSurfaceFactory;
wg::SurfaceFactory_p	    g_pSurfaceFactory;
wg::EdgemapFactory_p	    g_pEdgemapFactory;
wg::GfxDevice_p		        g_pGfxDevice;
wg::Surface_p               g_pCanvas;
//wg::SurfaceFactory_p g_pModernSurfaceFactory = nullptr;


extern std::ostream cout;

bool			eventLoop( WgEventHandler * pHandler );
WgRootPanel * 		setupGUI( wg::GfxDevice * pDevice );

void cbInitDrag( const WgEvent::Event* _pEvent, WgWidget * pWidget );
void cbDragWidget( const WgEvent::Event* _pEvent, WgWidget * pWidget );

void cbOpenModal( const WgEvent::Event* _pEvent, WgWidget * pWidget );
void cbCloseModal( const WgEvent::Event* _pEvent, WgWidget * pWidget );

void addResizablePanel( WgFlexPanel * pParent, WgWidget * pChild, WgEventHandler * pEventHandler );
void cbResize( const WgEvent::Event* _pEvent, void * _pFlexHook );


WgFlexPanel * createPresetSelector();


bool		g_bLeftPressed = false;
bool		g_bRightPressed = false;

WgModalLayer * g_pModal = 0;

WgWidget * pWidgetToMove = 0;

WgVolumeMeter * m_pVolMeter = 0;
WgSimpleVolumeMeter * m_pSimpleVolMeter1 = 0;

float	leftPeak = 1.f, rightPeak = 0.5f, leftHold = 0.5f, rightHold = 0.5f;

bool	leftUp = true, rightUp = false;

WgScrollChart * m_pScrollChart = nullptr;
int				m_hWave1 = 0;

WgFlexHook * g_pSpriteHook = nullptr;

WgKnob *		g_pKnob = nullptr;

WgWidget *		m_pMovingObject1 = nullptr;
WgWidget *		m_pMovingObject2 = nullptr;
int				m_moveSpeed1 = 1;
int				m_moveSpeed2 = 1;


//volatile int	m_bGotVsync = 0;

/*
static int TestThread(void *ptr)
{
	while (true)
	{
		m_bGotVsync = 1;
		DwmFlush();
	}

	return 0;
}
*/

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 10, posY = 10, width = 700, height = 500;

	int flags = 0;



#ifdef USE_OPEN_GL
	SDL_Window * pWin = SDL_CreateWindow("WonderGUI Testbench (OpenGL)", posX, posY, width, height, SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	SDL_GLContext context = SDL_GL_CreateContext(pWin);

	SDL_GL_SetSwapInterval(1);

//	SDL_Renderer * pRenderer = SDL_CreateRenderer(pWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

#ifdef WIN32
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif


	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();


#else
	SDL_Window * pWin = SDL_CreateWindow("WonderGUI Testbench (Software)", posX, posY, width, height, 0);
#endif


	int res = IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );


//	SDL_Thread *thread;
//	int         threadReturnValue;
//	thread = SDL_CreateThread(TestThread, "TestThread", (void *)NULL);


	// Init WonderGUI

	WgBase::Init( nullptr );
	sdl_wglib::MapKeys();
	
	WgBase::SetWidgetTracking(true);


//	WgBase::InitFreeType();

	// Setup gfxdevice and gui

#ifdef USE_OPEN_GL
	g_pGfxDevice = new WgGlGfxDevice( WgSize(width,height) );
	g_pSurfaceFactory = new WgGlSurfaceFactory();
	g_pEdgemapFactory = new WgGlEdgemapFactory();
	
//    g_pModernSurfaceFactory = wg::GlSurfaceFactory::create();
#else
	SDL_Surface * pScreen = SDL_GetWindowSurface(pWin);
	
	WgPixelType type = WgPixelType::Undefined;

	if (pScreen->format->BitsPerPixel == 32)
		type = WgPixelType::BGRA_8;
	else if (pScreen->format->BitsPerPixel == 24)
		type = WgPixelType::BGR_8;

	auto pCanvas = wg::SoftSurface::create( WGBP(Surface,
												 _.size = WgSize(width,height),
												 _.format = type,
												 _.canvas = true ),
											wg::Blob::create(pScreen->pixels, nullptr),
											pScreen->pitch);

	auto pBackend = wg::SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	pBackend->defineCanvas( wg::CanvasRef::Default, pCanvas );

	auto pGfxDevice = wg::GfxDeviceGen2::create(pBackend);

	

	g_pGfxDevice = pGfxDevice;
	g_pCanvas = pCanvas;
	
	g_pSoftSurfaceFactory = wg::SoftSurfaceFactory::create();
	g_pSurfaceFactory = g_pSoftSurfaceFactory;
	g_pEdgemapFactory = wg::SoftEdgemapFactory::create();

#endif
	
	WgBase::setDefaultGfxDevice(g_pGfxDevice);
	WgBase::setDefaultSurfaceFactory(g_pSurfaceFactory);
	WgBase::setDefaultEdgemapFactory(g_pEdgemapFactory);

	
	//	pGfxDevice->SetBilinearFiltering( true );



	// Load TTF-font

	char	ttfname[] = { "resources/DroidSans.ttf" };

	int size = sdl_wglib::FileSize( ttfname );
	char * pFontFile = (char*) sdl_wglib::LoadFile( ttfname );
	
	auto pFontBlob = wg::Blob::create(pFontFile, size, [pFontFile] { free(pFontFile); });

	auto pFont = wg::FreeTypeFont::create( { .blob = pFontBlob } );
	
	// Load bitmap font

//	auto pFont = sdl_wglib::LoadBitmapFont( "resources/anuvverbubbla_8x8.png", "resources/anuvverbubbla_8x8.fnt", g_pSurfaceFactory );

	// Load and setup cursor

	auto pCursorImg = sdl_wglib::LoadSurface("resources/cursors.png", g_pSurfaceFactory );

	WgGfxAnim * pCursorEOL = new WgGfxAnim();
	pCursorEOL->SetSize( WgSize(8,8) );
	pCursorEOL->AddFrames(pCursorImg, WgCoord(0,0), WgSize(2,1), 200 );
	pCursorEOL->SetPlayMode( WgAnimMode::Looping );

	WgGfxAnim * pCursorINS = new WgGfxAnim();
	pCursorINS->SetSize( WgSize(8,8) );
	pCursorINS->AddFrames( pCursorImg, WgCoord(0,8), WgSize(2,1), 200 );
	pCursorINS->SetPlayMode( WgAnimMode::Looping );

	WgCursor * pCursor = new WgCursor();
	pCursor->SetAnim(WgCursor::EOL, pCursorEOL);
	pCursor->SetAnim(WgCursor::INS, pCursorINS);
	pCursor->SetAnim(WgCursor::OVR, pCursorEOL);
	pCursor->SetBearing(WgCursor::EOL, WgCoord(0,-8));
	pCursor->SetBearing(WgCursor::INS, WgCoord(0,-8));
	pCursor->SetBearing(WgCursor::OVR, WgCoord(0,-8));

	WgBase::SetDefaultCursor( pCursor );

	// Set default textprop

	wg::TextStyle::Blueprint bp;
	bp.font = pFont;
	bp.color = wg::Color::White;
	bp.size = 10;
	
	auto pTextStyle = wg::TextStyle::create( bp );
		
	WgBase::setDefaultStyle( pTextStyle );


	WgRootPanel * pRoot = setupGUI( g_pGfxDevice );


//    packPanelPaddingTest( pRoot );
//    packPanelTextWrapTest( pRoot );
//    flexHookGrowthTest( pRoot );
//    packPanelStressTest( pRoot );
//    baselineTest( pRoot );
//    scrollPanelTest(pRoot);
//    shadowLayerTest(pRoot);
//	manuBlendTest();


//    cursorInViewTest( pRoot );
//	pianoKeyboardTest(pRoot);
//	rangeSliderTest(pRoot);
//    canvasCapsuleTest(pRoot);
//    tooltipLayerTest(pRoot);
//    flowPanelTest(pRoot);
//	fullStateSupportTest(pRoot);
//    gfxStreamingTest(pRoot);
//	blendFixedColorTest(pRoot);
//	multiSliderClickThroughTest(pRoot);
//	chartTest(pRoot);
	scrollChartTest(pRoot);
//	flipCanvasTest(pRoot);
//	mouseOverMovingObjectsTest(pRoot);

	
	WgBase::PrintWidgetTrees(std::cout);
	
/*
	// Setup debug overlays
	auto pOverlaySkin = wg::BoxSkin::create( { .color = WgColor(255,0,0,128), .outline = 1, .outlineColor = WgColor::Red,
		.states = { {wg::State::Default, {.color = WgColor::Transparent } }
		}
	});
	pRoot->SetUpdatedRectOverlay( pOverlaySkin,0);
*/
   // program main loop

	int counter = 0;

	while (eventLoop( pRoot->EventHandler() ))
	{

		if( m_pMovingObject1 )
		{
			auto pHook = static_cast<WgFlexHook*>(m_pMovingObject1->Hook());
			pHook->MovePoints({m_moveSpeed1,0});
			if( pHook->PointPos().x > 300 || pHook->PointPos().x <= 0 )
				m_moveSpeed1 *= -1;
		}

		if( m_pMovingObject2 )
		{
			auto pHook = static_cast<WgFlexHook*>(m_pMovingObject2->Hook());
			pHook->MovePoints({0,m_moveSpeed2});
			if( pHook->PointPos().y > 300 || pHook->PointPos().y <= 0 )
				m_moveSpeed2 *= -1;
		}

		// DRAWING STARTS HERE

//		pRoot->AddDirtyPatch( pRoot->Geo().Size() );

#ifndef USE_OPEN_GL
		SDL_LockSurface( pScreen );
#endif

		pRoot->Render();

#ifndef USE_OPEN_GL
		SDL_UnlockSurface( pScreen );

		
//        if( g_pStreamPlayer )
//            g_pStreamPlayer->playAll();
		
		// DRAWING ENDS HERE

		// finally, update the screen :)

		// GET DIRTY RECTS

		int nUpdatedRects;
		SDL_Rect	updatedRects[100];

		if( pRoot->NbUpdatedRects() <= 100 )
		{
			nUpdatedRects = pRoot->NbUpdatedRects();
			for( int i = 0 ; i < nUpdatedRects ; i++ )
			{
				const WgRect * pR = pRoot->FirstUpdatedRect() + i;

				updatedRects[i].x = pR->x;
				updatedRects[i].y = pR->y;
				updatedRects[i].w = pR->w;
				updatedRects[i].h = pR->h;
			}
		}
		else
		{
			nUpdatedRects = 1;

			const WgRect r = pRoot->PixelGeo();

			updatedRects[0].x = r.x;
			updatedRects[0].y = r.y;
			updatedRects[0].w = r.w;
			updatedRects[0].h = r.h;
		}

		SDL_UpdateWindowSurfaceRects(pWin, updatedRects, nUpdatedRects);
		
//		int ret = SDL_UpdateWindowSurface(pWin);
#endif
 
		// Pause for a while

		SDL_Delay(7);

//		while (!m_bGotVsync)
//		{
//			SDL_Delay(1);
//		}
//		m_bGotVsync = 0;

//		DwmFlush();

//		SDL_RenderPresent(pRenderer);

		counter++;

	} // end main loop


	// Exit WonderGUI

	delete pRoot;

	g_pSurfaceFactory = nullptr;
	g_pGfxDevice = nullptr;

	pTextStyle = nullptr;
	pFont = nullptr;
	pFontBlob = nullptr;
	
	WgBase::Exit();

	IMG_Quit();

	

	// all is well ;)
	printf("Exited cleanly\n");
	return 0;
}



//____ chartTest() __________________________________________________

bool chartTest(WgRootPanel* pRoot)
{
	float samples[101];
	
	for( int i = 0 ; i < 101 ; i++ )
		samples[i] = float(i % 30);
	
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);

	pBaseFlex->SetSkin(wg::ColorSkin::create(WgColor::Black));
	
	auto pChart = new WgChart2();
	pChart->SetSkin(wg::ColorSkin::create(WgColor::DarkGreen));
	
	int waveId = pChart->AddWave();
//	pChart->SetWaveStyle(waveId, WgColor::DarkBlue, WgColor::Red, 6.f, WgColor::LightBlue, 1.f, WgColor::LightBlue );
	pChart->SetWaveStyle(waveId, WgColor::DarkBlue, WgColor::LightBlue, 6.f, 1.f );
	pChart->SetWaveSamples(waveId, 0, 100, samples );

//	pBaseFlex->AddChild(pChart, WgRect(20,20,500,200) );

	//

	auto pChart2 = new WgChart2();
	pChart2->SetSkin(wg::ColorSkin::create(WgColor::DarkRed));
	
	int waveId2 = pChart2->AddWave();
//	pChart->SetWaveStyle(waveId, WgColor::DarkBlue, WgColor::Red, 6.f, WgColor::LightBlue, 1.f, WgColor::LightBlue );
	pChart2->SetWaveStyle(waveId2, WgColor::DarkBlue, WgColor::LightBlue, 0.f, 0.f );
	pChart2->SetWaveSamples(waveId2, 0, 100, samples );

	pBaseFlex->AddChild(pChart2, WgRect(20,240,500,200) );

	return true;
}

//____ scrollChartTest() __________________________________________________

bool scrollChartTest(WgRootPanel* pRoot)
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);

	pBaseFlex->SetSkin(wg::ColorSkin::create(WgColor::Blue));

	auto pChart = new WgScrollChart();
	pChart->SetSurfaceFactory(WgBase::defaultSurfaceFactory());
	pChart->SetSkin(wg::ColorSkin::create(WgColor::DarkGreen));

	pChart->SetValueRange(100, -100);
	pChart->SetPixelType(WgPixelType::BGRA_8);
	pChart->SetChartColor(WgColor::Transparent);
	/*
	 pChart->SetStaticMode(true);
	 pChart->SetStaticModeFadeOutLength(16);
	 pChart->SetStaticModeGradient( wg::Gradient( wg::Color::Black, wg::Color::White, wg::Color::White, wg::Color::Black ) );
	 */
	pChart->SetGridToForeground(true);

	auto hWave = pChart->StartSimpleWave( 50, 0, 1, WgColor::Red, 1, WgColor::Red, WgColor::Green );


	WgScrollChart::GridLine	grid[3];

	grid[0].pos = 0.0f;
	grid[0].thickness = 3;
	grid[0].color = WgColor::Red;
	grid[0].label = "0.0";

	grid[1].pos = 0.5f;
	grid[1].thickness = 3;
	grid[1].color = WgColor::Red;
	grid[1].label = "0.5";


	grid[2].pos = 1.0f;
	grid[2].thickness = 3;
	grid[2].color = WgColor::Red;
	grid[2].label = "1.0";

	pBaseFlex->AddChild(pChart, WgRect(20, 20, 500, 200));

	m_pScrollChart = pChart;		// So we get updates.
	m_hWave1 = hWave;

	pChart->Start( 8000 );


	pChart->SetSampleGridLines(3, grid);

	pChart->SetSampleLabelStyle(WgOrigo::South, {0,0}, nullptr, nullptr);

	auto pSkin = wg::ColorSkin::create( { .states = {{ wg::State::Default, wg::Color8::White}, { wg::State::Hovered, wg::Color8::Pink }}} );

	auto pRestartButton = new WgButton();
	pRestartButton->SetSkin(pSkin);

	pBaseFlex->AddChild(pRestartButton, WgRect(20,250,50,30));

	pRoot->EventHandler()->AddCallback( [](const WgEvent::Event * pEvent, WgWidget * pWidget )
	{
		if( pEvent->Type() == WgEventType::WG_EVENT_BUTTON_PRESS )
		{
			m_pScrollChart->ClearWave(m_hWave1, 10.0f, 1.0f );
		}

	}, pRestartButton);

	return true;
}

//____ mouseOverMovingObjectsTest() ___________________________________________

bool mouseOverMovingObjectsTest(WgRootPanel* pRoot)
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->SetSkin( wg::ColorSkin::create(WgColor::DarkBlue));

	
	auto pSkin = wg::ColorSkin::create( { .states = {{ wg::State::Default, wg::Color8::White}, { wg::State::Hovered, wg::Color8::Pink }}} );
	
	
	auto pFiller1 = new WgButton();
	pFiller1->SetSkin( pSkin );
	m_pMovingObject1 = pFiller1;
	
	pBaseFlex->AddChild( pFiller1, {10,100,50,50} );

	auto pFiller2 = new WgButton();
	pFiller2->SetSkin( pSkin );
	m_pMovingObject2 = pFiller2;

	pBaseFlex->AddChild( pFiller2, {200,10, 70,70} );

	
	return true;
}

//____ flipCanvasTest() __________________________________________________

bool flipCanvasTest(WgRootPanel* pRoot)
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->SetSkin( wg::ColorSkin::create(WgColor::Yellow));
	
	auto pPanelSkin = wg::ColorSkin::create( { .color = WgColor::Red, .padding = 10 } );
	
	
	
	auto pPackPanel = new WgPackPanel();
	pPackPanel->SetOrientation( wg::Axis::Y);
	pPackPanel->SetSkin(pPanelSkin);
	
	auto pSizeConstrainer = new WgSizeCapsule();
	pSizeConstrainer->SetChild(pPackPanel);
	pSizeConstrainer->SetPreferredSize( {300,-1} );

	auto pCanvasCapsule = new WgCanvasCapsule();
	pCanvasCapsule->SetFlip(wg::GfxFlip::Rot90);
	pCanvasCapsule->SetChild(pSizeConstrainer);
	pCanvasCapsule->SetSkin(wg::ColorSkin::create(wg::Color::Cyan));
	
	pBaseFlex->AddChild( pCanvasCapsule, {10,10} );

	auto pTextSkin = wg::ColorSkin::create( { .color = WgColor::Green, .padding = 5 } );
	
	auto pText = new WgTextDisplay();
	pText->SetText("\nTEXT  1.");
	pText->SetSkin( pTextSkin );
	//    pText->SetTextWrap(true);
	// pText->SetEditMode(WgTextEditMode::Editable);
	pPackPanel->AddChild(pText);

	auto pText2 = new WgTextDisplay();
	pText2->SetText("\nTEXT 2.");
	pText2->SetSkin( pTextSkin );
	//    pText2->SetTextWrap(true);
	pText2->SetEditMode(WgTextEditMode::Editable);

	auto pInterferer = new WgPackPanel();
	pInterferer->SetSizeBroker(new WgUniformSizeBroker());
	pInterferer->AddChild(pText2);
	
	auto pBalast = new WgFiller();
	pBalast->SetPreferredPointSize({20,20});
	pInterferer->AddChild(pBalast)->SetWeight(0.f);
	
	pPackPanel->AddChild(pInterferer);
	return true;
}


//____ multiSliderClickThroughTest() __________________________________________________

bool multiSliderClickThroughTest(WgRootPanel* pRoot)
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);

	pBaseFlex->SetSkin(wg::ColorSkin::create(WgColor::Black));
	
	auto pSliders = new WgMultiSlider();
	pSliders->SetSkin( wg::ColorSkin::create(WgColor::HotPink) );

	pSliders->AddSlider( 1, WgDirection::Right, [](WgMultiSlider::SetGeoVisitor& visitor){ return WgRectF(0.05f, 0.05f, 0.9f, 0.05f); }, 0.f, 0.f, 1.f, 0,
						nullptr, wg::ColorSkin::create(WgColor::Yellow),wg::ColorSkin::create( { .color = WgColor::Green, .padding = 20 } ) );

	pSliders->AddSlider2D( 1, WgOrigo::NorthWest, [](WgMultiSlider::SetGeoVisitor& visitor){ return WgRectF(0.05f, 0.20f, 0.9f, 0.70f); }, 0.f, 0.f,
						  0.f, 1.f, 0, 0.f, 1.f, 0,
						nullptr, wg::ColorSkin::create(WgColor::Yellow),wg::ColorSkin::create( { .color = WgColor::Green, .padding = 20 } ) );


	pSliders->SetOnlyHandlesPressable(true);
	
	pBaseFlex->AddChild(pSliders, WgRect(20,20,300,300) );
	return true;
}

//____ blendFixedColorTest() __________________________________________________

bool blendFixedColorTest(WgRootPanel* pRoot)
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);

	pBaseFlex->SetSkin(wg::ColorSkin::create(WgColor::Black));
	
	auto pText1 = new WgTextDisplay();
	pText1->SetText("TEXT 1 WITH FIXED BACKGROUND COLOR.");
	pText1->SetSkin( wg::ColorSkin::create(WgColor::HotPink) );
	pText1->SetTextWrap(true);
	pText1->SetFixedBlendColor(wg::HiColor(0,2048,0,4096));

	pBaseFlex->AddChild(pText1, WgCoord(20,20) );
	return true;
}


//____ manuBlendTest() _________________________________________________________

void manuBlendTest()
{
/*
	WgSurfaceSoft * pCanvas = new WgSurfaceSoft({ 2800,2800 }, WgPixelType::BGR_8);

	WgSurfaceFactory * pFactory = new WgSurfaceFactorySoft();

	WgSurface * pBackImg = sdl_wglib::LoadSurface("resources/BlendModes-layer1.png", *pFactory);
	WgSurface * pFrontImg = sdl_wglib::LoadSurface("resources/BlendModes-layer2.png", *pFactory);


	WgGfxDevice * pDevice = new WgGfxDeviceSoft(pCanvas);


	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->SetBlendMode(WgBlendMode::Blend);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_blend.png");

	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->SetBlendMode(WgBlendMode::Add);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_add.png");

	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->SetBlendMode(WgBlendMode::Invert);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_invert.png");

	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->SetBlendMode(WgBlendMode::Multiply);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_multiply.png");

	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_opaque.png");

	pDevice->SetBlendMode(WgBlendMode::Replace);
	pDevice->BeginRender();
	pDevice->Blit(pBackImg);
	pDevice->RealDevice()->setBlendMode( wg::BlendMode::Subtract);
	pDevice->Blit(pFrontImg);
	pDevice->EndRender();
	sdl_wglib::SavePNG(pCanvas, "blendmodes_subtract.png");



	delete pBackImg;
	delete pFrontImg;
	delete pDevice;
*/
}

//____ packPanelStressTest() _________________________________________________________

void packPanelStressTest( WgRootPanel * pRoot )
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	

	auto pVPackSkin = wg::BoxSkin::create( { .color = WgColor::Blue, .outlineColor = WgColor::Red, .outlineThickness = 10, .padding = 11 } );
	
	auto pVPack = new WgPackPanel();
	pVPack->SetOrientation(wg::Axis::Y);
	pVPack->SetSkin( pVPackSkin );

	
	auto pTextSkin = wg::ColorSkin::create(WgColor::Green);
//    pTextSkin->SetContentPadding( {2,10,10,12} );
	
	auto pText1 = new WgTextDisplay();
	pText1->SetText("TEXT 1 WITH AUTOWRAPPING ENABLED SO THAT WE GET MULTIPLE LINES.");
	pText1->SetSkin( pTextSkin );
	pText1->SetTextWrap(true);

	auto pText2 = new WgTextDisplay();
	pText2->SetSkin( pTextSkin );
	pText2->SetTextWrap(true);

	pBaseFlex->AddChild(pVPack, {50,50,100,400});

	pVPack->AddChild(pText1)->SetPadding( {0,0,0,0} );
	pVPack->AddChild(pText2)->SetPadding( {10,10,10,10} );

//    pText2->SetText("TEXA");
	pText2->SetText("TEXT");

	
}

//____ packPanelPaddingTest() _________________________________________________________

void packPanelPaddingTest( WgRootPanel * pRoot )
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->SetSkin( wg::ColorSkin::create(WgColor::Yellow));

	auto pPanelSkin = wg::ColorSkin::create( { .color = WgColor::Red, .padding = 10 } );
	
	auto pPackPanel = new WgPackPanel();
	pPackPanel->SetSkin(pPanelSkin);
	pBaseFlex->AddChild( pPackPanel, {10,10} );
	
	auto pTextSkin = wg::ColorSkin::create( { .color = WgColor::Green, .padding = 5 } );

	auto pText = new WgTextDisplay();
	pText->SetText("TEXT  1.");
	pText->SetSkin( pTextSkin );
//    pText->SetTextWrap(true);
	pText->SetEditMode(WgTextEditMode::Editable);
	
	auto pText2 = new WgTextDisplay();
	pText2->SetText("TEXT 2.");
	pText2->SetSkin( pTextSkin );
//    pText2->SetTextWrap(true);
	pText2->SetEditMode(WgTextEditMode::Editable);

	pPackPanel->AddChild(pText);
	pPackPanel->AddChild(pText2);
}

//____ packPanelTextWrapTest() _________________________________________________________

void packPanelTextWrapTest( WgRootPanel * pRoot )
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->SetSkin( wg::ColorSkin::create(WgColor::Yellow));
	
	auto pPanelSkin = wg::ColorSkin::create( { .color = WgColor::Red, .padding = 10 } );
	
	auto pPackPanel = new WgPackPanel();
	pPackPanel->SetOrientation( wg::Axis::Y);
	pPackPanel->SetSkin(pPanelSkin);
	
	auto pSizeConstrainer = new WgSizeCapsule();
	pSizeConstrainer->SetChild(pPackPanel);
	pSizeConstrainer->SetPreferredSize( {300,-1} );
	
	pBaseFlex->AddChild( pSizeConstrainer, {10,10} );

	auto pTextSkin = wg::ColorSkin::create( { .color = WgColor::Green, .padding = 5 } );
	
	auto pText = new WgTextDisplay();
	pText->SetText("\nTEXT  1.");
	pText->SetSkin( pTextSkin );
	//    pText->SetTextWrap(true);
	// pText->SetEditMode(WgTextEditMode::Editable);
	pPackPanel->AddChild(pText);

	auto pText2 = new WgTextDisplay();
	pText2->SetText("\nTEXT 2.");
	pText2->SetSkin( pTextSkin );
	//    pText2->SetTextWrap(true);
	pText2->SetEditMode(WgTextEditMode::Editable);

	auto pInterferer = new WgPackPanel();
	pInterferer->SetSizeBroker(new WgUniformSizeBroker());
	pInterferer->AddChild(pText2);
	
	auto pBalast = new WgFiller();
	pBalast->SetPreferredPointSize({20,20});
	pInterferer->AddChild(pBalast)->SetWeight(0.f);
	
	
	
	
	pPackPanel->AddChild(pInterferer);
}


//____ flexHookGrowthTest() _________________________________________________________

void flexHookGrowthTest( WgRootPanel * pRoot )
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->SetSkin( wg::ColorSkin::create(WgColor::Yellow));
	
	auto pTextSkin = wg::ColorSkin::create( { .color = WgColor::Green, .padding = 10 } );
	
	auto pText = new WgTextDisplay();
	pText->SetText("TEXT 1 WITH\nAUTOWRAPPING ENABLED SO THAT WE GET MULTIPLE LINES.");
	pText->SetSkin( pTextSkin );
	pText->SetTextWrap(true);
	pText->SetEditMode(WgTextEditMode::Editable);

	
	auto pkSizeCapsule = new WgSizeCapsule();
	pkSizeCapsule->SetPreferredSize({ 200, 0 });
	pkSizeCapsule->SetChild(pText);
	
	pBaseFlex->AddChild(pkSizeCapsule, {-20,-20}, WgOrigo::SouthWest );
	
}

//____ cursorInViewTest() _________________________________________________________

void cursorInViewTest( WgRootPanel * pRoot )
{
	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	
	
	auto pScrollPanel = new WgScrollPanel();
	pScrollPanel->SetContentSizePolicy(wg::SizeConstraint::Equal, wg::SizeConstraint::Equal);
	
	auto pTextSkin = wg::ColorSkin::create( { .color = WgColor::Green, .padding = {10,10,10,30} } );
	
	auto pText = new WgTextDisplay();
	pText->SetText("TEXT 1 WITH AUTOWRAPPING ENABLED SO THAT WE GET MULTIPLE LINES.");
	pText->SetSkin( pTextSkin );
	pText->SetTextWrap(true);
	pText->SetEditMode(WgTextEditMode::Editable);
	
	auto pPackX = new WgPackPanel();
	pPackX->SetOrientation(wg::Axis::X);
	pPackX->AddChild(pText);
	pPackX->SetSizeBroker(new WgUniformSizeBroker() );
	
	auto pPackSkin = wg::ColorSkin::create( { .color = WgColor::Brown, .padding = 10 } );

	auto pFiller = new WgFiller();
	pFiller->SetPreferredPointSize({10,10});
	pPackX->AddChild(pFiller);
	pPackX->SetSkin(pPackSkin);

	auto pPackY = new WgPackPanel();
	pPackY->SetOrientation(wg::Axis::X);
	pPackY->AddChild(pPackX);
	pPackY->SetSizeBroker(new WgUniformSizeBroker() );
	pPackY->SetSkin(pPackSkin);

	pScrollPanel->SetContent(pPackY);
	pBaseFlex->AddChild(pScrollPanel, {50,50,300,60});
	
}

//____ baselineTest() _________________________________________________________

void baselineTest( WgRootPanel * pRoot )
{
	
	auto pRootLane = new WgPackPanel();
	pRootLane->SetBaselineMode(true);
	
	auto pSplitter = new WgFiller();
	pSplitter->SetColors(WgColorset::Create(WgColor::Red));
	pSplitter->SetPreferredPointSize({80,200});

	auto pJoiner = new WgFiller();
	pJoiner->SetColors(WgColorset::Create(WgColor::Blue));
	pJoiner->SetPreferredPointSize({80,200});

	auto pLanes = new WgPackPanel();
	pLanes->SetOrientation(wg::Axis::Y);

	auto pUpperLane = new WgPackPanel();
	pUpperLane->SetBaselineMode(true);

	auto pLowerLane = new WgPackPanel();
	pLowerLane->SetBaselineMode(true);

	pLanes->AddChild(pUpperLane);
	pLanes->AddChild(pLowerLane);
	
	auto pUpperContent = new WgFiller();
	pUpperContent->SetColors(WgColorset::Create(WgColor::Yellow));
	pUpperContent->SetPreferredPointSize({80,200});

	auto pLowerContent = new WgFiller();
	pLowerContent->SetColors(WgColorset::Create(WgColor::Orange));
	pLowerContent->SetPreferredPointSize({90,200});

	pLanes->AddChild(pUpperContent);
	pLanes->AddChild(pLowerContent);

	auto pLastFiller = new WgFiller();
	pLastFiller->SetColors(WgColorset::Create(WgColor::Orange));
	pLastFiller->SetPreferredPointSize({90,200});

	
	pRootLane->AddChild(pSplitter)->SetBaseline(0.5f);


	pRootLane->AddChild(pLanes)->SetBaseline(0.5f);
	
	pRootLane->AddChild(pJoiner)->SetBaseline(0.5f);

	pRootLane->InsertChild(pLastFiller, pLanes)->SetBaseline(0.5f);

	auto pBaseFlex = new WgFlexPanel();
	pRoot->SetChild(pBaseFlex);
	pBaseFlex->AddChild(pRootLane);
	
	pUpperContent->SetPreferredPointSize({80,250});
}


//____ scrollPanelTest() _________________________________________________________

void scrollPanelTest( WgRootPanel * pRoot )
{
	wg::BoxSkin_p    pBgSkin = wg::BoxSkin::create( { .color = WgColor::Green, .outlineColor = WgColor::Red, .outlineThickness = 2, .padding = 2 } );
	
	wg::BoxSkin_p    pButtonSkin = wg::BoxSkin::create({ .color = WgColor::Red, .outlineColor = WgColor::Black, .outlineThickness = 2 } );
//    pButtonSkin->SetContentPadding(6);

	wg::BoxSkin_p    pBarSkin = wg::BoxSkin::create({ .color = WgColor::Grey, .outlineColor = WgColor::DarkGrey, .outlineThickness = 2, .padding = 6 });

	auto pScrollPanel = new WgScrollPanel();
	
	pScrollPanel->SetBgColor(WgColor::YellowGreen);
	
	auto pVSlider = new WgVSlider();
	pVSlider->SetSkins(pBgSkin, pBarSkin, pButtonSkin, pButtonSkin );
	pScrollPanel->SetVSlider(pVSlider);

	auto pHSlider = new WgHSlider();
	pHSlider->SetSkins(pBgSkin, pBarSkin, pButtonSkin, pButtonSkin );
	pScrollPanel->SetHSlider(pHSlider);

	auto pContent = new WgFlowPanel();
	
	for( int i = 0 ; i < 100 ; i++ )
	{
		auto pFiller = new WgFiller();
		pFiller->SetPreferredPointSize({50,50});
		
		pFiller->SetColors( WgColorset::Create( WgColor( 255*(i%2),64*(i%4),32*(i%8) )));
		
		pContent->AddChild(pFiller);
	}
	
/*
	auto pText = new WgTextDisplay();
	pText->SetTextProperties( WgBase::GetDefaultTextprop());
	for( int i = 0 ; i < 50 ; i++ )
		pText->AddText( "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." );
  */
	pScrollPanel->SetContent(pContent);

	pScrollPanel->SetShowOverlaySlidersOnlyOnHover(true);
	
	auto pBaseFlex = new WgFlexPanel();
	pBaseFlex->AddChild(pScrollPanel, WgRect(20,20,200,100));

	pRoot->SetChild(pBaseFlex);
}


//____ shadowLayerTest() _________________________________________________________

void shadowLayerTest( WgRootPanel * pRoot )
{
	auto pImgSurface = sdl_wglib::LoadSurface("resources/shadow.png", g_pSurfaceFactory );
	
	auto pShadowSkin = wg::BlockSkin::create( { .firstBlock = {0,128,128,0}, .padding = {0,128,128,0}, .surface = pImgSurface } );
	
	
	auto pShadowLayer = new WgShadowLayer();
	auto pFrontLayer = new WgFlexPanel();
	auto pBaseLayer = new WgFlexPanel();
	
	pRoot->SetChild( pShadowLayer );
	
	pShadowLayer->SetBase( pBaseLayer );
	pShadowLayer->SetFront( pFrontLayer );
	
	
	auto pFiller1 = new WgFiller();
	auto pFiller2 = new WgFiller();
	auto pFiller3 = new WgFiller();
	
	pFiller1->SetColors( WgColorset::Create(WgColor::DarkKhaki));
	pFrontLayer->AddChild(pFiller1, {0,0,50,50} );
	
	pFiller2->SetColors( WgColorset::Create(WgColor::DarkSlateBlue));
	pFrontLayer->AddChild(pFiller2, {50,50,50,50} );
	
	pFiller3->SetColors( WgColorset::Create(WgColor::DarkSeaGreen));
	pFrontLayer->AddChild(pFiller3, {100,100,50,50} );
	
	auto pBackground = new WgFiller();
	pBackground->SetColors(WgColorset::Create(WgColor::LightSalmon));
	pBaseLayer->AddChild(pBackground, WgOrigo::NorthWest, WgOrigo::SouthEast);
	
	pShadowLayer->AddShadow( pFiller1, pShadowSkin );
	pShadowLayer->AddShadow( pFiller2, pShadowSkin );
	pShadowLayer->AddShadow( pFiller3, pShadowSkin );
	
	WgEventHandler * pEventHandler = pRoot->EventHandler();

	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pFiller1, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
							   {
								   WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();
								   
								   static_cast<WgFlexHook*>(pWin->Hook())->MovePoints(drag);
							   }, pFiller1);

	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pFiller2, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
							   {
								   WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();
								   
								   static_cast<WgFlexHook*>(pWin->Hook())->MovePoints(drag);
							   }, pFiller2);

	pEventHandler->AddCallback(WgEventFilter::MouseButtonDrag(pFiller3, 1), [](const WgEvent::Event * pEvent, WgWidget *pWin)
							   {
								   WgCoord drag = static_cast<const WgEvent::MouseButtonDrag*>(pEvent)->DraggedNowPoints();
								   
								   static_cast<WgFlexHook*>(pWin->Hook())->MovePoints(drag);
							   }, pFiller3);

}


//____ pianoKeyboardTest() ____________________________________________________

bool pianoKeyboardTest(WgRootPanel * pRoot)
{
	auto pBaseLayer = new WgFlexPanel();
	pBaseLayer->SetSkin(wg::ColorSkin::create(wg::Color::PapayaWhip));

	auto pSkin = wg::ColorSkin::create( { .color = wg::Color::Black, .padding = { 60,10,10,10 } });
	
	{

		wg::Surface_p pOddWhiteKeys		= sdl_wglib::LoadSurface("resources/whiteoddkeys.png", g_pSurfaceFactory);
		wg::Surface_p pEvenWhiteKeys	= sdl_wglib::LoadSurface("resources/whiteevenkeys.png", g_pSurfaceFactory);
		wg::Surface_p pBlackKeys		= sdl_wglib::LoadSurface("resources/blackkeys.png", g_pSurfaceFactory);

		auto pSimplePiano = new WgPianoKeyboard();

		pSimplePiano->SetSkin(pSkin);
		pSimplePiano->setLayout(7, std::bitset<7>("1101110"));
		pSimplePiano->setSurfaces(pOddWhiteKeys, pEvenWhiteKeys, pBlackKeys, { wg::State::Default, wg::State::Hovered, wg::State::Pressed });

		pBaseLayer->AddChild(pSimplePiano, { 20,20 });
	}

	{
		wg::Surface_p pOddWhiteKeys = sdl_wglib::LoadSurface("resources/NisOddWhite.png", g_pSurfaceFactory);
		wg::Surface_p pEvenWhiteKeys = sdl_wglib::LoadSurface("resources/NisEvenWhite.png", g_pSurfaceFactory);
		wg::Surface_p pBlackKeys = sdl_wglib::LoadSurface("resources/NisBlack.png", g_pSurfaceFactory);

		auto pNisPiano = new WgPianoKeyboard();
		pNisPiano->SetSkin(pSkin);
		pNisPiano->setLayout(26, std::bitset<7>("1110110"));
		pNisPiano->setSurfaces(pOddWhiteKeys, pEvenWhiteKeys, pBlackKeys, { wg::State::Default, wg::State::Pressed });

		pNisPiano->setGeoTweak(3, {16,0,0,0});
		pNisPiano->setGeoTweak(5, {16,0,0,0});
		pNisPiano->setGeoTweak(10, {16,0,0,0});
		pNisPiano->setGeoTweak(17, {16,0,0,0});

		pNisPiano->setGeoTweak(25, {-16,0,0,0});
		pNisPiano->setGeoTweak(29, {16,0,0,0});
		pNisPiano->setGeoTweak(32, {-16,0,0,0});
		pNisPiano->setGeoTweak(37,{-16,0,0,0});

		pBaseLayer->AddChild(pNisPiano, { 20,220 });

//		pNisPiano->setFlipKeyOnPress(true);
	}

	pRoot->SetChild(pBaseLayer);
	return true;
}


//____ rangeSliderTest() ____________________________________________________

bool rangeSliderTest(WgRootPanel* pRoot)
{
	auto pBaseLayer = new WgFlexPanel();
	pBaseLayer->SetSkin(wg::ColorSkin::create(wg::Color::PapayaWhip));

	auto pHandleSkin = wg::ColorSkin::create( {
		.color = wg::Color8(0x7F808080),
		.padding = 10,
		.states = {
			{ wg::State::Hovered, { .color = wg::Color8(0x7FA0A0A0)}},
			{ wg::State::Pressed, { .color = wg::Color8(0xFFF0F0F0)}}
		}
	});
		
	auto pSliderX = new WgRangeSlider();
	{
		auto pBgSkin = wg::FillMeterSkin::create( {
			.backColor = wg::Color::Black,
			.color = wg::Color::Green,
			.direction = wg::Direction::Right,
			.padding = wg::Border(),
			.spacing = wg::Border(0, 10, 0, 10)
			});

		pSliderX->SetAxis(wg::Axis::X);
		pSliderX->SetSkin(pBgSkin);
		pSliderX->SetBeginHandleSkin(pHandleSkin);
		pSliderX->SetEndHandleSkin(pHandleSkin);
		pSliderX->SetPreferredSlideLength(100);
	}

	auto pSliderY = new WgRangeSlider();
	{
		auto pBgSkin = wg::FillMeterSkin::create( {
			.backColor = wg::Color::Black,
			.color = wg::Color::Green,
			.direction = wg::Direction::Up,
			.padding = wg::Border(),
			.spacing = wg::Border(10, 0, 10, 0)
			});
		
		pSliderY->SetAxis(wg::Axis::Y);
		pSliderY->SetSkin(pBgSkin);
		pSliderY->SetBeginHandleSkin(pHandleSkin);
		pSliderY->SetEndHandleSkin(pHandleSkin);
		pSliderY->SetPreferredSlideLength(100);
	}

	pBaseLayer->AddChild(pSliderY, WgCoord(10, 10));
	pBaseLayer->AddChild(pSliderX, WgCoord(40, 10));


	pRoot->SetChild(pBaseLayer);
	return true;
}

//____ canvasCapsuleTest() _________________________________________________________

bool canvasCapsuleTest(WgRootPanel* pRoot)
{
	auto pBaseLayer = new WgFlexPanel();
	pBaseLayer->SetSkin(wg::ColorSkin::create(wg::Color::PapayaWhip));

	auto pCanvas = new WgCanvasCapsule();
	pCanvas->SetSurfaceFactory(g_pSurfaceFactory);

	pCanvas->SetCanvasFillColor(wg::Color(255,255,255,128));
	
//    auto pFiller = new WgFiller();
//    pFiller->SetColors( WgColorset::Create(wg::Color(255,255,255,0)) );
//    pCanvas->SetChild(pFiller);
	
	pBaseLayer->AddChild(pCanvas, WgRect(10, 10,100,100));
	
//    pCanvas->StartFade(WgColor(0,0,0,255), 5000 );

	
	pRoot->SetChild(pBaseLayer);
	return true;
}


//____ tooltipLayerTest() _____________________________________________________

bool tooltipLayerTest(WgRootPanel * pRoot)
{
	auto pTooltipLayer = new WgTooltipLayer();

	wg::Surface_p pTooltipBg = sdl_wglib::LoadSurface("resources/tooltip_under_bg.png", g_pSurfaceFactory);
	auto pSkin = wg::BlockSkin::create( { .frame = wg::Border( 10,4,3,4 ), .padding = { 10,4,4,4 }, .surface = pTooltipBg }  );
//        pSkin->setRigidPartX(5, 16, wg::YSections::Top | wg::YSections::Center | wg::YSections::Bottom);

/*
	pTooltipLayer->SetTooltipGenerator([pSkin](WgTooltipLayer::Placement& placement, const WgWidget* pHoveredWidget, const WgBorders& widgetMargins)
	{
		auto pTooltip = new WgTextDisplay();

		pTooltip->SetSkin(pSkin);


		placement.bTooltipAroundPointer = false;
		placement.direction = wg::Origo::South;
		placement.spacing = wg::BorderI(0);

		pTooltip->SetText(pHoveredWidget->GetTooltipString());
		return pTooltip;
	});
*/

	auto pTooltipSkin = wg::BoxSkin::create( { .color = wg::Color::White, .outlineColor = wg::Color::Black, .outlineThickness = 1, .padding = 2 } );

	auto pBaseLayer = new WgFlexPanel();
	pBaseLayer->SetSkin( wg::ColorSkin::create(wg::Color::PapayaWhip) );


	auto pWidget1 = new WgFiller();
	pWidget1->SetColors( WgColorset::Create(wg::Color::Blue) );
	pWidget1->SetTooltipString( wg::String("This is widget 1.") );
	pBaseLayer->AddChild(pWidget1, WgRect(10, 10, 100, 100));

	auto pWidget2 = new WgFiller();
	pWidget2->SetColors( WgColorset::Create(wg::Color::Green) );
	pWidget2->SetTooltipString( wg::String("This is widget 2.") );
	pBaseLayer->AddChild(pWidget2, WgRect(150, 20, 170, 100));

	pTooltipLayer->SetBase( pBaseLayer );
	pRoot->SetChild(pTooltipLayer);

/*
	auto pBlocker = new WgMouseBlocker();
	
	WgRect hole( 20,20,30,30 );
	
	pBlocker->SetHoles( 1, &hole );
	pBaseLayer->AddChild( pBlocker, WgRect( 0,0,300,300) );
*/


	return true;
}


//____ flowPanelTest() _________________________________________________________

bool flowPanelTest(WgRootPanel* pRoot)
{
	wg::ColorSkin_p pSkin[5];
	
	pSkin[0] = wg::ColorSkin::create(WgColor(255,0,0));
	pSkin[1] = wg::ColorSkin::create(WgColor(0,255,0));
	pSkin[2] = wg::ColorSkin::create(WgColor(0,0,255));
	pSkin[3] = wg::ColorSkin::create(WgColor(255,255,0));
	pSkin[4] = wg::ColorSkin::create(WgColor(255,0,255));

	int sizes[4] = {10,15,20,40};
	

	auto pBaseLayer = new WgFlexPanel();
	pBaseLayer->SetSkin(wg::ColorSkin::create(wg::Color::PapayaWhip));

	auto pFlow1 = new WgFlowPanel();
	pFlow1->SetSkin(wg::ColorSkin::create(wg::Color::Black));

	for( int i = 0 ; i < 10 ; i++ )
	{
		auto pFiller  = new WgFiller();
		
		pFiller->SetSkin( pSkin[i%5] );
		pFiller->SetPreferredPointSize({sizes[i%4],sizes[i%4]});
		pFlow1->AddChild(pFiller);
	}

	
	pBaseLayer->AddChild(pFlow1, WgRect(10, 10,100,100));

	auto pFlow2 = new WgFlowPanel();
	pFlow2->SetOrientation(wg::Axis::Y);
	pFlow2->SetSkin(wg::ColorSkin::create(wg::Color::Black));

	for( int i = 0 ; i < 10 ; i++ )
	{
		auto pFiller  = new WgFiller();
		
		pFiller->SetSkin( pSkin[i%5] );
		pFiller->SetPreferredPointSize({sizes[i%4],sizes[i%4]});
		pFlow2->AddChild(pFiller);
	}

	
	pBaseLayer->AddChild(pFlow2, WgRect(230, 10,100,100));


	
//    pCanvas->StartFade(WgColor(0,0,0,255), 5000 );

	
	pRoot->SetChild(pBaseLayer);
	return true;
}

//____ fullStateSupportTest() __________________________________________________

bool fullStateSupportTest(WgRootPanel* pRoot)
{

	auto pBaseLayer = new WgFlexPanel();
	pRoot->SetChild(pBaseLayer);
	
	auto pTextSkin = wg::ColorSkin::create( {
	   .padding = {10,10,10,30},
	   .states = {
		   {WgState::Default, { .color = wg::HiColor(0,4096,0) }},
		   {WgState::Hovered, { .color = wg::HiColor(4096,4096,0) }},
		   {WgState::Focused, { .color = wg::HiColor(4096,0,0) }},
		   {WgState::HoveredFocused, { .color = wg::HiColor(4096,0,0) }}
	   }
	});
	
	
	auto bp = WgBase::defaultStyle()->blueprint();
	bp.modifyState(wg::State::Focused, { .color = WgColor::Black } );
	auto pTextStyle = wg::TextStyle::create(bp);
	
	auto pText = new WgTextDisplay();
	pText->SetText("TEXT 1 WITH AUTOWRAPPING ENABLED SO THAT WE GET MULTIPLE LINES.");
	pText->SetSkin( pTextSkin );
	pText->SetTextWrap(true);
	pText->SetTextStyle(pTextStyle);
	pText->SetEditMode(WgTextEditMode::Editable);

	auto pText2 = new WgTextDisplay();
	pText2->SetText("TEXT 2 WITH AUTOWRAPPING ENABLED SO THAT WE GET MULTIPLE LINES.");
	pText2->SetSkin( pTextSkin );
	pText2->SetTextWrap(true);
	pText2->SetTextStyle(pTextStyle);
	pText2->SetEditMode(WgTextEditMode::Editable);
 
	
	auto pPackX = new WgPackPanel();
	pPackX->SetOrientation(wg::Axis::X);
	pPackX->AddChild(pText);
	pPackX->AddChild(pText2);
	pPackX->SetSizeBroker(new WgUniformSizeBroker() );
	
	auto pPackSkin = wg::ColorSkin::create( { .color = WgColor::Brown, .padding = {10,10,10,10} } );

	pBaseLayer->AddChild(pPackX, {50,50,300,160});

	pRoot->SetChild(pBaseLayer);
	return true;

}

//____ gfxStreamingTest() _________________________________________________________

bool gfxStreamingTest(WgRootPanel* pRoot)
{
//    rangeSliderTest(pRoot);
//    fullStateSupportTest(pRoot);
  
//    pianoKeyboardTest(pRoot);
	
/*
	wg::Surface_p pSurf  = sdl_wglib::LoadSurface("resources/splash.png", g_pSurfaceFactory);

	auto pSkin = wg::StaticBlockSkin::create( pSurf );
	
	auto pFiller  = new WgFiller();
	pFiller->SetSkin( pSkin );
//    pFiller->SetSkin(wg::ColorSkin::create(wg::Color::Red));

	pRoot->SetChild(pFiller);
*/
	return true;
}


//____ setupGUI() ______________________________________________________________

WgRootPanel * setupGUI(wg::GfxDevice * pDevice)
{
	std::map<std::string,WgWidget*> * pDB = sdl_wglib::LoadStdWidgets("resources/blocks.png", nullptr, nullptr, g_pSurfaceFactory);
	if (!pDB)
		return 0;

	WgRootPanel * pRoot = new WgRootPanel( wg::CanvasRef::Default, pDevice);

	pRoot->SetScale(4096);

	WgEventHandler * pEventHandler = pRoot->EventHandler();

	WgEventLogger * pEventLogger = new WgEventLogger(std::cout);
	pEventLogger->LogAllEvents();
	pEventLogger->IgnoreEvent(WG_EVENT_TICK);
//	pEventLogger->IgnoreEvent(WG_EVENT_MOUSE_POSITION);
//	pEventLogger->IgnoreEvent(WG_EVENT_MOUSE_MOVE);
	pEventLogger->IgnoreEvent(WG_EVENT_MOUSEBUTTON_REPEAT);

	//	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_POSITION );
	//	pEventLogger->IgnoreEvent( WG_EVENT_MOUSEBUTTON_REPEAT );
	//	pEventLogger->IgnoreEvent( WG_EVENT_BUTTON_PRESS );
	//	pEventLogger->IgnoreAllEvents();
	//	pEventLogger->LogMouseButtonEvents();
	pEventHandler->AddCallback(pEventLogger);


	// Load images and specify blocks

	auto pBackImg = sdl_wglib::LoadSurface("resources/What-Goes-Up-3.bmp", g_pSurfaceFactory, { .tiling = true });
	auto pBackBlock = wg::TileSkin::create(pBackImg);

	auto pSplashImg = sdl_wglib::LoadSurface("resources/splash.png", g_pSurfaceFactory);
	auto pSplashBlock = wg::BlockSkin::create(pSplashImg);

	auto pBigImg = sdl_wglib::LoadSurface("resources/frog.jpg", g_pSurfaceFactory);
	auto pBigBlock = wg::BlockSkin::create(pBigImg);

	auto pPlateImg = sdl_wglib::LoadSurface("resources/grey_pressable_plate.bmp", g_pSurfaceFactory);

	auto pAnimSurf = sdl_wglib::LoadSurface("resources/dummy_anim.png", g_pSurfaceFactory);

	auto pPressablePlateSkin = wg::BlockSkin::create( {
		.axis = wg::Axis::X,
		.firstBlock = wg::Rect(0,0,pPlateImg->pointSize().w/3,pPlateImg->pointSize().h),
		.frame = 3,
		.states = { wg::State::Default, {},
				wg::State::Hovered, {},
				wg::State::Pressed, {}
				
		},
		.surface = pPlateImg
	} );

//	WgSurface * pInjectWidget = sdl_wglib::LoadSurface("resources/IDR_MOD_INJECT_WIDGET_CHROME.2x.png", *g_pSurfaceFactory);
//	pInjectWidget->SetScaleFactor(4096 * 2);

	

//	WgBlockSkinPtr pTagSkin = WgBlockSkin::Create();


	// Popup Layer

	WgPopupLayer * pPopupLayer = new WgPopupLayer();
	pRoot->SetChild(pPopupLayer);

	// Bottom Flex

	WgFlexPanel * pBottom = new WgFlexPanel();
	pPopupLayer->SetBase(pBottom);
//	pRoot->SetChild(pBottom);
	pBottom->SetSkin(wg::ColorSkin::create(wg::Color8(60,60,60,255) ));
//	pBottom->SetSkin(WgColorSkin::Create({ 255,255,255,255 }));

	// Main Flex

	WgFlexPanel * pFlex = new WgFlexPanel();
	pBottom->AddChild(pFlex, (int) WgOrigo::NorthWest, (int) WgOrigo::SouthEast, WgBorders(10));

	// Background

	WgImage * pBackground = new WgImage();
	pBackground->SetSource(pBackBlock);
	WgFlexHook * pHook = pFlex->AddChild(pBackground);
	pHook->SetAnchored( (int) WgOrigo::NorthWest, (int) WgOrigo::SouthEast);
	

	return pRoot;
}


//____ eventLoop() ____________________________________________________________

bool eventLoop( WgEventHandler * pHandler )
{
	if (m_pScrollChart)
	{
		int ticks = SDL_GetTicks();
		

		m_pScrollChart->FeedSample(m_hWave1, sin(ticks / 100.0)*50 );
	}
	sdl_wglib::BeginEvents( pHandler );

   // message processing loop
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// check for messages
		switch (event.type)
		{
			// exit if the window is closed
			case SDL_QUIT:
				return false;

			// check for keypresses
			case SDL_KEYDOWN:
			{
				// exit if ESCAPE is pressed
//				if (event.key.keysym.sym == SDLK_ESCAPE)
//					return false;

				if (event.key.keysym.sym == SDLK_RIGHT)
					g_bRightPressed = true;
				else
					g_bRightPressed = false;

				if (event.key.keysym.sym == SDLK_LEFT)
					g_bLeftPressed = true;
				else
					g_bLeftPressed = false;

				break;
			}
		}
		sdl_wglib::TranslateEvent( event );
	}

	sdl_wglib::EndEvents();

	return true;
}



WgCoord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	WgFlexHook * pHook = static_cast<WgFlexHook*>(pWidget->Hook());


	dragStartPos = pHook->FloatPointOfs();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
	if( _pEvent->Type() != WG_EVENT_MOUSEBUTTON_DRAG || !pWidget->Parent() )
		return;

	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord	dragDistance = pEvent->DraggedTotalPixels();

	WgCoord	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	WgFlexHook * pHook = static_cast<WgFlexHook*>(pWidget->Hook());
	pHook->SetPointOfs(dragStartPos+dragDistance);
}

//____ cbOpenModal() __________________________________________________________

void cbOpenModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
//	g_pModal->AddModal( pWidget, WgCoord(), WgOrigo::SouthEast );
}

//____ cbCloseModal() __________________________________________________________

void cbCloseModal( const WgEvent::Event* _pEvent, WgWidget * pWidget )
{
//	g_pModal->ReleaseChild(pWidget);
}

//____ cbResizeWidget() _________________________________________________________

void cbResize( const WgEvent::Event* _pEvent, void * _pFlexHook )
{
	WgFlexHook * pHook = static_cast<WgFlexHook*>(_pFlexHook);
	const WgEvent::MouseButtonDrag* pEvent = static_cast<const WgEvent::MouseButtonDrag*>(_pEvent);

	WgCoord dragged = pEvent->DraggedNowPixels();

	pHook->SetPointSize( pHook->PixelSize() + WgSize(dragged.x,dragged.y) );
}



//____ addResizablePanel() _________________________________________________

void addResizablePanel( WgFlexPanel * pParent, WgWidget * pChild, WgEventHandler * pEventHandler )
{
	WgHook * pHook = pParent->AddChild( pChild );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pChild, 3), cbResize, pHook );


	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pChild, 1), cbInitDrag, pChild );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pChild, 1), cbDragWidget, pChild );
}




WgFlexPanel * createPresetSelector()
{
	WgFlexPanel * pBottom = new WgFlexPanel();
	
	wg::BoxSkin_p pSkin = wg::BoxSkin::create( { .color = WgColor(0, 0, 0, 255), .outlineColor = WgColor(0, 0, 0, 255), .outlineThickness = 0 } );

	pBottom->SetSkin(pSkin);
/*
		WgButton * pButton = new WgButton();
		pButton->SetText( "BUTTON" );
		pBottom->AddChild( pButton, WgRect(4,75,100,15) );
*/

	for(int i = 0; i< 10; i++)
	{
		WgButton * pButton = new WgButton();
		pButton->SetText( "BUTTON" );
		pBottom->AddChild( pButton, WgRect(4,i*15,100,15) );
	}
	
	return pBottom;
}


/*

SectionPresetSelector::SectionPresetSelector(float fCenterX, float fTopY, float fScale, CSoftubeGUIModule pkModule, WgEventHandler pHandler, WgTextpropPtr pWgProp, WgTextManager pTextManager, WgFlexPanel pkRootPanel, int iParam, std::vector<SectionPreset> pPresets) : WgFlexPanel(),
m_iParam(iParam),
m_pPresets(pPresets),
m_bIsVisible(false)
{
	//Coordinates and sizes are in 1150 px Width to match the rest of the GUI placement
	int iRows = pPresets.size();
	
	float fHalfWidth  = 0.045f;
	if(iParam == eParamReverbPreset)
		fHalfWidth = 0.06f;
	
	float fStepHeight = 0.0232f;
	float fHalfHeightOffset = 0.0125f;
	
	int iTextHeight = (int)floor(14.0f*fScale + .5f);
	int iTextWidth  = (int)floor(90.0f*fScale + .5f);
	if(iParam == eParamReverbPreset)
		iTextWidth  = (int)floor(120.0f*fScale + .5f);
		
	int iBorder     = (int)floor(10.0f*fScale + .5f);
	int iSpacing    = (int)floor(2.0f*fScale + .5f);
	
	//WgBoxSkinPtr pSkin = WgBoxSkin::Create(WgColor(0, 0, 0, 255), WgBorders(0), WgColor(0, 0, 0, 255));
	WgBoxSkinPtr pTextSkin = WgBoxSkin::Create(WgColor(0, 0, 0, 0), WgBorders(0), WgColor(0, 0, 0, 0));

 //SetSkin(pSkin);

	//Place Preset Selection window on root panel
	int iTopLeft = pkRootPanel->AddAnchor(fCenterX-fHalfWidth, fTopY);
	int iBottomRight = pkRootPanel->AddAnchor(fCenterX + fHalfWidth, fTopY + fStepHeight*iRows + 2.0f*fHalfHeightOffset);
	m_pHook = pkRootPanel->AddChild(this, iTopLeft, iBottomRight);
	//m_pHook = pkRootPanel->AddChild(this, WgRect(500, 500, 100, 200), WgOrigo::West);
	DBG_ASSERT(m_pHook != NULL);

	// Create background image
 WgImage* pImage = new WgImage();
 WgSurface* pSurface = pkModule->GetGui()->GetSurface(IDR_BACKGROUND_PRESET);

 WgFlexHook * pkHook = this->AddChild( pImage, WgRect( 0, 0, pSurface->Width(), pSurface->Height()) );

 if( pSurface )
  pImage->SetSource( WgBlockset::CreateFromSurface( pSurface ) );

 pImage->SetMarkOpacity(0);
 pkHook->SetVisible(true);

	//Add text button and callback for all presets
	for(int i = 0; i<pPresets.size(); i++)
	{
		WgGizmoTextButton* pkTextButton;
		pkTextButton = pkModule->AddTextButton(pPresets[i].GetPresetName(), pWgProp, WgCoord(iBorder, i*(iTextHeight+iSpacing)+iBorder), WgCoord(iTextWidth, iTextHeight), i, WgOrigo::NorthWest, this);
		
		pkTextButton->SetSkin(pTextSkin);
		pkTextButton->SetHoverSkin(pTextSkin);
		pkTextButton->SetSelectedSkin(pTextSkin);
		pkTextButton->SetSelectedHoverSkin(pTextSkin);
		pkTextButton->SetDisabledSkin(pTextSkin);
		pkTextButton->SetTextManager(pTextManager);
		pkTextButton->SetTextAlignment(WgOrigo::West);
		pkTextButton->SetParam(pPresets[i].GetPresetParam());
		m_pTextButtons.push_back(pkTextButton);
		
		pHandler->AddCallback( WgEventFilter::KnobTurn(pkTextButton), cbPresetSelectionCallback, pkTextButton);
	}
	m_pHook->SetVisible(m_bIsVisible);
}

*/
