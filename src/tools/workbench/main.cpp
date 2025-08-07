
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>

#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#    include <GL/glew.h>
#elif __APPLE__
#	include <SDL2/SDL.h>
#	include <SDL2_image/SDL_image.h>
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#   include <GL/glew.h>
#endif


#include <wondergui.h>
#include <wg_componentptr.h>
#include <wg_memheap.h>

#include <bitset>
#include <string>
#include <iostream>
#include <climits>

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>

#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_gledgemapfactory.h>
#include <wg_glgfxdevice.h>

#include <wg_freetypefont.h>

#include <wg_dynamicbuffer.h>

#include <wg_drawerpanel.h>

#include <wg_debugger.h>
#include <wg_debugoverlay.h>
#include <themes/simplistic/wg_simplistic.h>


//#define USE_OPEN_GL


using namespace wg;



class MyHostBridge : public HostBridge
{
public:
	bool		hidePointer() override { return false; };
	bool		showPointer() override { return true; };

	bool		lockHidePointer() override { return false; };
	bool		unlockShowPointer() override { return false; };

	bool		setPointerStyle( PointerStyle style ) override { return false; };
	
	std::string	getClipboardText() override { return ""; };
	bool		setClipboardText(const std::string& text) override { return ""; };
	
	bool		requestFocus(uintptr_t windowRef) override { return true; };
	bool		yieldFocus(uintptr_t windowRef) override { return true; };
};


void			unitTestMemHeap();

void 			translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot );
MouseButton 	translateMouseButton( Uint8 button );
void 			updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow );
void 			myButtonClickCallback( const Msg_p& pMsg );
void * 			loadFile( const char * pPath );
Blob_p 			loadBlob( const char * pPath );
Surface_p		loadSurface(const std::string& path, const Surface::Blueprint& blueprint = Surface::Blueprint() );
bool			savePNG(Surface * pSurface, const char * path);

void			convertSDLFormat( PixelDescription * pWGFormat, const SDL_PixelFormat * pSDLFormat );

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter );

bool	bQuit = false;

BlockSkin_p m_pSimpleButtonSkin;
BlockSkin_p m_pPressablePlateSkin;

FreeTypeFont_p m_pFont;

MyHostBridge * g_pHostBridge = nullptr;


void freeSDLSurfCallback( void * pSDLSurf )
{
	SDL_FreeSurface( (SDL_Surface*) pSDLSurf );
}

InputHandler * pDebug;

int sortWidgets( const Widget * p1, const Widget * p2 )
{
	return p2->id() - p1->id();
}

bool shadowLayerTest(ComponentPtr<DynamicSlot> pSlot);
bool stretchBlitTest(ComponentPtr<DynamicSlot> pSlot);
bool scrollIntoViewTest(ComponentPtr<DynamicSlot> pSlot);
bool textClipTest(ComponentPtr<DynamicSlot> pSlot);
bool textEditorTest(ComponentPtr<DynamicSlot> pSlot);
bool lineEditorTest(ComponentPtr<DynamicSlot> pSlot);
bool popupOpenerTest(ComponentPtr<DynamicSlot> pSlot);
bool popupOpenerTest2(ComponentPtr<DynamicSlot> pSlot);
bool scrollbarTest(ComponentPtr<DynamicSlot> pSlot);
bool modalLayerTest(ComponentPtr<DynamicSlot> pSlot);
bool splitPanelTest(ComponentPtr<DynamicSlot> pSlot);
bool designLayerTest(ComponentPtr<DynamicSlot> pSlot);
bool pianoKeyboardTest(ComponentPtr<DynamicSlot> pSlot);
bool sliderTest(ComponentPtr<DynamicSlot> pSlot);
bool rangeSliderTest(ComponentPtr<DynamicSlot> pSlot);
bool pieKnobTest(ComponentPtr<DynamicSlot> pSlot);
bool spinKnobTest(ComponentPtr<DynamicSlot> pSlot);
bool animKnobTest(ComponentPtr<DynamicSlot> pSlot);
bool canvasStackTest(ComponentPtr<DynamicSlot> pSlot);
bool doubleSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool timerTest(ComponentPtr<DynamicSlot> pSlot);
bool animPlayerTest(ComponentPtr<DynamicSlot> pSlot);
bool selectBoxTest(ComponentPtr<DynamicSlot> pSlot);
bool tileSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool gradientSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool bakeSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool animSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool renderLayerTest(ComponentPtr<DynamicSlot> pSlot);
bool rigidPartNinePatchTest(ComponentPtr<DynamicSlot> pSlot);
bool scrollSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool tooltipLayerTest(ComponentPtr<DynamicSlot> pSlot);
bool kerningTest(ComponentPtr<DynamicSlot> pSlot);
bool circleSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool packPanelTest(ComponentPtr<DynamicSlot> pSlot);
bool glyphAsSurfaceTest(ComponentPtr<DynamicSlot> pSlot, Font_p pFont );
bool memHeapFragmentationTest(ComponentPtr<DynamicSlot> pSlot);
bool blendRGB565BigendianTest(ComponentPtr<DynamicSlot> pSlot);
bool twoSlotPanelTest(ComponentPtr<DynamicSlot> pSlot);
bool customSkinTest(ComponentPtr<DynamicSlot> pSlot);
bool areaChartTest(ComponentPtr<DynamicSlot> pSlot);
bool areaChartTest2(ComponentPtr<DynamicSlot> pSlot);
bool plotChartTest(ComponentPtr<DynamicSlot> pSlot);
bool nortonCommanderTest(ComponentPtr<DynamicSlot> pSlot);
bool skinMarginTest(ComponentPtr<DynamicSlot> pSlot);
bool wgcombTest(ComponentPtr<DynamicSlot> pSlot);
bool widgetRecording(ComponentPtr<DynamicSlot> pSlot);
bool canvasCapsuleTest(ComponentPtr<DynamicSlot> pSlot);
bool canvasCapsuleGlowTest(ComponentPtr<DynamicSlot> pSlot);
bool textDisplayTest(ComponentPtr<DynamicSlot> pSlot);
bool scrollChartTest(ComponentPtr<DynamicSlot> pSlot);
bool scrollPanelTest(ComponentPtr<DynamicSlot> pEntry);
bool packPanelStressTest(ComponentPtr<DynamicSlot> pEntry);
bool packPanelStressTest2(ComponentPtr<DynamicSlot> pEntry);
bool blockingCapsuleTest(ComponentPtr<DynamicSlot> pEntry);
bool tablePanelTest(ComponentPtr<DynamicSlot> pEntry);
bool tablePanelTest2(ComponentPtr<DynamicSlot> pEntry);
bool dragndropTest(ComponentPtr<DynamicSlot> pEntry);
bool fillerTransitionTest(ComponentPtr<DynamicSlot> pEntry);
bool reorderCapsuleTest(ComponentPtr<DynamicSlot> pEntry);
bool widgetMoveTest(ComponentPtr<DynamicSlot> pEntry);
bool labelCapsuleTest(ComponentPtr<DynamicSlot> pEntry);
bool elipsisTest(ComponentPtr<DynamicSlot> pEntry);
bool packPanelSpacingBugTest(ComponentPtr<DynamicSlot> pEntry);
bool bracketSkinTest(ComponentPtr<DynamicSlot> pEntry);
bool selectCapsuleTest(ComponentPtr<DynamicSlot> pEntry);
bool drawerPanelTest(ComponentPtr<DynamicSlot> pEntry);


void nisBlendTest();
void commonAncestorTest();
void textStyleTest();

//____ main() _________________________________________________________________

int main(int argc, char** argv)
{
	sleep(1);

/*	Base::init(nullptr);
	unitTestMemHeap();
	Base::exit();
*/

	SoftSurface_p		pCanvas;
	GfxDevice_p			pDevice;
	SurfaceFactory_p	pSurfaceFactory;
	EdgemapFactory_p	pEdgemapFactory;


	Coord muC;
	CoordI iC;
	CoordF fC;



	printf("SizeOf Filler: %d\n", (int) sizeof(Filler));
	printf("SizeOf Widget: %d\n", (int)sizeof(Widget));
	printf("SizeOf Object: %d\n", (int) sizeof(Object));
	printf("SizeOf Receiver: %d\n", (int) sizeof(Receiver));

	printf("SizeOf SkinSlot: %d\n", (int)sizeof(SkinSlot));
	printf("SizeOf String: %d\n", (int)sizeof(String));
	printf("SizeOf SizeSPX: %d\n", (int)sizeof(SizeSPX));


//	printf("Slot is safe to relocate: %d\n", StaticSlot::safe_to_relocate);
//	printf("LambdaSlot is safe to relocate: %d\n", LambdaSlot::safe_to_relocate);


//	pPackPanel->slots[0].

//	auto it = pPackPanel->slots.begin();

//	(*it).setWeight(1.0f);



	// Point test
/*
	Base::init();

	auto pContext0 = Context::create();

	pContext0->setScale(2.25f);

	Base::setActiveContext(pContext0);

	Coord a(1, 2);

	CoordF b = a;
	CoordI c = a;

	float pixX = toPixel(float(a.x));
	float pixY = toPixel(float(a.y));

	CoordI d(MU::fromPX(pixX), MU::fromPX(pixY));


	CoordI cI1 = Coord(1.f, 2.f);
	 
//	CoordI cI2 = CoordF(1.f, 2.f);									// Should fail!
	CoordI cI3 = static_cast<CoordI>(CoordF(1.f, 2.f));

	CoordI cI4 = Rect(1.f, 2.f, 3.f, 4.f);

//	CoordI cI5 = RectF(1.f, 2.f, 3.f, 4.f);							// Should fail!
	CoordI cI6 = static_cast<CoordI>(RectF(1.f, 2.f, 3.f, 4.f));


	SizeI sI1 = Size(1.f, 2.f);

//	SizeI sI2 = SizeF(1.f, 2.f);										// Should fail!
	SizeI sI3 = static_cast<SizeI>(SizeF(1.f, 2.f));

	SizeI sI4 = Rect(1.f, 2.f, 3.f, 4.f);

//	SizeI cI5 = RectF(1.f, 2.f, 3.f, 4.f);							// Should fail!
	SizeI sI6 = static_cast<SizeI>(RectF(1.f, 2.f, 3.f, 4.f));


	RectI rI1 = Rect(1, 2, 3, 4);

//	RectI rI2 = RectF(1.f, 2.f, 3.f, 4.f);							// Should fail!
	RectI rI3 = static_cast<RectI>(RectF(1.f, 2.f, 3.f, 4.f));

	RectI rI4 = Coord(1.f, 2.f);

//	RectI rI5 = CoordF(1.f, 2.f);									// Should fail!
	RectI rI6 = static_cast<RectI>(CoordF(1.f, 2.f));

	RectI rI7 = Size(1.f, 2.f);

//	RectI rI8 = SizeF(1.f, 2.f);										// Should fail!
	RectI rI9 = static_cast<RectI>(SizeF(1.f, 2.f));


	Coord cP1 = CoordF(1.f, 2.f);	
	Coord cP2 = RectF(1.f, 2.f, 3.f, 4.f);

	Size sP1 = SizeF(1.f, 2.f);	
	Size sP2 = RectF(1.f, 2.f, 3.f, 4.f);

	Rect rP1 = RectF(1.f, 2.f, 3.f, 4.f);
	Rect rP2 = CoordF(1.f, 2.f);
	Rect rP3 = SizeF(1.f, 2.f);
*/

	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

//	int posX = 100, posY = 100, width = 1300, height = 1620;
	int posX = 100, posY = 100, width = 1400, height = 800;

#ifdef USE_OPEN_GL

	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	SDL_GLContext context = SDL_GL_CreateContext(pWin);

#if defined(_WIN32) || defined(__linux__)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif

	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();


#else
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0/*SDL_WINDOW_ALLOW_HIGHDPI*/);
#endif



	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------
/*
	struct StructX {

		int aa, bb;

	};

	struct StructY {

		int yy, zz;
	};

	const char * p = Dataset<StructX>::TYPEINFO.className;
	const char * q = Dataset<StructY>::TYPEINFO.className;

	Dataset_p<StructX> pX = Dataset<StructX>::create();

	Dataset_p<StructX> pX2 = pX;

	Dataset_p<StructX> pX3 = Dataset<StructX>::create();

	Dataset_p<StructY> pY = Dataset<StructY>::create( { .yy = 10, .zz = 20} );

	int x = pY->data.yy;
	
	if (pY->typeInfo() != pX->typeInfo())
	{
		int success = true;
	}


	if ( p == pX->typeInfo().className )
	{
		int success = true;
	}

	if ( pX2->typeInfo() == pX3->typeInfo() )
	{
		int success = true;
	}
*/



	
	g_pHostBridge = new MyHostBridge();

	Base::init(g_pHostBridge);
	Base::beginObjectTracking();


//	Base::setErrorHandler([](Error&) { int x = 0; });
	{

#ifdef USE_OPEN_GL
		{
			pCanvas = nullptr;
			auto pGlDevice = GlGfxDevice::create();
			pGlDevice->setDefaultCanvas({ width * 64,height * 64 });
			pDevice = pGlDevice;

			pFactory = GlSurfaceFactory::create();
			pEdgemapFactory = GlEdgemapFactory::create();
		}

#else
		{
			SDL_Surface* pScreen = SDL_GetWindowSurface(pWin);

			PixelFormat type = PixelFormat::Undefined;

			if (pScreen->format->BitsPerPixel == 32)
				type = PixelFormat::BGRA_8;
			else if (pScreen->format->BitsPerPixel == 24)
				type = PixelFormat::BGR_8;

			Blob_p pBlob = Blob::create(pScreen->pixels, nullptr);

			pCanvas = SoftSurface::create(WGBP(Surface,
				_.size = SizeI(width, height),
				_.format = type,
				_.canvas = true),
				pBlob,
				pScreen->pitch);

			auto pSoftBackend = SoftBackend::create();
			addDefaultSoftKernels(pSoftBackend);
			pSoftBackend->defineCanvas(CanvasRef::Default, pCanvas);
			pDevice = GfxDeviceGen2::create(pSoftBackend);

			pSurfaceFactory = SoftSurfaceFactory::create();
			pEdgemapFactory = SoftEdgemapFactory::create();
		}
#endif

		Base::setDefaultSurfaceFactory(pSurfaceFactory);
		Base::setDefaultEdgemapFactory(pEdgemapFactory);
		Base::setDefaultGfxDevice(pDevice);

		//	nisBlendTest();
		commonAncestorTest();
		textStyleTest();

		//	FreeTypeFont::init(SoftSurfaceFactory::create());

		InputHandler_p pInput = Base::inputHandler();

		pInput->mapKey(SDLK_LEFT, Key::Left);
		pInput->mapKey(SDLK_RIGHT, Key::Right);
		pInput->mapKey(SDLK_UP, Key::Up);
		pInput->mapKey(SDLK_DOWN, Key::Down);
		pInput->mapKey(SDLK_BACKSPACE, Key::Backspace);
		pInput->mapKey(SDLK_DELETE, Key::Delete);
		pInput->mapKey(SDLK_END, Key::End);
		pInput->mapKey(SDLK_ESCAPE, Key::Escape);
		pInput->mapKey(SDLK_HOME, Key::Home);
		pInput->mapKey(SDLK_PAGEDOWN, Key::PageDown);
		pInput->mapKey(SDLK_PAGEUP, Key::PageUp);
		pInput->mapKey(SDLK_RETURN, Key::Return);
		pInput->mapKey(SDLK_SPACE, Key::Space);
		pInput->mapKey(SDLK_TAB, Key::Tab);
		pInput->mapKey(SDLK_F1, Key::F1);
		pInput->mapKey(SDLK_F2, Key::F2);
		pInput->mapKey(SDLK_F3, Key::F3);
		pInput->mapKey(SDLK_F4, Key::F4);
		pInput->mapKey(SDLK_F5, Key::F5);
		pInput->mapKey(SDLK_F6, Key::F6);
		pInput->mapKey(SDLK_F7, Key::F7);
		pInput->mapKey(SDLK_F8, Key::F8);
		pInput->mapKey(SDLK_F9, Key::F9);
		pInput->mapKey(SDLK_F10, Key::F10);
		pInput->mapKey(SDLK_F11, Key::F11);
		pInput->mapKey(SDLK_F12, Key::F12);


		pInput->mapKey(SDLK_LSHIFT, Key::Shift);
		pInput->mapKey(SDLK_RSHIFT, Key::Shift);

		pInput->mapKey(SDLK_LALT, Key::Alt);
		pInput->mapKey(SDLK_RALT, Key::Alt);

	#ifdef __APPLE__
		pInput->mapKey(SDLK_LCTRL, Key::MacControl);
		pInput->mapKey(SDLK_RCTRL, Key::MacControl);

		pInput->mapKey(SDLK_LGUI, Key::Command);
		pInput->mapKey(SDLK_RGUI, Key::Command);
	#else
		pInput->mapKey(SDLK_LCTRL, Key::StdControl);
		pInput->mapKey(SDLK_RCTRL, Key::StdControl);

		pInput->mapKey(SDLK_LGUI, Key::OSKey);
		pInput->mapKey(SDLK_RGUI, Key::OSKey);

	#endif
		
		pInput->mapKey(SDLK_KP_ENTER, Key::Return);


		pInput->mapCommand(SDLK_x, ModKeys::Command, EditCmd::Cut);
		pInput->mapCommand(SDLK_c, ModKeys::Command, EditCmd::Copy);
		pInput->mapCommand(SDLK_v, ModKeys::Command, EditCmd::Paste);

		pInput->mapCommand(SDLK_a, ModKeys::Command, EditCmd::SelectAll);

		pInput->mapCommand(SDLK_z, ModKeys::Command, EditCmd::Undo);
		pInput->mapCommand(SDLK_z, ModKeys::CommandShift, EditCmd::Redo);
		
		pInput->mapCommand(SDLK_ESCAPE, ModKeys::None, EditCmd::Escape);


		/*
			PixelFormat format = PixelFormat::Unknown;

				if( pWinSurf->format->BitsPerPixel == 32 )
					format = PixelFormat::BGRA_8;
				else if( pWinSurf->format->BitsPerPixel == 24 )
					format = PixelFormat::BGR_8;

				Blob_p pCanvasBlob = Blob::create( pWinSurf->pixels, 0);
				SoftSurface_p pCanvas = SoftSurface::create( SizeI(pWinSurf->w,pWinSurf->h), format, pCanvasBlob, pWinSurf->pitch );

				SoftGfxDevice_p pGfxDevice = SoftGfxDevice::create( pCanvas );
		*/


		RootPanel_p pRoot = RootPanel::create(CanvasRef::Default, pDevice);
		/*
			auto pCanvasLayers = CanvasLayers::create( WGBP(CanvasLayers,
				_.layers = { {.format = PixelFormat::Alpha_8 }, {.format = PixelFormat::BGRA_8}, {.format = PixelFormat::BGRA_8} }
													));
		*/

		auto pCanvasLayers = CanvasLayers::create(WGBP(CanvasLayers,
			_.layers = { { PixelFormat::Alpha_8 }, { PixelFormat::BGRA_8 }, { PixelFormat::BGRA_8, nullptr } }
		));


		pRoot->setCanvasLayers(pCanvasLayers);

		//		pRoot->setDebugMode(true);

		Base::inputHandler()->setFocusedWindow(pRoot);

		//

		MsgLogger_p pLogger = MsgLogger::create(std::cout);
		pLogger->logAllMsgs(true);
		pLogger->logMsg(MsgType::MouseMove, false);
		pLogger->logMsg(MsgType::MouseDrag, false);
		pLogger->logMsg(MsgType::MouseRepeat, false);

		Base::msgRouter()->broadcastTo(pLogger);

		// Init font

		PixelDescription	pixelDesc;
		/*
			char * pFontSpec = (char*)loadFile("resources/anuvverbubbla_8x8.fnt");

			SDL_Surface * pFontSurf = IMG_Load("resources/anuvverbubbla_8x8.png");
			//	convertSDLFormat( &pixelDesc, pFontSurf->format );
			Surface_p pFontImg = pSurfaceFactory->createSurface(SizeI(pFontSurf->w, pFontSurf->h), PixelFormat::BGRA_8, (unsigned char*)pFontSurf->pixels, pFontSurf->pitch);
			SDL_FreeSurface(pFontSurf);

			BitmapFont_p pBmpFont = BitmapFont::create(pFontImg, pFontSpec);

			TextStyle_p pStyle = TextStyle::create();
			pStyle->setFont(pBmpFont);
			pStyle->setSize(8);
			Base::setDefaultStyle(pStyle);
		*/

		Blob_p pFontFile = loadBlob("resources/DroidSans.ttf");

		m_pFont = FreeTypeFont::create(pFontFile);

		TextStyle_p pStyle = TextStyle::create({ .color = Color::Black, .font = m_pFont, .size = 16 });
		Base::setDefaultStyle(pStyle);


		TextStyle_p pStyle2 = TextStyle::create({ .color = Color::Black,
													.font = m_pFont,
													.size = 16,
													.states = { {State::Hovered, {.color = Color::Grey, .size = 17} }, {State::Pressed, {.color = Color::Red }} }
			});


		/*
			BasicTextLayout_p pMapper = BasicTextLayout::create();
			pMapper->setSelectionBackColor(Color(0,0,255,128), BlendMode::Blend);
			Base::setDefaultTextLayout(pMapper);
		*/


		// Init skins

		SDL_Surface* pSDLSurf = IMG_Load("resources/simple_button.bmp");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pButtonSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		m_pSimpleButtonSkin = BlockSkin::create({
			.axis = wg::Axis::X,
			.frame = 3,
			.padding = 5,
			.states = { {State::Hovered}, {State::Pressed, {.contentShift = Coord(2.f,2.f) }}, {State::Disabled} },
			.surface = pButtonSurface
			});

		pSDLSurf = IMG_Load("resources/simple_icon.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pBackgroundSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		BlockSkin_p pBackgroundSkin = BlockSkin::create(pBackgroundSurface);


		pSDLSurf = IMG_Load("resources/splash.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pSplashSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		BlockSkin_p pSplashSkin = BlockSkin::create(pSplashSurface);


		pSDLSurf = IMG_Load("resources/state_button.bmp");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pStateButtonSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		//	BlockSkin_p pStateButtonSkin = BlockSkin::create(pStateButtonSurface, { State::Default, State::Hovered, State::Checked, State::Checked + State::Hovered, State::Disabled }, Border(3), Axis::X);
		auto pStateButtonSkin = BlockSkin::create(BlockSkin::Blueprint
		{
			.axis = Axis::X,
				.frame = 3,
				.padding = 5,
				.states = { State::Default, {}, State::Hovered, {},
								State::Checked, {}, State::Checked + State::Hovered, {},
								State::Disabled, {}
			},
				.surface = pStateButtonSurface
		});

		pSDLSurf = IMG_Load("resources/grey_pressable_plate.bmp");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pPressablePlateSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		//	BlockSkin_p pPressablePlateSkin = BlockSkin::create(pPressablePlateSurface, { State::Default, State::Hovered, State::Pressed, State::Disabled }, Border(3), Axis::X);
		m_pPressablePlateSkin = BlockSkin::create(BlockSkin::Blueprint
		{
			.axis = Axis::X,
				.frame = 3,
				.padding = 3,
				.states = { State::Default, {}, State::Hovered, {},
								State::Pressed, {}, State::Disabled, {}
			},
				.surface = pPressablePlateSurface
		});

		pSDLSurf = IMG_Load("resources/list_entry.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pListEntrySurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGRA_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		Skin_p pListEntrySkin = BlockSkin::create(pListEntrySurface, { State::Default, State::Hovered, State::Selected, State::Selected + State::Hovered, State::Disabled }, Border(2), Axis::X);

		pSDLSurf = IMG_Load("resources/splash.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pImgSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGR_8, .sampleMethod = SampleMethod::Bilinear, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		BlockSkin_p pImgSkin = BlockSkin::createStaticFromSurface(pImgSurface);


		//------------------------------------------------------
		// Init theme
		//------------------------------------------------------


		auto pFont1Blob = loadBlob("resources/NotoSans-Regular.ttf");
		auto pFont2Blob = loadBlob("resources/NotoSans-Bold.ttf");
		auto pFont3Blob = loadBlob("resources/NotoSans-Italic.ttf");
		auto pFont4Blob = loadBlob("resources/DroidSansMono.ttf");

		auto pFont1 = FreeTypeFont::create(pFont1Blob);
		auto pFont2 = FreeTypeFont::create(pFont2Blob);
		auto pFont3 = FreeTypeFont::create(pFont3Blob);
		auto pFont4 = FreeTypeFont::create(pFont4Blob);


		pSDLSurf = IMG_Load("resources/skin_widgets.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pThemeSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGRA_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);

		auto pTheme = Simplistic::create(pFont1,pFont2,pFont3,pFont4,pThemeSurface);
		if (!pTheme)
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create default theme", nullptr, nullptr, __func__, __FILE__, __LINE__);
			return -1;
		}
		Base::setDefaultTheme(pTheme);
		Base::setDefaultStyle(pTheme->defaultStyle());



		//------------------------------------------------------
		// Setup debugger
		//------------------------------------------------------

		auto pDebugger = Debugger::create();


		pSDLSurf = IMG_Load("resources/debugger_gfx.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pIconSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::BGRA_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);

		pSDLSurf = IMG_Load("resources/checkboardtile.png");
		convertSDLFormat(&pixelDesc, pSDLSurf->format);
		Surface_p pTransparencyGrid = pSurfaceFactory->createSurface({ .format = PixelFormat::BGRA_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h), .tiling = true }, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);


		auto pDebugOverlay = DebugOverlay::create( { .debugger = pDebugger, .theme = pTheme, .icons = pIconSurface, .transparencyGrid = pTransparencyGrid } );

		pRoot->slot = pDebugOverlay;
		pRoot->setSkin(ColorSkin::create(Color::Black));

		//------------------------------------------------------
		// Setup a simple GUI consisting of a filled background and
		// a button using scaled bitmaps.
		//------------------------------------------------------

		DragNDropOverlay_p pDnDLayer = DragNDropOverlay::create();
		pDebugOverlay->mainSlot = pDnDLayer;

		PopupOverlay_p pPopupOverlay = PopupOverlay::create();
		pDnDLayer->mainSlot = pPopupOverlay;

		LambdaPanel_p pBasePanel = LambdaPanel::create();
		pBasePanel->setSkin(ColorSkin::create(Color::Burlywood));
		pPopupOverlay->mainSlot = pBasePanel;

		/*	Filler_p pBackground = Filler::create();
			pBackground->setSkin( ColorSkin::create(Color::aqua) );
			pFlexPanel->addWidget(pBackground, WG_NORTHWEST, CoordI(), WG_SOUTHEAST, CoordI());
		*/


		//		auto pTestSkin = BoxSkin::create( Border(5), {{State::Default, Color::Red, Color::Black}, {State::Pressed, Color::Red, Color::Pink} , {State::Hovered, Color::Green, Color::LightGreen} });
		//		pTestSkin->setBlendMode(BlendMode::Add);

		//	Image_p pImage = Image::create();
		//	pImage->setSkin( pSimpleButtonSkin );
		//	pFlexPanel->slots.addMovable( pImage, RectI(0,0,80*2,33*2), Placement::Center, Placement::Center );


		//	pRoot->msgRouter()->AddCallback( MsgFilter::select(), pButton, myButtonClickCallback );

//		Base::update(int64_t(SDL_GetTicks()) * 1000 + 5000000);


		auto pSlot = ComponentPtr(pDnDLayer, pDnDLayer->mainSlot);

		//	shadowLayerTest(pRoot->child.ptr());
		//	stretchBlitTest(pRoot->child.ptr());
		//	scrollIntoViewTest(pRoot->child.ptr());
		//	textClipTest(pSlot);
		//	textEditorTest(pSlot);
		//	lineEditorTest(pSlot);
		//	popupOpenerTest(pSlot);
		//	popupOpenerTest2(pSlot);
		//	scrollbarTest(pSlot);
		//	modalLayerTest(pSlot);
		//	splitPanelTest(pSlot);
		//	designLayerTest(pSlot);
		//	pianoKeyboardTest(pSlot);
		//	sliderTest(pSlot);
		//	rangeSliderTest(pSlot);
		//	pieKnobTest(pSlot);
		//	spinKnobTest(pSlot);
		//  animKnobTest(pSlot);
		//	canvasStackTest(pSlot);
		//	doubleSkinTest(pSlot);
		//	timerTest(pSlot);
		//	animPlayerTest(pSlot);
		//	selectBoxTest(pSlot);
		//	tileSkinTest(pSlot);
		//	gradientSkinTest(pSlot);
		//	bakeSkinTest(pSlot);
		//	animSkinTest(pSlot);
		//	renderLayerTest(pSlot);
		//	rigidPartNinePatchTest(pSlot);
			scrollSkinTest(pSlot);
		//	tooltipLayerTest(pSlot);
		//	kerningTest(pSlot);
		//	circleSkinTest(pSlot);
		//	packPanelTest(pSlot);
		//	glyphAsSurfaceTest(pSlot, pFont);
		//	memHeapFragmentationTest(pSlot);
		//	blendRGB565BigendianTest( pSlot );
		//	twoSlotPanelTest(pSlot);
		//	customSkinTest(pSlot);
		//	areaChartTest(pSlot);
		//	areaChartTest2(pSlot);
		//	plotChartTest(pSlot);
		//	nortonCommanderTest(pSlot);
		//	skinMarginTest(pSlot);
		//	wgcombTest(pSlot);
		//  widgetRecording(pSlot);
		//	canvasCapsuleTest(pSlot);
		//	canvasCapsuleGlowTest(pSlot);
		//	textDisplayTest(pSlot);
		//  scrollChartTest(pSlot);
		//  scrollPanelTest(pSlot);
		//	packPanelStressTest(pSlot);
		//	packPanelStressTest2(pSlot);
		//	blockingCapsuleTest(pSlot);
		//	tablePanelTest(pSlot);
		//	tablePanelTest2(pSlot);
		//	dragndropTest(pSlot);
		//	fillerTransitionTest(pSlot);
			reorderCapsuleTest(pSlot);
		//	widgetMoveTest(pSlot);
		//	labelCapsuleTest(pSlot);
		//	elipsisTest(pSlot);
		//	packPanelSpacingBugTest(pSlot);
		//	bracketSkinTest(pSlot);
		//	selectCapsuleTest(pSlot);
		//	drawerPanelTest(pSlot);

		//------------------------------------------------------
		// Program Main Loop
		//------------------------------------------------------


		
		while (!bQuit)
		{

			translateEvents(pInput, pRoot);
			Base::update(int64_t(SDL_GetTicks()) * 1000);
			pRoot->render();
			//		SDL_UpdateWindowSurface(pWin);
			updateWindowRects(pRoot, pWin);

			SDL_Delay(4);
		}

		// Cleanup

	//	pSlot = ComponentPtr(nullptr, nullptr);

	}

	pCanvas = nullptr;
	pDevice = nullptr;
	pSurfaceFactory = nullptr;
	pEdgemapFactory = nullptr;

	m_pSimpleButtonSkin = nullptr;
	m_pFont = nullptr;
	
	// Exit WonderGUI

	Base::printObjects(std::cout);
	Base::endObjectTracking();
	Base::exit();

	// Exit SDL

	IMG_Quit();
	SDL_Quit();

	delete g_pHostBridge;
	return 0;
}



//____ translateEvents() ___________________________________________________________

void translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot )
{
	// Process all the SDL events in a loop

	SDL_Event e;
	while(SDL_PollEvent(&e)) {

		switch( e.type )
		{
			case SDL_QUIT:
				bQuit = true;
				break;

			case SDL_MOUSEMOTION:
				pInput->setPointer(pRoot, { pts(e.motion.x), pts(e.motion.y) });
				break;

			case SDL_MOUSEBUTTONDOWN:
				pInput->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_MOUSEBUTTONUP:
				pInput->setButton( translateMouseButton(e.button.button), false );
				break;

			case SDL_MOUSEWHEEL:
			{
				bool bInvertScroll = false;
				Coord distance(e.wheel.x, e.wheel.y);
				if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				{
					bInvertScroll = true;
					distance *= -1;
				}

				if (e.wheel.y != 0)
					pInput->setWheelRoll(1, distance.y, bInvertScroll);

				if (e.wheel.x != 0)
					pInput->setWheelRoll(2, distance.x, bInvertScroll);
				break;
			}

			case SDL_KEYDOWN:
			{
				pInput->setKey( e.key.keysym.sym, true );
				break;
			}

			case SDL_KEYUP:
			{
				pInput->setKey( e.key.keysym.sym, false );
				break;
			}

			case SDL_TEXTINPUT:
				pInput->putText( e.text.text );
				break;


			default:
				break;
		}
	}

	Base::msgRouter()->dispatch();
}

//____ translateMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateMouseButton( Uint8 button )
{
	switch( button )
	{
		default:
		case SDL_BUTTON_LEFT:
			return MouseButton::Left;
		case SDL_BUTTON_MIDDLE:
			return MouseButton::Middle;
		case SDL_BUTTON_RIGHT:
			return MouseButton::Right;
		case SDL_BUTTON_X1:
			return MouseButton::X1;
		case SDL_BUTTON_X2:
			return MouseButton::X2;
	}
}

//____ updateWindowRects() _____________________________________________________
//
// Updates the rectangles of the SDL Window that WonderGUI has modified.
//
void updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow )
{
	int nRects = pRoot->nbUpdatedRects();
	if( nRects == 0 )
		return;

	const RectSPX * pUpdatedRects = pRoot->firstUpdatedRect();
	SDL_Rect * pSDLRects = (SDL_Rect*) Base::memStackAlloc( sizeof(SDL_Rect) * nRects );

	for( int i = 0 ; i < nRects ; i++ )
	{
		pSDLRects[i].x = pUpdatedRects[i].x/64;
		pSDLRects[i].y = pUpdatedRects[i].y/64;
		pSDLRects[i].w = pUpdatedRects[i].w/64;
		pSDLRects[i].h = pUpdatedRects[i].h/64;
	}

	SDL_UpdateWindowSurfaceRects( pWindow, pSDLRects, nRects );

	Base::memStackFree( sizeof(SDL_Rect) * nRects );
}

//____ myButtonClickCallback() _________________________________________________

void myButtonClickCallback( const Msg_p& pMsg )
{
	bQuit = true;
}



//____ loadBlob() _____________________________________________________________

Blob_p loadBlob( const char * pPath )
{
	FILE* fp = fopen( pPath, "rb" );
	if( !fp )
		return 0;

	fseek( fp, 0, SEEK_END );
	size_t size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	Blob_p pBlob = Blob::create( (int) size );

	size_t nRead = fread( pBlob->data(), 1, size, fp );
	fclose( fp );

	if( nRead < size )
		return 0;

	return pBlob;

}


//____ loadFile() _____________________________________________________________

void * loadFile( const char * pPath )
{
	FILE* fp = fopen(pPath, "rb");
	if( !fp )
		return 0;

	fseek( fp, 0, SEEK_END );
	size_t size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	char * pMem = (char*) malloc( size+1 );
	pMem[size] = 0;
	size_t nRead = fread( pMem, 1, size, fp );
	fclose( fp );

	if( nRead < size )
	{
		free( pMem );
		return 0;
	}

	return pMem;

}


CoordI dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const Msg_p& _pMsg, const Object_p& pObject )
{
/*
	Widget_p pWidget = Widget::cast(pObject);

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());


	dragStartPos = pHook->floatOfs();
	printf( "DRAG START!\n" );
*/
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const Msg_p& _pMsg, const Object_p& pObject )
{
/*
	Widget_p pWidget = Widget::cast(pObject);

	if( _pMsg->type() != MsgType::MouseDrag || !pWidget->parent() )
		return;

	const MouseDragMsg_p pMsg = MouseDragMsg::cast(_pMsg);



	CoordI	dragDistance = pMsg->draggedTotal();

	CoordI	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());
	pHook->setOfs(dragStartPos+dragDistance);
*/
}

//____ cbMoveResize() _________________________________________________________

void cbMoveResize( const Msg_p& _pMsg, const Object_p& _pWidget )
{
/*
	static CoordI posAtPress[MouseButton_Max];

	auto	pWidget = Widget::cast(_pWidget);
	FlexHook_p 	pHook = FlexHook::cast(pWidget->hook());

	switch( _pMsg->type() )
	{
		case MsgType::MousePress:
		{
			auto pMsg = MousePressMsg::cast(_pMsg);
			posAtPress[(int)pMsg->button()] = pWidget->pos();

		}
		break;
		case MsgType::MouseDrag:
		{
			auto pMsg = MouseDragMsg::cast(_pMsg);
			if( pMsg->button() == MouseButton::Right )
			{
				pHook->setSize( pHook->size() + pMsg->draggedNow().toSize() );
			}
			else if( pMsg->button() == MouseButton::Middle )
			{
				pHook->setOfs( posAtPress[(int)MouseButton::Middle] + pMsg->draggedTotal() );
			}
		}
		break;
		case MsgType::MouseRelease:
		break;

		default:
		break;
	}
*/
}


//____ addResizablePanel() _________________________________________________

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter )
{
	pParent->slots.pushBack( pChild );
//	pMsgRouter->addRoute( pChild, MsgFunc::create(cbMoveResize, pChild) );
}

//____ convertSDLFormat() ______________________________________________________

void convertSDLFormat( PixelDescription * pWGFormat, const SDL_PixelFormat * pSDLFormat )
{
	pWGFormat->type = PixelType::Chunky;
	pWGFormat->bits = pSDLFormat->BitsPerPixel;

	pWGFormat->R_mask = pSDLFormat->Rmask;
	pWGFormat->G_mask = pSDLFormat->Gmask;
	pWGFormat->B_mask = pSDLFormat->Bmask;
	pWGFormat->A_mask = pSDLFormat->Amask;

}


//____ loadSurface() ___________________________________________________________

Surface_p loadSurface(const std::string& path, const Surface::Blueprint& blueprint)
{
	PixelDescription	pixelDesc;

	auto pSDLSurf = IMG_Load(path.c_str());
	if (pSDLSurf == nullptr)
	{
		printf("ERROR: Could not load '%s'\n", path.c_str());
		exit(-1);
	}

	convertSDLFormat(&pixelDesc, pSDLSurf->format);
	
	Surface::Blueprint bp = blueprint;
	bp.size.w = pSDLSurf->w;
	bp.size.h = pSDLSurf->h;

	if (pixelDesc.bits == 24)
		bp.format = PixelFormat::BGR_8;
	else if (pixelDesc.A_mask == 0)
		bp.format = PixelFormat::BGRX_8;
	else
		bp.format = PixelFormat::BGRA_8;

	Surface_p pImgSurface = Base::defaultSurfaceFactory()->createSurface( bp, (unsigned char*)pSDLSurf->pixels, pixelDesc, pSDLSurf->pitch);

	SDL_FreeSurface(pSDLSurf);

	return pImgSurface;
}

//____ savePNG() ________________________________________________________

bool savePNG(Surface * pSurface, const char * path)
{
	SizeI size = pSurface->pixelSize();
	const PixelDescription* pFmt = pSurface->pixelDescription();

	SDL_Surface * pOutput = SDL_CreateRGBSurface(0, size.w, size.h, pFmt->bits, (uint32_t) pFmt->R_mask, (uint32_t) pFmt->G_mask, (uint32_t) pFmt->B_mask, (uint32_t) pFmt->A_mask);


	int err = SDL_LockSurface(pOutput);
	if (err != 0)
		return false;

	auto pixbuf = pSurface->allocPixelBuffer();
	pSurface->pushPixels(pixbuf);

	for (int y = 0; y < size.h; y++)
	{
		char * pDest = ((char *)pOutput->pixels) + pOutput->pitch * y;
		char * pSource = (char*) pixbuf.pixels + pixbuf.pitch * y;
		memcpy(pDest, pSource, size.w * pFmt->bits / 8);
	}

	pSurface->freePixelBuffer(pixbuf);

	SDL_UnlockSurface(pOutput);


	IMG_SavePNG(pOutput, path);
	SDL_FreeSurface(pOutput);

	return true;
}



//____ shadowLayerTest() ______________________________________________________

bool shadowLayerTest(ComponentPtr<DynamicSlot> pEntry )
{
/*
	Surface_p pImgSurface = loadSurface("resources/shadow.png", PixelFormat::A8);
	
	BlockStateSkin_p pShadowSkin = BlockSkin::createStaticFromSurface(pImgSurface);
	pShadowSkin->setFrame({ 0,128,128,0 });
	pShadowSkin->setContentPadding({ 0,128,128,0 });
	pImgSurface->setSampleMethod(SampleMethod::Nearest);


	auto pShadowLayer = ShadowLayer::create();
	auto pFrontLayer = FlexPanel::create();
	auto pBaseLayer = FlexPanel::create();

	pRoot->child = pShadowLayer;

	pShadowLayer->base = pBaseLayer;
	pShadowLayer->front = pFrontLayer;


	auto pFiller1 = Filler::create();
	auto pFiller2 = Filler::create();
	auto pFiller3 = Filler::create();

	pFiller1->setSkin(ColorSkin::create(Color::DarkKhaki));
	pFrontLayer->slots.addMovable(pFiller1, { 0,0,50,50 });

	pFiller2->setSkin(ColorSkin::create(Color::DarkSlateBlue));
	pFrontLayer->slots.addMovable(pFiller2, { 50,50,50,50 });

	pFiller3->setSkin(ColorSkin::create(Color::DarkSeaGreen));
	pFrontLayer->slots.addMovable(pFiller3, { 100,100,50,50 });

	auto pBackground = Filler::create();
	pBackground->setSkin(ColorSkin::create(Color::LightSalmon));
	pBaseLayer->slots.addPinned(pBackground, Placement::NorthWest, Placement::SouthEast);

	pShadowLayer->shadows.add(pFiller1, pShadowSkin);
	pShadowLayer->shadows.add(pFiller2, pShadowSkin);
	pShadowLayer->shadows.add(pFiller3, pShadowSkin);

	pFiller1->setId(7);

	Base::msgRouter()->addRoute(pFiller1, MsgType::MouseDrag, [pFrontLayer](Msg* _pMsg)
	{
		MouseDragMsg* pMsg = static_cast<MouseDragMsg*>(_pMsg);
		pFrontLayer->slots.move(0, pMsg->draggedNow());
	});

	Base::msgRouter()->addRoute(pFiller2, MsgType::MouseDrag, [pFrontLayer](Msg* _pMsg)
	{
		MouseDragMsg* pMsg = static_cast<MouseDragMsg*>(_pMsg);
		pFrontLayer->slots.move(1, pMsg->draggedNow());
	});

	Base::msgRouter()->addRoute(pFiller3, MsgType::MouseDrag, [pFrontLayer](Msg* _pMsg)
	{
		MouseDragMsg* pMsg = static_cast<MouseDragMsg*>(_pMsg);
		pFrontLayer->slots.move(2, pMsg->draggedNow());
	});
*/
	return true;
}

//____ stretchBlitTest() ______________________________________________________

bool stretchBlitTest(ComponentPtr<DynamicSlot> pEntry)
{
	Surface_p pImgSurface = loadSurface("resources/white_frame_256x256.png", { .format = PixelFormat::BGR_8, .sampleMethod = SampleMethod::Bilinear });

	auto pBack = FlexPanel::create();
	pBack->setSkin( ColorSkin::create(Color::Blue) );
	* pEntry = pBack;

	auto pImage = Image::create();
	pImage->setImage(pImgSurface, { 0,0,256,256 });
//	pImage->setImage(pImgSurface, { 1,1,254,254 });
	pBack->slots.pushBack(pImage, { .pos = {10,10}, .size = {256,256} });

	Base::msgRouter()->addRoute(pImage, MsgType::MouseDrag, [pBack,pImage](Msg* pMsg) { 
	
		Coord distance = static_cast<MouseDragMsg*>(pMsg)->draggedNow();

		pBack->slots[0].setSize(pImage->size()+Size(distance.x, distance.y) );
	
	});

	return true;
}

//____ scrollIntoViewTest() ______________________________________________________

bool scrollIntoViewTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pEditor = TextEditor::create();

	pEditor->editor.setText("TEXT AREA.");
	pEditor->setSkin( ColorSkin::create(Color::YellowGreen) );

	auto pScrollPanel = ScrollPanel::create();
	pScrollPanel->slot = pEditor;
	pScrollPanel->setSkin( ColorSkin::create(Color::Pink) );


	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pScrollPanel, { .pos = {10,10}, .size = {200,50} });

	* pEntry = pFlex;

	return true;
}




//____ textClipTest() ______________________________________________________

bool textClipTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pSkin = ColorSkin::create(Color::YellowGreen, 10);

	auto pEditor = TextEditor::create();

	pEditor->editor.setText("TEXTAREA LINE 1.\nTEXTAREA LINE 2\nTEXTAREA LINE 3.");
	pEditor->setSkin( pSkin );

	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pEditor, { .pos = {10,10}, .size = {50,30} });

	*pEntry = pFlex;

	return true;
}

//____ textEditorTest() ______________________________________________________

bool textEditorTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pLayout = BasicTextLayout::create({ .lineSpacing = 1.2f, .paragraphSpacing = 1.f, .wrap = true});

	auto pSkin = ColorSkin::create(Color::YellowGreen, 2);

	auto pEditor = TextEditor::create( {	.editor = { .layout = pLayout, .text = String("TEXTAREA LINE 1.\nTEXTAREA LINE 2\nTEXTAREA LINE 3.") },
											.skin = pSkin});

	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pEditor, { .pos = { 10,10 }, .size = { 150, 200 } });

	*pEntry = pFlex;

	return true;
}


//____ lineEditorTest() ______________________________________________________

bool lineEditorTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pSkin = ColorSkin::create(Color::YellowGreen, 10);
	auto pFlex = FlexPanel::create();

	auto pEditor = LineEditor::create( { .skin = pSkin, .stickyFocus = true });
	pEditor->editor.setText("TEXT.");
	pFlex->slots.pushBack(pEditor, { .pos = {10,100}, .size = {80,40} });

	auto pEditor2 = LineEditor::create({ .skin = pSkin });
	pEditor2->editor.setText("TEXT.");
	pFlex->slots.pushBack(pEditor2, { .pos = {10,150}, .size = {80,40} });

	auto pButton = Button::create( { .label = {.text = "SET TEXT" }, .skin = m_pSimpleButtonSkin } );

	Base::msgRouter()->addRoute(pButton, MsgType::Select, [pEditor2](Msg* p){ pEditor2->editor.setText("MODIFIED."); } );
	
	pFlex->slots.pushBack(pButton, { .pos = {10, 200} });
	

	
	
	* pEntry = pFlex;
	return true;
}

//____ popupOpenerTest2() ______________________________________________________

bool popupOpenerTest2(ComponentPtr<DynamicSlot> pEntry)
{
	auto pButtonSkin = BoxSkin::create( WGBP(BoxSkin,
											 _.outlineThickness = 1,
											 _.outlineColor = Color::Black,
											 _.padding = 8,
											 _.states = { { State::Default, { .color = Color::LightGrey }}, { State::Hovered, { .color = Color::Green }} }
										));

	auto pBackSkin = BoxSkin::create( WGBP(BoxSkin,
											 _.outlineThickness = 1,
											 _.outlineColor = Color::Black,
											 _.padding = 8,
											 _.color = Color::LightGrey
										));

	
	auto pPopupOverlay = PopupOverlay::create( { .blockLeftMouseButton = true, .skin = ColorSkin::create(Color::PaleGoldenrod) } );
	* pEntry = pPopupOverlay;

	auto pFlex = FlexPanel::create();
	pPopupOverlay->mainSlot = pFlex;

	pFlex->setSkin( ColorSkin::create(Color::OrangeRed) );

	auto  pOpener = PopupOpener::create({ .closeOnSelect = false, .label = {.text = "OPEN POPUP"}, .openOnHover = true, .skin = pButtonSkin });
	pFlex->slots.pushBack(pOpener, { .pos = {50,50}, .size = {70,40} });

	auto pOpened = PackPanel::create( { .axis = Axis::Y, .skin = pBackSkin } );
	pOpener->setPopup(pOpened);


	auto pSubEntry1 = Filler::create( { .defaultSize = { 100, 20 }, .id = 11, .skin = pButtonSkin });

	auto pSubEntry2 = Filler::create( { .defaultSize = { 100, 20 }, .id = 21, .skin = pButtonSkin });

	auto pSubEntry3 = Filler::create( { .defaultSize = { 100, 20 }, .id = 31, .skin = pButtonSkin });

	auto pSubEntry4 = Filler::create( { .defaultSize = { 100, 20 }, .id = 41, .skin = pButtonSkin });


	auto  pEntry1_1 = PopupOpener::create({ .attachPoint =  Placement::West, .closeOnSelect = false, .id = 10, .label = {.text = "First"}, .openOnHover = true, .selectable = true, .skin = pButtonSkin });
	pEntry1_1->setPopup(pSubEntry1);
	pOpened->slots << pEntry1_1;

	auto  pEntry1_2 = PopupOpener::create({ .attachPoint =  Placement::West, .closeOnSelect = false, .id = 20, .label = {.text = "Second"}, .openOnHover = true, .selectable = true, .skin = pButtonSkin });
	pEntry1_2->setPopup(pSubEntry2);
	pOpened->slots << pEntry1_2;

	auto  pEntry1_3 = PopupOpener::create({ .attachPoint =  Placement::West, .closeOnSelect = false, .id = 30, .label = {.text = "Third"}, .openOnHover = true, .selectable = true, .skin = pButtonSkin });
	pEntry1_3->setPopup(pSubEntry3);
	pOpened->slots << pEntry1_3;

	auto  pEntry1_4 = PopupOpener::create({ .attachPoint =  Placement::West, .closeOnSelect = false, .id = 40, .label = {.text = "Fourth"}, .openOnHover = true, .selectable = true, .skin = pButtonSkin });
	pEntry1_4->setPopup(pSubEntry4);
	pOpened->slots << pEntry1_4;


	return true;
}

//____ popupOpenerTest() ______________________________________________________

bool popupOpenerTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pButtonSkin = BoxSkin::create( WGBP(BoxSkin,
											 _.outlineThickness = 1,
											 _.outlineColor = Color::Black,
											 _.padding = 8,
											 _.states = { { State::Default, { .color = Color::LightGrey }}, { State::Hovered, { .color = Color::Green }} }
										));

	auto pBackSkin = BoxSkin::create( WGBP(BoxSkin,
											 _.outlineThickness = 1,
											 _.outlineColor = Color::Black,
											 _.padding = 8,
											 _.color = Color::LightGrey
										));


	auto pPopupOverlay = PopupOverlay::create( { .blockLeftMouseButton = true, .skin = ColorSkin::create(Color::PaleGoldenrod) } );
	* pEntry = pPopupOverlay;

	auto pFlex = FlexPanel::create();
	pPopupOverlay->mainSlot = pFlex;

	pFlex->setSkin( ColorSkin::create(Color::OrangeRed) );

	auto  pOpener = PopupOpener::create({ .closeOnSelect = false, .label = {.text = "OPEN POPUP"}, .skin = pButtonSkin });
	pFlex->slots.pushBack(pOpener, { .pos = {50,50}, .size = {70,40} });

	auto pOpened = PackPanel::create( { .axis = Axis::Y, .skin = pBackSkin } );
	pOpener->setPopup(pOpened);

	auto pEntry1 = Filler::create( { .defaultSize = { 100, 20 }, .skin = pButtonSkin });
	pOpened->slots << pEntry1;

	auto pEntry2 = Filler::create( { .defaultSize = { 100, 20 }, .skin = pButtonSkin });
	pOpened->slots << pEntry2;

	auto pShadowSkin = BoxSkin::create(8, Color::Transparent, Color8(0, 0, 0, 128), 8);

	auto pCombSkin = DoubleSkin::create(pButtonSkin, pShadowSkin);


	auto pSupPopup = Filler::create( { .defaultSize = { 100, 36 }, .skin = pCombSkin });

	auto pEntry3 = PopupOpener::create( { .attachPoint = Placement::East, .openOnHover = true, .popup = pSupPopup, .popupOverflow = 8, .skin = pButtonSkin });
	pOpened->slots << pEntry3;

//--------

	auto pRawFlex = pFlex.rawPtr();

	Base::msgRouter()->addRoute( pFlex, MsgType::MousePress, [pRawFlex](Msg * pMsg) {

		static HiColor color = Color::RosyBrown;
		static HiColor otherColor = Color::Cornsilk;

		pRawFlex->setSkin( ColorSkin::create(color) );

		std::swap(color,otherColor);
	} );


	return true;
}


//____ scrollbarTest() ______________________________________________________

bool scrollbarTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pButtonSkin = BoxSkin::create(1, Color::LightGrey, Color::Black, 5);


	auto pEditor = TextEditor::create();

	pEditor->editor.setText("TEXT AREA.");
	pEditor->setSkin( ColorSkin::create(Color::YellowGreen) );

	auto pScrollPanel = ScrollPanel::create();
	pScrollPanel->slot = pEditor;
	pScrollPanel->setSkin( ColorSkin::create(Color::Pink) );

	pScrollPanel->scrollbarX.setSkins(ColorSkin::create(Color::DarkGreen), pButtonSkin, pButtonSkin, pButtonSkin);
	pScrollPanel->scrollbarY.setSkins(ColorSkin::create(Color::DarkGreen), pButtonSkin, pButtonSkin, pButtonSkin);


	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pScrollPanel, { .pos = {10,10}, .size = {200,50} });

	* pEntry = pFlex;

	
	auto pButtonFlipHideX = Button::create( { .label = { .text = "FLIP HIDE X"}, .skin = pButtonSkin } );
	
	pFlex->slots.pushBack(pButtonFlipHideX, { .origo = Placement::SouthWest, .pos = {5,-5} });
	
	Base::msgRouter()->addRoute(pButtonFlipHideX, MsgType::Select, [pScrollPanel](Msg * p) { pScrollPanel->scrollbarX.setVisible(!pScrollPanel->scrollbarX.isVisible()); }   );

	auto pButtonFlipHideY = Button::create( { .label = { .text = "FLIP HIDE Y"}, .skin = pButtonSkin } );
	
	pFlex->slots.pushBack(pButtonFlipHideY, { .origo = Placement::SouthWest, .pos = {5 + 100,-5} });
	
	Base::msgRouter()->addRoute(pButtonFlipHideY, MsgType::Select, [pScrollPanel](Msg * p) { pScrollPanel->scrollbarY.setVisible(!pScrollPanel->scrollbarY.isVisible()); }   );

	
	return true;
}

//____ scrollPanelTest() ______________________________________________________

bool scrollPanelTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pButtonSkin = BoxSkin::create(1, Color::LightGrey, Color::Black, 5);


	auto pEditor = TextEditor::create();

	pEditor->editor.setText("TEXT AREA This is a long text to make the text be longer than the scrollpanel which is hopefully is by now. Or now, or now or now...");
	pEditor->setSkin(ColorSkin::create(Color::YellowGreen));

	auto pScrollPanel = ScrollPanel::create();
	pScrollPanel->slot = pEditor;
	pScrollPanel->setSkin(ColorSkin::create(Color::Pink));

	pScrollPanel->scrollbarX.setSkins(ColorSkin::create(Color::DarkGreen), pButtonSkin, pButtonSkin, pButtonSkin);
//	pScrollPanel->scrollbarY.setSkins(ColorSkin::create(Color::DarkGreen), pButtonSkin, pButtonSkin, pButtonSkin);


	auto pFlex = FlexPanel::create();
	pFlex->slots.pushBack(pScrollPanel, { .pos = {10,10}, .size = {200,50} });

	*pEntry = pFlex;


	auto pButtonMoveLeft = Button::create({ .label = {.text = "<<<"}, .skin = pButtonSkin });

	pFlex->slots.pushBack(pButtonMoveLeft, { .origo = Placement::SouthWest, .pos = {5,-5} });

	Base::msgRouter()->addRoute(pButtonMoveLeft, MsgType::Select, [pScrollPanel](Msg* p) { pScrollPanel->setViewOffset(pScrollPanel->viewOffset() + Coord{-10,0}); });

	auto pButtonMoveRight = Button::create({ .label = {.text = ">>>"}, .skin = pButtonSkin });

	pFlex->slots.pushBack(pButtonMoveRight, { .origo = Placement::SouthWest, .pos = {5 + 100,-5} });

	Base::msgRouter()->addRoute(pButtonMoveRight, MsgType::Select, [pScrollPanel](Msg* p) { pScrollPanel->setViewOffset(pScrollPanel->viewOffset() + Coord{10, 0}); });


	return true;
}


//____ modalLayerTest() ______________________________________________________

bool modalLayerTest(ComponentPtr<DynamicSlot> pEntry)
{
	lineEditorTest(pEntry);
	Widget_p p = *pEntry;

	scrollbarTest(pEntry);

	auto pLayer = ModalOverlay::create();

	pLayer->mainSlot = pEntry->widget();
//	pLayer->mainSlot = *pEntry;

	
	//	Widget_p pTemp = pRoot->child;
//	pLayer->base = pTemp;

	*pEntry = pLayer;


	pLayer->modalSlots << p;

	return true;
}

//____ splitPanelTest() ______________________________________________________

bool splitPanelTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pSplit = SplitPanel::create();

	pSplit->setHandleSkin(ColorSkin::create({ 
		.color = Color::Black, 
		.states = {{ State::Hovered, {.color = Color::Brown }},
					{ State::Pressed, {.color = Color::Red }}
		} 
	}));

	pSplit->setHandleThickness(10);

	auto pContent1 = TextEditor::create();
	pContent1->setSkin( ColorSkin::create(Color::AntiqueWhite) );
	pSplit->slots[0] = pContent1;

	
	auto pContent2 = TextEditor::create();
	pContent2->setSkin( ColorSkin::create(Color::BlanchedAlmond) );
	pSplit->slots[1] = pContent2;


	*pEntry = pSplit;

	return true;
}

//____ designLayerTest() ______________________________________________________

bool designLayerTest(ComponentPtr<DynamicSlot> pSlot)
{
//	auto pDebugOverlay = DebugOverlay::create();

	auto pFlexPanel = FlexPanel::create();
	pFlexPanel->setSkin( ColorSkin::create(Color::Thistle) );

	auto pFiller1 = Filler::create();
	pFiller1->setSkin( ColorSkin::create(Color::Azure) );
	pFiller1->setDefaultSize({ 20,20 });

	auto pFiller2 = Filler::create();
	pFiller2->setSkin( ColorSkin::create(Color::Maroon) );
	pFiller2->setDefaultSize({ 20,20 });

	auto pFiller3 = Filler::create();
	pFiller3->setSkin( ColorSkin::create(Color::CadetBlue) );
	pFiller3->setDefaultSize({ 20,20 });

	pFlexPanel->slots.pushFront(pFiller1, { .pos = {10,10}, .size = {50,50} });
	pFlexPanel->slots.pushFront(pFiller2, { .pos = {20,20}, .size = {50,50} });
	pFlexPanel->slots.pushFront(pFiller3, { .pos = {30,30}, .size = {50,50} });

//	pDebugOverlay->mainSlot = pFlexPanel;
	*pSlot = pFlexPanel;

//	pDebugOverlay->setEditMode(true);

//	*pSlot = pFlexPanel;

	return true;
}

//____ pianoKeyboardTest() ____________________________________________________

bool pianoKeyboardTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	{
		Surface_p pOddWhiteKeys = loadSurface("resources/whiteoddkeys.png" );
		Surface_p pEvenWhiteKeys = loadSurface("resources/whiteevenkeys.png" );
		Surface_p pBlackKeys = loadSurface("resources/blackkeys.png" );


		auto pSimplePiano = PianoKeyboard::create({ 
													.blackKeys		= pBlackKeys,
													.evenWhiteKeys	= pEvenWhiteKeys,
													.keyLayout		= std::bitset<7>("1101110"),
													.keyStates		= { State::Default, State::Hovered, State::Pressed },
													.nbWhiteKeys	= 7,
													.oddWhiteKeys	= pOddWhiteKeys,
													.skin			= ColorSkin::create(Color::Black, { 60,10,10,10 })
			});



/*
		auto pSimplePiano = PianoKeyboard::create();
		pSimplePiano->setSkin( ColorSkin::create(Color::Black, { 60,10,10,10 }) );
		pSimplePiano->setLayout(7, std::bitset<7>("1101110"));
		pSimplePiano->setSurfaces(pOddWhiteKeys, pEvenWhiteKeys, pBlackKeys, { State::Default, State::Hovered, State::Pressed });
*/
		pBaseLayer->slots.pushFront(pSimplePiano, { .pos = {20,20}, .size = {0,0} });

		pSimplePiano->selectKey(3);
		pSimplePiano->selectKey(6);
		pSimplePiano->selectKey(9);
	}
/*
	{
		Surface_p pOddWhiteKeys = loadSurface("resources/NisOddWhite.png", PixelFormat::BGRA_8);
		Surface_p pEvenWhiteKeys = loadSurface("resources/NisEvenWhite.png", PixelFormat::BGRA_8);
		Surface_p pBlackKeys = loadSurface("resources/NisBlack.png", PixelFormat::BGRA_8);

		auto pNisPiano = PianoKeyboard::create();
		pNisPiano->setSkin(ColorSkin::create(Color::Black, { 0,0,0,0 }));
		pNisPiano->setLayout(26, std::bitset<7>("1110110"));
		pNisPiano->setSurfaces(pOddWhiteKeys, pEvenWhiteKeys, pBlackKeys, { State::Default, State::Pressed });

		pBaseLayer->slots.pushFrontMovable(pNisPiano, { 20,220,0,0 });

		pNisPiano->selectKey(3);
		pNisPiano->selectKey(6);
		pNisPiano->selectKey(9);
	}
*/


	*pSlot = pBaseLayer;
	return true;
}


//____ pieKnobTest() ____________________________________________________

bool pieKnobTest(ComponentPtr<DynamicSlot> pSlot)
{

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pBgSkin1 = FillMeterSkin::create({ .backColor = Color::Black, .color = Color::Green, .direction = Direction::Up });
	auto pBgSkin2 = FillMeterSkin::create({ .backColor = Color::Black, .color = Color::Green, .direction = Direction::Right });


//	auto pBgSkin3 = PieMeterSkin::create(10.f / 12.f, 0.05f, 4.f / 12.f, Color::Green, Color::Red, Color::DarkGray, 0.75f, Color::Transparent, Color::Black);
	auto pBgSkin3 = PieMeterSkin::create({	
		.backColor = Color::Black,
		.emptyColor = Color::DarkGrey,
		.hubColor = Color::Transparent,
		.hubSize = 0.75f,
		.length = 4.f / 12.f * 360,
		.min = 0.05f,
		.rotation = 10.f / 12.f * 360,
		.slices = {{ 1.f, Color::Green, Color::Red }}
	});


	auto pBgSkin4 = PieMeterSkin::create({
		.backColor = Color::Black,
		.emptyColor = Color::DarkGrey,
		.hubColor = Color::Transparent,
		.hubSize = 0.75f,
		.length = 4.f / 12.f * 360,
		.min = 1 / 8.f,
		.square = true,
		.rotation = 10.f / 12.f * 360,
		.slices = { {0.4f, Color::Red, Color::Red},
					{0.4f, Color::Orange, Color::Orange},
					{1.2f,Color::Blue,Color::Green},
					{0.2f,Color::White,Color::Black} }
	});

	auto pBgSkin5 = PieMeterSkin::create({
		.backColor = Color::Black,
		.emptyColor = Color::DarkGrey,
		.hubColor = Color::Yellow,
		.hubSize = 0.25f,
		.length = 4.f / 12.f * 360,
		.min = 0,
		.movingSlices = true,
		.rotation = 10.f / 12.f * 360,
		.slices = { {0.7f,Color::DarkBlue,Color::DarkBlue},
					{0.2f,Color::Blue,Color::Blue},
					{0.1f,Color::LightBlue,Color::LightBlue}
		}
	});
/*
	auto pBgSkin6 = PieMeterSkin::create({
		.backColor = Color::Black,
		.emptyColor = Color::Transparent,
		.hubColor = Color::Yellow,
		.hubSize = 0.25f,
		.length = 8.f / 12.f * 360,
		.min = 0.2f,
		.movingSlices = true,
		.rotation = 10.f / 12.f * 360,
		.slices = { {0.7f,Color::DarkBlue,Color::DarkBlue},
					{0.2f,Color::Blue,Color::Blue},
					{0.1f,Color::LightBlue,Color::LightBlue}
		},
		.startFromCenter = true
	});
*/

	auto pBgSkin6 = PieMeterSkin::create({
		.backColor = Color::Transparent,
		.emptyColor = Color::Black,
		.hubColor = Color::Yellow,
		.hubSize = 0.87f,
		.length = 360,
		.min = 0.0f,
//		.movingSlices = false,
//		.rotation = 10.f / 12.f * 360,
		.slices = { {1.0f,Color::Red,Color::Red}
		},
		.startFromCenter = true
	});

	
	
	auto pKnob1 = Knob::create( { .skin = pBgSkin1, .value = 0.5f } );
	auto pKnob2 = Knob::create( { .dragAxis = Axis::X, .skin = pBgSkin2, .value = 0.5f });
	auto pKnob3 = Knob::create( { .dragAxis = Axis::X, .skin = pBgSkin3, .value = 0.5f });
	auto pKnob4 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin4, .value = 0.5f });
	auto pKnob5 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin5, .value = 0.5f });
	auto pKnob6 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin6, .value = 0.5f });

	pKnob6->setValue(0.0f);
	
	pBaseLayer->slots.pushBack(pKnob1, { .pos = {10, 10}, .size = {100, 100} });
	pBaseLayer->slots.pushBack(pKnob2, { .pos = {120, 10}, .size = {100, 100} });
	pBaseLayer->slots.pushBack(pKnob3, { .pos = {230, 10}, .size = {100, 100} });
	pBaseLayer->slots.pushBack(pKnob4, { .pos = {340, 10}, .size = {100, 200} });
	pBaseLayer->slots.pushBack(pKnob5, { .pos = {450, 10}, .size = {200, 100} });
	pBaseLayer->slots.pushBack(pKnob6, { .pos = {10, 220}, .size = {100, 100} });


	*pSlot = pBaseLayer;
  
	return true;
}

//____ spinKnobTest() ____________________________________________________

bool spinKnobTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSurfKnob_bg = loadSurface("resources/knob_bg.png" );
	Surface_p pSurfKnob_fg = loadSurface("resources/knob_fg.png" );

	Surface_p pSurfArrow = loadSurface("resources/dialarrow_small.png", { .sampleMethod = SampleMethod::Bilinear } );
	Surface_p pSurfClockFace = loadSurface("resources/clockface.png" );

	auto pArrowSkin = SpinMeterSkin::create({ .angleBegin = -135, .angleEnd = 135, .defaultSize = { 400,400 }, .pivot = { 0.5f, 540 / 600.f }, .placement = {0.5f,0.5f}, .surface = pSurfArrow });

	auto pKnob1 = Knob::create();
	pKnob1->setSkin( pArrowSkin );
	pKnob1->setValue(0.5f);
	pKnob1->setDragRange(500);
	/*
		auto pKnob2 = Knob::create();
		pKnob2->setSkin(pBgSkin2);
		pKnob2->setValue(0.5f);
		pKnob2->setDragAxis(Axis::X);

		auto pKnob3 = Knob::create();
		pKnob3->setSkin(pBgSkin3);
		pKnob3->setValue(0.5f);
		pKnob3->setDragAxis(Axis::Y);

		auto pKnob4 = Knob::create();
		pKnob4->setSkin(pBgSkin4);
		pKnob4->setValue(0.5f);
		pKnob4->setDragAxis(Axis::Y);
	*/

	pBaseLayer->slots.pushBack(pKnob1, { .pos = {10,10}, .size = {400,100} });
	//	pBaseLayer->slots.pushBackMovable(pKnob2, Rect(120, 10, 100, 100));
	//	pBaseLayer->slots.pushBackMovable(pKnob3, Rect(230, 10, 100, 100));
	//	pBaseLayer->slots.pushBackMovable(pKnob4, Rect(340, 10, 100, 100));
	*pSlot = pBaseLayer;
	return true;
}

//____ animKnobTest() ____________________________________________________

bool animKnobTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSurfAnim = loadSurface("resources/boxanim.png" );


	FrameMeterSkin::Blueprint bp;

	bp.surface = pSurfAnim;
	bp.size = { 64,64 };

	for (int y = 0; y < 5; y++)
	{
		bp.frames.push_back({ { pts(0),pts(y * 64) },150 });
	}
	
	bp.frames = { {{0,0}} , {{0,16}}, {{0,32}} };



	auto pAnimSkin = FrameMeterSkin::create(bp);


	auto pKnob1 = Knob::create();
	pKnob1->setSkin( pAnimSkin );
	pKnob1->setValue(0.5f);
	pKnob1->setDragRange(500);

	pBaseLayer->slots.pushBack(pKnob1, { .pos = {10,10} });
	*pSlot = pBaseLayer;
	return true;
}


//____ sliderTest() ____________________________________________________

bool sliderTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pHandleSkin = ColorSkin::create({ .color = Color8(0x7F808080), .padding = 10, .states = {{State::Hovered, {.color = Color8(0x7FA0A0A0) }}, {State::Pressed, {.color = Color8(0xFFF0F0F0)}} } });

	auto pSliderX = Slider::create();
	{
//		auto pBgSkin = FillMeterSkin::create( Direction::Right, Color::Green, Color::Green, Color::Black, Border(0,10,0,10), Border(), true );
		auto pBgSkin = FillMeterSkin::create({ .backColor = Color::Black, .color = Color::Green, .spacing = Border(0,10,0,10) } );

		pSliderX->setAxis(Axis::X);
		pSliderX->setSkin( pBgSkin );
		pSliderX->setHandleSkin( pHandleSkin );
		pSliderX->setDefaultSlideLength(100);
	}

	auto pSliderY = Slider::create();
	{
//		auto pBgSkin = FillMeterSkin::create(Direction::Up, Color::Green, Color::Green, Color::Black, Border(10, 0, 10, 0), Border(), false);
		auto pBgSkin = FillMeterSkin::create( WGBP(FillMeterSkin,
											    _.color = Color::Green,
												_.backColor = Color::Black,
												_.direction = Direction::Up,
												_.spacing = Border(10,0,10,0) ));

		pSliderY->setAxis(Axis::Y);
		pSliderY->setSkin( pBgSkin );
		pSliderY->setHandleSkin( pHandleSkin );
		pSliderY->setDefaultSlideLength(100);
	}

	pBaseLayer->slots.pushBack(pSliderY, { .pos = Coord(10, 10) });
	pBaseLayer->slots.pushBack(pSliderX, { .pos = Coord(40, 10) });
	*pSlot = pBaseLayer;
	return true;
}

//____ rangeSliderTest() ____________________________________________________

bool rangeSliderTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pHandleSkin = ColorSkin::create({ .color = Color8(0x7F808080), .padding = 10, .states = {{State::Hovered, {.color = Color8(0x7FA0A0A0) }}, {State::Pressed, {.color = Color8(0xFFF0F0F0)}} } });

	auto pSliderX = RangeSlider::create();
	{
		auto pBgSkin = FillMeterSkin::create({ .backColor = Color::Black,
										.color = Color::Green,
										.direction = Direction::Right,
										.spacing = Border(0,10,0,10) });

		pSliderX->setAxis(Axis::X);
		pSliderX->setSkin( pBgSkin );
		pSliderX->setBeginHandleSkin( pHandleSkin );
		pSliderX->setEndHandleSkin( pHandleSkin );
		pSliderX->setDefaultSlideLength(100);
	}

	auto pSliderY = RangeSlider::create();
	{
		auto pBgSkin = FillMeterSkin::create({ .backColor = Color::Black,
												.color = Color::Green,
												.direction = Direction::Up,
												.spacing = Border(10,0,10,0) });

		pSliderY->setAxis(Axis::Y);
		pSliderY->setSkin( pBgSkin );
		pSliderY->setBeginHandleSkin( pHandleSkin );
		pSliderY->setEndHandleSkin( pHandleSkin );
		pSliderY->setDefaultSlideLength(100);
	}

	pBaseLayer->slots.pushBack(pSliderY, { .pos = Coord(10, 10) });
	pBaseLayer->slots.pushBack(pSliderX, { .pos = Coord(40, 10) });
	*pSlot = pBaseLayer;
	return true;
}



//____ canvasStackTest() ____________________________________________________

bool canvasStackTest(ComponentPtr<DynamicSlot> pSlot)
{
/*
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->skin = ColorSkin::create(Color::PapayaWhip);

	auto pCanvasStack = CanvasStack::create();

	pCanvasStack->setDefaultSize({ 1280, 1600 });
	pCanvasStack->setCanvases(2);

	Surface_p pBgSurf = loadSurface("resources/parallels/background0001.png", PixelFormat::BGR_8);
	Surface_p pFgSurf = loadSurface("resources/parallels/panel0001.png", PixelFormat::BGRA_8);
	Surface_p pLedMaskSurf = loadSurface("resources/parallels/led_mask0001.png", PixelFormat::BGRA_8);

	Surface_p pLedMixTempSurf = Base::activeContext()->surfaceFactory()->createSurface(pLedMaskSurf->size());
	pLedMixTempSurf->fill(Color::Transparent);

	pCanvasStack->setCombineFunc([=](GfxDevice* pDevice, Surface* pEndCanvas, std::vector<Surface_p>& canvases)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->setBlitSource(pBgSurf);
		pDevice->blit({ 0,0 });

		//

		pDevice->setCanvas(pLedMixTempSurf, CanvasInit::Keep, false);
		pDevice->fill(Color::Transparent);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setBlitSource(canvases[1]);
		pDevice->blit({ 0,0 });
//		pDevice->fill(Color(144,196,235));

		pDevice->setBlendMode(BlendMode::Multiply);
		pDevice->setBlitSource(pLedMaskSurf);
		pDevice->blit({ 0,0 });

		pDevice->setCanvas(pEndCanvas, CanvasInit::Keep,false);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setBlitSource(pLedMixTempSurf);
		pDevice->blit({ 0,0 });

		//

		pDevice->setBlitSource(pFgSurf);
		pDevice->blit({ 0,0 });

		pDevice->setBlitSource(canvases[0]);
		pDevice->blit({ 0,0 });

	}
	);


	auto pContent = FlexPanel::create();
	pCanvasStack->slot.setWidget(pContent);


	Surface_p pHandleSurf = loadSurface("resources/parallels/fader0001.png", PixelFormat::BGRA_8);
	auto pHandleSkin = BlockSkin::create(pHandleSurf);

	auto pSliderBgSkin = FillMeterSkin::create(Direction::Up, Color(144/2, 196/2, 235/2), Color(144, 196, 235),Color::Transparent, BorderI(36,0,40,0), BorderI(),true);

	auto pRedirectedSliderBgSkin = pCanvasStack->redirectSkin(pSliderBgSkin,1);

	auto pSlider = Slider::create();
	pSlider->setAxis(Axis::Y);
	pSlider->handleSkin = pHandleSkin;
	pSlider->skin = pRedirectedSliderBgSkin;

	pContent->slots.pushFrontMovable(pSlider, { 92,854,72,333 });


	auto pFiller = Filler::create();
	pFiller->skin = ColorSkin::create(Color(144, 196, 235));

	auto pRedirectedFiller = pCanvasStack->redirectWidget(pFiller, 1);
	pContent->slots.pushFrontMovable(pRedirectedFiller, { 625,270,330,330 });


	pBaseLayer->slots.pushBackMovable(pCanvasStack, Coord(10, 10));
	*pSlot = pBaseLayer;
*/
	return true;
}


//____ doubleSkinTest() ____________________________________________________

bool doubleSkinTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );
/*
	Surface_p pSurfKnob_bg = loadSurface("resources/knob_bg.png");
	Surface_p pSurfKnob_fg = loadSurface("resources/knob_fg.png");

	Surface_p pSurfArrow = loadSurface("resources/dialarrow_small.png", { .sampleMethod = SampleMethod::Bilinear });
	Surface_p pSurfClockFace = loadSurface("resources/clockface.png");

	auto pArrowSkin = SpinMeterSkin::create({ .angleBegin = -135, .angleEnd = 135, .defaultSize = { 400,400 }, .pivot = { 0.5f, 540 / 600.f }, .placement = { 0.5f,0.5f }, .surface = pSurfArrow });

	auto pClockFaceSkin = BlockSkin::create(pSurfClockFace);

	auto pFillMeterSkin = FillMeterSkin::create({ .color = Color::Green, .direction = Direction::Up });

	auto pDoubleSkin = DoubleSkin::create(pArrowSkin, pClockFaceSkin);

	auto pKnob1 = Knob::create();
	pKnob1->setSkin( pDoubleSkin );
	pKnob1->setValue(0.5f);
	pKnob1->setDragRange(500);

	pBaseLayer->slots.pushBack(pKnob1, { .pos = {10, 10}, .size = {400, 400} });
*/

	auto pShadowSurface = Base::defaultSurfaceFactory()->createSurface({ .size = {50,50} });
	pShadowSurface->fill(Color::Purple);

	auto pShadow = BlockSkin::create({ .overflow = 20, .surface = pShadowSurface });

//	auto pShadow = ColorSkin::create({ .color = Color::Black, .overflow = 20 });

	auto pForeground = ColorSkin::create({ .color = Color::White, .padding = 5 });

	auto pDoubleSkin = DoubleSkin::create(pForeground, pShadow);

	auto pTextEditor = TextEditor::create({ .skin = pDoubleSkin });

	pTextEditor->editor.setText("HEJ");

	pBaseLayer->slots.pushBack(pTextEditor, { .pos = {30, 30} });

	*pSlot = pBaseLayer;
	return true;
}

//____ timerTest() ____________________________________________________

bool timerTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

//	Surface_p pSurfClockFace = loadSurface("resources/clockface.png", PixelFormat::BGRA_8);
//	pSurfClockFace->setSampleMethod(SampleMethod::Bilinear);
//	auto pSkin = SpinMeterSkin::create(pSurfClockFace, { 419,419 } );

	auto pSkin = FillMeterSkin::create({});

	auto pTimer1 = Timer::create();
	pTimer1->setSkin( pSkin );
	pTimer1->setDuration(5000);
	pTimer1->setPlayMode(PlayMode::BackwardPingPong);
	pTimer1->start();

//	pBaseLayer->slots.pushBack(pTimer1, { .pos = {10,10}, .size = {419,419} });
	pBaseLayer->slots.pushBack(pTimer1, { .pos = {10,10}, .size = {419,50} });

	*pSlot = pBaseLayer;
	return true;
}

//____ animPlayerTest() ____________________________________________________

bool animPlayerTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSurfAnim = loadSurface("resources/giraffe-anim-1024x1024.jpg");


	auto pPlayer = AnimPlayer::create();
	pPlayer->frames.setSurface(pSurfAnim);
	pPlayer->frames.setFrameSize({ 204,180 });
	pPlayer->setPlayMode(PlayMode::Looping);

	for (int y = 0; y < 2; y++)
	{
		for (int x = 0; x < 5; x++)
		{
			pPlayer->frames.pushBack({ .source = {pts(x * 204),pts(y * 180)}, .duration = 150 });
		}
	}

	pPlayer->play();

	pBaseLayer->slots.pushBack(pPlayer, { .pos = Coord(10, 10) });
	*pSlot = pBaseLayer;
	return true;
}

//____ selectBoxTest() ____________________________________________________

bool selectBoxTest(ComponentPtr<DynamicSlot> pSlot)
{
	PopupOverlay_p pPopupOverlay = PopupOverlay::create();

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pMainSurf = loadSurface("resources/selectbox.png");

//	auto pMainSkin = BlockSkin::create(pMainSurf, { 0,0,32,16 }, { State::Default, State::Hovered, State::Pressed }, { 3,23,3,3 });
	auto pMainSkin = BlockSkin::create(BlockSkin::Blueprint{ .firstBlock = { 0,0,32,16 }, .frame = { 3,23,3,3 }, .padding = { 3,23,3,3 }, .states = { State::Default, {}, State::Hovered, {}, State::Pressed }, .surface = pMainSurf
});


	
	auto pSelectBox = SelectBox::create();
	pSelectBox->setSkin( pMainSkin );

	auto pListSkin = BoxSkin::create(1, Color::White, Color::Black, 3);

	auto pListEntrySkin = BoxSkin::create({ .color = Color::Transparent, .outlineColor = Color::Transparent, .outlineThickness = 1, .padding = 3,
											 .states = {{State::Hovered, {.color = Color::Yellow, .outlineColor = Color::Orange }},
														{State::Selected, {.color = Color::LightBlue, .outlineColor = Color::White }} } });


	pSelectBox->setListSkin(pListSkin);
	pSelectBox->setEntrySkin(pListEntrySkin);

	pSelectBox->entries.pushBack({ .id = 1, .text = "ENTRY 1" });
	pSelectBox->entries.pushBack({ .id = 2, .text = "ENTRY 2" });

	pBaseLayer->slots.pushBack(pSelectBox, { .pos = Coord(10, 10) });
	pPopupOverlay->mainSlot = pBaseLayer;

	*pSlot = pPopupOverlay;
	return true;
}

//____ tileSkinTest() ____________________________________________________

bool tileSkinTest(ComponentPtr<DynamicSlot> pSlot)
{

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pNormalSurf = loadSurface("resources/tile_blocks.png", { .tiling = true } );

	Surface_p pHoveredSurf = loadSurface("resources/tile_blocks_marked.png", { .tiling = true } );

	auto pStaticSkin = TileSkin::create({ .padding = 10, .surface = pNormalSurf });

	auto pDynamicSkin = TileSkin::create( { .padding = 10, .states = { {State::Hovered, {.surface = pHoveredSurf} } }, .surface = pNormalSurf  });

	auto pFiller1 = Filler::create();
	pFiller1->setSkin( pStaticSkin );

	auto pFiller2 = Filler::create();
	pFiller2->setSkin( pDynamicSkin );




	pBaseLayer->slots.pushBack(pFiller1, { .pos = {10, 10}, .size = {100, 100} });
	pBaseLayer->slots.pushBack(pFiller2, { .pos = {10, 120}, .size = {400, 100} });

	*pSlot = pBaseLayer;
	return true;
}

//____ gradientSkinTest() ____________________________________________________

bool gradientSkinTest(ComponentPtr<DynamicSlot> pSlot)
{
/*
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pSkin1 = GradientSkin::create(wg::Gradient(wg::Placement::South, wg::HiColor(0.0f, 0.0f, 0.0f, 0.0f), wg::HiColor(0.0f, 0.0f, 0.0f, 0.2f)));

	auto pSkin2 = GradientSkin::create(wg::Gradient(wg::Placement::South, wg::HiColor(0.5f, 0.0f, 0.0f, 1.0f), wg::HiColor(0.0f, 0.0f, 0.0f, 1.0f)));


	auto pFiller1 = Filler::create();
	pFiller1->setSkin(pSkin1);

	auto pFiller2 = Filler::create();
	pFiller2->setSkin(pSkin2);

	pBaseLayer->slots.pushBack(pFiller1, { .pos = {10, 10}, .size = {100, 100} });
	pBaseLayer->slots.pushBack(pFiller2, { .pos = {10, 120}, .size = {400, 100} });

	*pSlot = pBaseLayer;

*/
	return true;
}



//____ bakeSkinTest() ____________________________________________________

bool bakeSkinTest(ComponentPtr<DynamicSlot> pSlot)
{

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pNormalSurf = loadSurface("resources/tile_blocks.png", { .tiling = true } );

	Surface_p pHoveredSurf = loadSurface("resources/tile_blocks_marked.png", { .tiling = true } );

	auto pDynamicSkin = TileSkin::create({ .padding = 10, .states = {{ State::Hovered, {.surface = pHoveredSurf} }}, .surface = pNormalSurf });


	Surface_p pSplashSurf = loadSurface("resources/splash.png" );

	auto pSplashSkin = BlockSkin::create({ .blendMode = BlendMode::Add, .surface = pSplashSurf });

	Surface_p pBakeSurface = Base::defaultSurfaceFactory()->createSurface({ .size = SizeI(512, 512) });

	auto pBakedSkin = BakeSkin::create({ .skins = { pSplashSkin, pDynamicSkin }, .surface = pBakeSurface });

	auto pFiller1 = Filler::create();
	pFiller1->setSkin( pBakedSkin );





	pBaseLayer->slots.pushBack(pFiller1, { .pos = {10,10}, .size = {100,100} });

	*pSlot = pBaseLayer;
	return true;
}

//____ animSkinTest() ____________________________________________________

bool animSkinTest(ComponentPtr<DynamicSlot> pSlot)
{

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSplashSurf = loadSurface("resources/splash.png");

	auto pTransition = ValueTransition::create( 500000, TransitionCurve::EaseIn );
	auto pReturnTransition = ValueTransition::create( 500000, TransitionCurve::EaseOut );

	auto pAnimSkin = SpinAnimSkin::create({
		.angleBegin = 0.f,
		.angleEnd = 360.f,
		.defaultSize = { 256,256 },
		.padding = 80,
//		.returnTransition = pReturnTransition,
		.surface = pSplashSurf,
//		.transition = pTransition
	});

	auto pAnimSkin2 = SpinAnimSkin::create({ .defaultSize = { 256,256 }, .padding = 10, .surface = pSplashSurf });

	auto pBoxSkin = BoxSkin::create(2, Color::Red, Color::Black);

	auto pFramingSkin = BoxSkin::create(2, Color::Transparent, Color::Black, 80);

	auto pDoubleSkin = DoubleSkin::create(pAnimSkin2, pFramingSkin, true);

	auto pBakeSurface = Base::defaultSurfaceFactory()->createSurface({ .size = SizeI(512, 512) });

	auto pBakeSkin = BakeSkin::create({ .skinInSkin = true, .skins = {pAnimSkin2, pBoxSkin,pAnimSkin}, .surface = pBakeSurface });

	auto pFiller1 = Filler::create();
	pFiller1->setSkin( pBakeSkin );

	pBaseLayer->slots.pushBack(pFiller1, { .pos = {10,10}, .size = {256, 256} });

	*pSlot = pBaseLayer;
	return true;
}

//____ renderLayerTest() ____________________________________________________

bool renderLayerTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::SlateGrey) );

	auto pBoxSkin = BoxSkin::create({ .color = Color::Green, .layer = 2, .outlineColor = Color::Black, .outlineThickness = 2 });
	
	auto pShadowSkin = BoxSkin::create({	.blendMode = BlendMode::Max,
											.color = Color(255,255,255,32),
											.layer = 1,
											.outlineColor = Color::Transparent,
											.outlineThickness = 20,
											.padding = { 0,40,40,0},
										} );

	auto pDaSkin = DoubleSkin::create({ .skinInSkin = true, .skins = { pBoxSkin, pShadowSkin } } );

	for (int i = 0; i < 5; i++)
	{
		auto pWidget = Filler::create();
		pWidget->setSkin( pDaSkin );

		pBaseLayer->slots.pushBack(pWidget, { .pos = {i * 30.f, i * 30.f}, .size = {100, 100} });

		Base::msgRouter()->addRoute(pWidget, MsgType::MouseDrag, [pBaseLayer, i](Msg* pMsg) { pBaseLayer->slots[i].move(static_cast<MouseDragMsg*>(pMsg)->draggedNow()); });
	}

	*pSlot = pBaseLayer;
	return true;
}

//____ rigidPartNinePatchTest() ________________________________________________

bool rigidPartNinePatchTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSurf = loadSurface("resources/fixed_sections_alphatest.png");
/*
	auto pSkin = BlockSkin::create(pSurf, {State::Default, State::Hovered}, 10, Axis::X);
	pSkin->setRigidPartX(24, 16, YSections::Top | YSections::Center | YSections::Bottom);
	pSkin->setRigidPartY(24, 16, XSections::Left | XSections::Center | XSections::Right);

*/
	auto pSkin = BlockSkin::create({
								.axis = Axis::X,
								.frame = 10,
								.rigidPartX = { 24, 16, YSections::Top | YSections::Center | YSections::Bottom },
								.rigidPartY = { 24, 16,  XSections::Left | XSections::Center | XSections::Right },
								.states = { State::Hovered, {} },
								.surface = pSurf
		});


	auto pWidget = Filler::create();
	pWidget->setSkin( pSkin );

	pBaseLayer->slots.pushBack(pWidget, { .pos = {10, 10}, .size = {256, 256} });

	Base::msgRouter()->addRoute(pWidget, MsgType::MouseDrag, [pBaseLayer](Msg* pMsg) { pBaseLayer->slots[0].setSize(pBaseLayer->slots[0].size() + Size(static_cast<MouseDragMsg*>(pMsg)->draggedNow())); });

	*pSlot = pBaseLayer;
	return true;
}

//____ scrollSkinTest() ________________________________________________

bool scrollSkinTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	Surface_p pSliderSurf = loadSurface("resources/sliding_statebutton.png");
	Surface_p pCoverSurf = loadSurface("resources/sliding_statebutton_cover.png");

	auto pSliderSkin = BlockSlideSkin::create({
											.blockLength = 84,
											.blockSpacing = 0,
											.slideDirection = Direction::Left,
											.slideDuration = 100,
											.slideState = PrimState::Checked,
											.states = { State::Default, {}, State::Hovered, {}, State::Disabled, {} },
											.surface = pSliderSurf });

	auto pCoverSkin = BlockSkin::create(pCoverSurf);

	auto pComboSkin = DoubleSkin::create(pCoverSkin, pSliderSkin, false);

	auto pWidget = ToggleButton::create();
	pWidget->setSkin( pComboSkin );

	pBaseLayer->slots.pushBack(pWidget, { .pos = {10, 10}, .size = {84, 36} });

	*pSlot = pBaseLayer;
	return true;
}

//____ tooltipLayerTest() _____________________________________________________

TextDisplay_p	s_pTooltip;

bool tooltipLayerTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pTooltipOverlay = TooltipOverlay::create();

	s_pTooltip = TextDisplay::create();

	Surface_p pTooltipBg = loadSurface("resources/tooltip_under_bg.png");
	auto pSkin = BlockSkin::create({
		.frame		= Border(10,4,3,4),
		.padding	= { 10,4,4,4 },
		.rigidPartX	= {5, 16, YSections::Top | YSections::Center | YSections::Bottom},
		.surface	= pTooltipBg,
		});
		
	s_pTooltip->setSkin( pSkin );


	pTooltipOverlay->setTooltipGenerator([](TooltipOverlay::Position& position, const Widget* pHoveredWidget, const Border& widgetMargins) 
	{
		position.bAroundPointer = false;
		position.placement = Placement::South;
		position.spacing = Border(0);

		s_pTooltip->display.setText(pHoveredWidget->tooltip());
		return s_pTooltip;
	});

	auto pTooltipSkin = BoxSkin::create(1, Color::White, Color::Black, 2);

	auto pTooltip = TextDisplay::create();
	pTooltip->setSkin( pTooltipSkin );

	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pWidget1 = Filler::create();
	pWidget1->setSkin( ColorSkin::create(Color::Blue) );
	pWidget1->setTooltip( String("This is widget 1.") );
	pBaseLayer->slots.pushBack(pWidget1, { .pos = {20, 20}, .size = {100, 100} });

	auto pWidget2 = Filler::create();
	pWidget2->setSkin( ColorSkin::create(Color::Green) );
	pWidget2->setTooltip(String("This is widget 2."));
	pBaseLayer->slots.pushBack(pWidget2, { .pos = {150, 20}, .size = {100, 100} });

	pTooltipOverlay->mainSlot = pBaseLayer;
	*pSlot = pTooltipOverlay;
	return true;
}

//____ tooltipLayerTest() _____________________________________________________

bool kerningTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pDisplay1 = TextDisplay::create();

	TextStyle::Blueprint bp; // = Base::defaultStyle()->blueprint();
	bp.size = 32;

	auto pBigStyle = TextStyle::create(bp);

	pDisplay1->display.setText("HHTATHHWAW");
	pDisplay1->display.setStyle(pBigStyle);

	pBaseLayer->slots.pushBack(pDisplay1, { .pos = {10,10}, .size = {300,100} });

	*pSlot = pBaseLayer;
	return true;
}

//____ circleSkinTest() ________________________________________________________

bool circleSkinTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin( ColorSkin::create(Color::PapayaWhip) );

	auto pWidget = Filler::create();

	auto pSkin = CircleSkin::create({ .color = Color8::Green,
										.outlineThickness = 1.f,
										.size = 1.f,
										.states = { {State::Hovered, {.color = Color8::Yellow, .size = 1.f, .thickness = 15.f }},
													{State::Pressed, {.color = Color8::Red, .size = 0.8f, .thickness = 100.f } } },
										.thickness = 5.f });

	pWidget->setSkin( pSkin );


	pBaseLayer->slots.pushBack(pWidget, { .pos = {10, 10}, .size = {300, 100} });


	*pSlot = pBaseLayer;
	return true;

}

//____ packPanelTest() ________________________________________________________

bool packPanelTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pPackPanel = PackPanel::create();
	pPackPanel->setSkin(ColorSkin::create(Color::Azure));
	pPackPanel->setAxis(Axis::X);
	pPackPanel->setLayout(PackLayout::create({}));

	auto pText1 = TextDisplay::create();
	pText1->display.setText("TEXT1");
	pPackPanel->slots << pText1;

	auto pText2 = TextDisplay::create();
	pText2->display.setText("TEXT2");
	pPackPanel->slots << pText2;

	auto pText3 = TextDisplay::create();
	pText3->display.setText("TEXT3");
	pPackPanel->slots << pText3;

	pBaseLayer->slots.pushBack(pPackPanel);
	*pSlot = pBaseLayer;
	return true;

}

//____ memHeapFragmentationTest() ________________________________________________________

bool memHeapFragmentationTest(ComponentPtr<DynamicSlot> pSlot)
{
/*
	Base::setErrorHandler( [](Error& error)  {
		
		int x = 0;
		
	});
	
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	int x = rand();
	
	//---

	int heapSize = 1024*1024;
	
	auto * pBuffer = new char[heapSize];

	MemHeap heap(pBuffer,heapSize);
		
	heap.setDebugLevel(3);

	void * areas[100];

	// Randomly allocate some areas.
	
	for( int i = 0 ; i < 100 ; i++ )
	{
		int size = rand() % 8000;
		areas[i] = heap.malloc(size);
	}
	
	// Randomly free and reallocate areas
	
	for( int i = 0 ; i < 100000 ; i++ )
	{
		int entry = rand() % 100;

		heap.free(areas[entry]);
		
		areas[entry] = heap.malloc( rand() % 6000 );
	}
	
	
	auto pSurface = Base::activeContext()->surfaceFactory()->createSurface( { .format = PixelFormat::RGB_555_bigendian, .size = SizeI(512,512) } );
	
	uint16_t sectionTable[1024*4];
	
	heap.drawFragmentMap(1024*4, sectionTable, pSurface);
	
	
	// Free all areas.
	
	for( int i = 0 ; i < 100 ; i++ )
	{
		heap.free(areas[i]);
	}

	delete [] pBuffer;
	
	Base::setErrorHandler( nullptr );
	
	auto pImage = Image::create( { .image = pSurface } );
//	pImage->setImage(pSurface);

	pBaseLayer->slots.pushBack(pImage);
	*pSlot = pBaseLayer;
	return true;
*/
	return false;
}



//____ glyphAsSurfaceTest() ________________________________________________________

bool glyphAsSurfaceTest(ComponentPtr<DynamicSlot> pSlot, Font_p pFont )
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::Red));

	pFont->setSize(80*64);
	
	auto pSurface = pFont->getGlyphAsSurface('Q');
	
	auto pImage = Image::create( { .image = pSurface } );

	pBaseLayer->slots.pushBack(pImage);

	*pSlot = pBaseLayer;
	return true;
}

//____

void nisBlendTest()
{
/*
	SoftSurface_p pCanvas = SoftSurface::create({ 256,256 }, PixelFormat::BGR_8);
	SurfaceFactory_p pFactory = SoftSurfaceFactory::create();
	Surface_p pBackImg = loadSurface("../alpha_blending_example_files/knob_background.png", PixelFormat::BGRA_8);
	Surface_p pFrontImg = loadSurface("../alpha_blending_example_files/knob_foreground_linear.png", PixelFormat::BGRA_8);
	GfxDevice_p pDevice = SoftGfxDevice::create(pCanvas);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->beginRender();
	pDevice->setBlitSource(pBackImg);
	pDevice->blit({ 0,0 });
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->setBlitSource(pFrontImg);
	pDevice->blit({ 0,0 });
	pDevice->endRender();
	savePNG(pCanvas, "knob_wg_software.png");
*/ 
}


void commonAncestorTest()
{
	auto pBrother = PackPanel::create();

	auto pMe = PackPanel::create();
	auto pParent = PackPanel::create();
	auto pUncle = PackPanel::create();

	auto pGranny = PackPanel::create();

	auto pGreatGranny = PackPanel::create();

	auto pNephew = PackPanel::create();
	auto pCousine = PackPanel::create();

	auto pUnrelated = PackPanel::create();
	auto pUnrelatedsChild = PackPanel::create();
	pUnrelated->slots << pUnrelatedsChild;

	pGreatGranny->slots << pGranny;
	pGranny->slots << pParent;
	pGranny->slots << pUncle;

	pUncle->slots << pCousine;

	pParent->slots << pMe;
	pParent->slots << pBrother;
	
	pBrother->slots << pNephew;

	assert(pMe->commonAncestor(pBrother) == pParent);
	assert(pMe->commonAncestor(pNephew) == pParent);
	assert(pMe->commonAncestor(pMe) == pMe);
	assert(pMe->commonAncestor(pParent) == pParent);
	assert(pParent->commonAncestor(pMe) == pParent);

	assert(pParent->commonAncestor(pMe) == pParent);

	assert(pMe->commonAncestor(pCousine) == pGranny);
	assert(pMe->commonAncestor(pUncle) == pGranny);

	assert(pMe->commonAncestor(pGranny) == pGranny);
	assert(pMe->commonAncestor(pGreatGranny) == pGreatGranny);
	assert(pGreatGranny->commonAncestor(pGreatGranny) == pGreatGranny);

	assert(pMe->commonAncestor(nullptr) == nullptr);

	assert(pMe->commonAncestor(pUnrelated) == nullptr);
	assert(pMe->commonAncestor(pUnrelatedsChild) == nullptr);

}

void textStyleTest()
{
/*
	auto pBase = TextStyle::create();
	auto pAdded = TextStyle::create();

	assert(pBase->bgColor(State::Selected) == Color::Transparent);
	assert(pBase->color(State::Hovered) == Color::Black);

	pBase->setColor(Color::Red, State::Hovered);
	assert(pBase->color(State::Default) == Color::Black);
	assert(pBase->color(State::Hovered) == Color::Red);
	assert(pBase->color(State::Pressed) == Color::Red);
	assert(pBase->color(State::SelectedHoveredFocused) == Color::Red);
	assert(pBase->color(State::SelectedFocused) == Color::Black);

	pBase->setSize(16, State::Hovered);
	pAdded->setSize(15);

	TextAttr attr;
	
	pBase->exportAttr(State::Hovered, &attr, 64);
	assert(attr.size == 16*64);

	pAdded->exportAttr(State::Hovered, &attr, 64);
	assert(attr.size == 15*64);
*/

}

void unitTestMemHeap()
{
	Base::setErrorHandler( [](Error& error)  {
		
		int x = 0;
		
	});
	
	
	int heapSize = 1024*1024;
      	auto * pBuffer = new char[heapSize];

        {
	  MemHeap heap(pBuffer, heapSize );
          heap.setDebugLevel(3);

	  void * areas[100];

	  // Randomly allocate some areas.
	  
	  for( int i = 0 ; i < 100 ; i++ )
	  {
		  int size = rand() % 8000;
		  areas[i] = heap.malloc(size);
	  }
	  
	  // Randomly free and reallocate areas
	  
	  for( int i = 0 ; i < 100000 ; i++ )
	  {
		  int entry = rand() % 100;

		  heap.free(areas[entry]);
		  
		  areas[entry] = heap.malloc( rand() % 6000 );
	  }
	  
	  // Free all areas.
	  
	  for( int i = 0 ; i < 100 ; i++ )
	  {
		  heap.free(areas[i]);
	  }
	}
	delete [] pBuffer;
}


//____ blendRGB565BigendianTest() ______________________________________________________

bool blendRGB565BigendianTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pCanvas = CanvasCapsule::create( WGBP(CanvasCapsule,
															_.pixelFormat = PixelFormat::RGB_565_bigendian ) );

	auto pHSplit = PackPanel::create();
	pCanvas->slot = pHSplit;

	auto pTextEditor = TextEditor::create( WGBP(TextEditor,
		_.skin = ColorSkin::create( Color8::Yellow, 5 ) ));
	pTextEditor->editor.setText("TESTING");
	pHSplit->slots << pTextEditor;


	Surface_p pSplashSurf = loadSurface("resources/splash_opaque.png");

	auto pImage = Image::create( WGBP(Image,
		_.image = pSplashSurf));
	pHSplit->slots << pImage;

	*pEntry = pCanvas;

	return true;
}

//____ twoSlotPanelTest() ______________________________________________________

bool twoSlotPanelTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pWrapTextLayout = BasicTextLayout::create( { .wrap = true });

	auto pPanel = TwoSlotPanel::create();
	pPanel->setAxis(Axis::X);

	auto pContent1 = TextEditor::create({ .editor = {.layout = pWrapTextLayout } } );
	pContent1->setSkin(ColorSkin::create(Color::Red));
	pPanel->slots[0] = pContent1;

	auto pContent2 = TextEditor::create({ .editor = {.layout = pWrapTextLayout } });
	pContent2->setSkin(ColorSkin::create(Color::Green));
	pPanel->slots[1] = pContent2;


	auto pLayout = PackLayout::create( { .expandFactor = PackLayout::Factor::One, .shrinkFactor = PackLayout::Factor::Weight } );
	pPanel->setLayout(pLayout);

	*pEntry = pPanel;

	return true;
}

//____ customSkinTest() ______________________________________________________

bool customSkinTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pFlex = FlexPanel::create();
	
	pFlex->setSkin(ColorSkin::create(Color::LightYellow));
	
	auto pFiller = Filler::create();
/*
	auto pLambdaSkin = LambdaSkin::create( WGBP(LambdaSkin,
												_.renderFunc = [](GfxDevice* pDevice,const RectSPX& canvas, int scale, State state, float value1, float value2)
												{
		pDevice->fill( canvas, HiColor::White );
		pDevice->drawElipse(canvas, 64*4, Color8::Green, 64, Color8::Black );
		
												} ));
*/

	auto pLambdaSkin = LambdaSkin::create({
		.ignoreState = true,
		.markTestFunc = [](const CoordSPX& pos, const RectSPX& canvas, int scale, State state, float value1, float value2, int minAlpha )
		{
			RectSPX canv = canvas;
			canv.w = canvas.w * value1;
			return canv.contains(pos);
		},
		
		.renderFunc = [](GfxDevice* pDevice,const RectSPX& canvas,
						 int scale, State state, float value1, float value2)
		{
			RectSPX canv = canvas;
			canv.w = canvas.w * value1;
			pDevice->fill( canv, Color::Green );
		}
	});


	auto pKnob = Knob::create( { .skin = pLambdaSkin } );
	
	pKnob->setValue(1.f);
	
//	pFiller->setSkin( pLambdaSkin );

//	pFlex->slots.pushBack(pFiller, { .pos = {10,10}, .size = {200,200} });
	pFlex->slots.pushBack(pKnob, { .pos = {10,10}, .size = {200,200} });

	*pEntry = pFlex;

	return true;
}

//____ areaChartTest() ______________________________________________________

bool areaChartTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pFlex = FlexPanel::create();

	pFlex->setSkin(ColorSkin::create(Color::LightYellow));

	auto pGraph = AreaChart::create(WGBP(AreaChart,
		_.displayCeiling = 0.5f,
		_.displayFloor = - 0.5f,
		_.displaySkin = BoxSkin::create( WGBP(BoxSkin,
										_.color = Color::Black,
										_.outlineColor = Color::Green,
										_.padding = 2,
										_.outlineThickness = 2 )),
		_.skin = ColorSkin::create(Color::Pink)
	));




	pFlex->slots.pushBack(pGraph, { .pos = {10,10}, .size = {200,200} });

	*pEntry = pFlex;


	pGraph->entries.pushBack( {
		.bottomOutlineThickness = 0,
		.outlineColor = Color::Red,
		.topOutlineThickness = 5
		});


	static float topSamples[5] = { 0, -0.25f, 0.25f, 0.23f, 0.5f };

	static float topSamples2[5] = { 0, 0.25f, -0.25f, -0.23f, -0.5f };
	static float bottomSamples2[1] = { 0.f };


	pGraph->entries.back().setTopSamples(5, topSamples);

	auto pTransition = ValueTransition::create(2000000, TransitionCurve::Bezier);

	pGraph->entries.back().transitionSamples(pTransition, 5, topSamples2, 1, bottomSamples2);


	static float curveSamples[6][201];

	static float beg[1] = { 0.5f };
	static float end[1] = { -0.5f };

	auto pLinear = ValueTransition::create(201, TransitionCurve::Linear);
	auto pEaseIn = ValueTransition::create(201, TransitionCurve::EaseIn);
	auto pEaseOut = ValueTransition::create(201, TransitionCurve::EaseOut);
	auto pEaseInOut = ValueTransition::create(201, TransitionCurve::EaseInOut);
	auto pBezier = ValueTransition::create(201, TransitionCurve::Bezier);
	auto pParametric = ValueTransition::create(201, TransitionCurve::Parametric);

	for (int i = 0 ; i < 201 ; i++ )
	{
		pLinear->snapshot(i, 1, beg, end, &curveSamples[0][i]);
		pEaseIn->snapshot(i, 1, beg, end, &curveSamples[1][i]);
		pEaseOut->snapshot(i, 1, beg, end, &curveSamples[2][i]);
		pEaseInOut->snapshot(i, 1, beg, end, &curveSamples[3][i]);
		pBezier->snapshot(i, 1, beg, end, &curveSamples[4][i]);
		pParametric->snapshot(i, 1, beg, end, &curveSamples[5][i]);
	}


	Color colors[6] = { Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Pink, Color::Brown };

	for (int i = 0; i < 6; i++)
	{
		pGraph->entries.pushBack({
			.bottomOutlineThickness = 0,
			.color = Color::Transparent,
			.outlineColor = colors[i],
			.topOutlineThickness = 2,
			});

		pGraph->entries.back().setTopSamples(201, curveSamples[i] );
	}

	// Setup grid

	pGraph->xLines.pushBack({ .label = "-0.5", .pos = -0.5f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "-0.25", .pos = -0.25f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 1.f });
	pGraph->xLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });

	pGraph->yLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.5", .pos = 0.5f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "1.0", .pos = 1.f, .thickness = 0.5f });

/*
	pGraph->xLines.pushBack(WGBP(GridLine,
							_.value = -0.25f
							));
*/

	//

	pGraph->glow.setActive(true);


	return true;
}

//____ areaChartTest2() ______________________________________________________

bool areaChartTest2(ComponentPtr<DynamicSlot> pEntry)
{
	auto pFlex = FlexPanel::create();

	pFlex->setSkin(ColorSkin::create(Color::LightYellow));

	auto pGraph = AreaChart::create(WGBP(AreaChart,
		_.displayCeiling = 0.5f,
		_.displayFloor = -0.5f,
		_.displaySkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::White,
			_.outlineColor = Color::Green,
			_.padding = 2,
			_.outlineThickness = 2)),
		_.skin = ColorSkin::create(Color::Pink)
	));




	pFlex->slots.pushBack(pGraph, { .pos = {10,10}, .size = {200,200} });

	*pEntry = pFlex;


	pGraph->entries.pushBack({
		.bottomOutlineThickness = 0,
		.color = Color::Transparent,
		/*.flip = GfxFlip::Rot270,*/
		.outlineColor = Color::Red,
		.topOutlineThickness = 5,
		});


	static float topSamples[2][5] = { 0, -0.25f, 0.25f, 0.23f, 0.5f,
									  0, 0.25f, -0.25f, -0.23f, -0.5f };

	static float bottomSamples[1] = { 0.f };

	static int transitionIndex = 0;

	pGraph->entries.back().setTopSamples(5, topSamples[0]);





	Color colors[6] = { Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Pink, Color::Brown };

	// Setup grid

	pGraph->xLines.pushBack({ .label = "-0.5", .labelAtEnd = true, .pos = -0.5f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "-0.25", .pos = -0.25f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 1.f });
	pGraph->xLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });

	pGraph->yLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.5", .pos = 0.5f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "1.0", .labelAtEnd = true, .pos = 1.f, .thickness = 0.5f });

	/* 
		pGraph->xLines.pushBack(WGBP(GridLine,
								_.value = -0.25f
								));
	*/

	//


	pGraph->glow.setActive(true);


	auto pTransition = ValueTransition::create(2000000, TransitionCurve::Bezier);
	pGraph->entries.back().transitionSamples(pTransition, 5, topSamples[1], 1, bottomSamples);



	auto pButtonSkin = BoxSkin::create({ .color = Color8::Grey,
									  .outlineColor = Color8::Black,
									  .outlineThickness = 1,
									  .padding = 3
		});


	auto pButton = Button::create({ .label = {.text = "TRANSITION"}, .skin = pButtonSkin });

	Base::msgRouter()->addRoute(pButton, MsgType::Select, [pGraph, pTransition](Msg* pMsg) 
	{ 
		transitionIndex = (transitionIndex + 1) % 2;
		pGraph->entries.back().transitionSamples(pTransition, 5, topSamples[transitionIndex], 1, bottomSamples);  
	});


	pFlex->slots.pushBack(pButton, { .pos = {105, 220 } });

	//---

	auto pButton2 = Button::create({ .label = {.text = "RESIZE OUTLINE"}, .skin = pButtonSkin });

	Base::msgRouter()->addRoute(pButton2, MsgType::Select, [pGraph](Msg* pMsg)
	{
		auto& entry = pGraph->entries.back();

		pts top = entry.topOutlineThickness();

		if( top == 1.f )
			top = 5.f;
		else
			top = 1.f;

		entry.setOutlineThickness(top, 0.f);
	});

	pFlex->slots.pushBack(pButton2, { .pos = {205, 220 } });

	//---

	auto pButton3 = Button::create({ .label = {.text = "RESIZE RANGE"}, .skin = pButtonSkin });

	Base::msgRouter()->addRoute(pButton3, MsgType::Select, [pGraph](Msg* pMsg)
	{
		static bool bExpanded = false;

		if( bExpanded )
		{
			pGraph->setDisplayRange(0.5f, -0.5f, ValueTransition::create( 400000 ) );
			bExpanded = false;
		}
		else
		{
			pGraph->setDisplayRange(0.3f, -0.3f, ValueTransition::create( 400000 ) );
			bExpanded = true;
		}
	});



	pFlex->slots.pushBack(pButton3, { .pos = {105, 250 } });
	return true;
}

//____ plotChartTest() ______________________________________________________

bool plotChartTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pFlex = FlexPanel::create();

	pFlex->setSkin(ColorSkin::create(Color::LightYellow));

	auto pGraph = PlotChart::create(WGBP(PlotChart,
		_.displayCeiling = 0.5f,
		_.displayFloor = -0.5f,
		_.displaySkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::Black,
			_.outlineColor = Color::Green,
			_.padding = 2,
			_.outlineThickness = 2)),
		_.skin = ColorSkin::create(Color::Pink),
		_.glow.active = true
	));




	pFlex->slots.pushBack(pGraph, { .pos = {10,10}, .size = {200,200} });

	*pEntry = pFlex;


	pGraph->entries.pushBack({

		.color = Color::Red,
		.outlineColor = Color::Transparent,
		.outlineThickness = 5/64,
		.radius = 10
		});


	static CoordF topSamples[2][5] = { {{0.1f,0.f}, { 0.2f, -0.25f}, { 0.4f, 0.25f}, { 0.6f, 0.23f}, { 0.8f, 0.5f}},
		{{0.5f, 0}, {0.5f, 0}, {0.5f, 0}, {0.5f,0}, {0.5f, 0}} };

	static int transitionIndex = 0;

	pGraph->entries.back().setSamples(5, topSamples[0]);





	Color colors[6] = { Color::Red, Color::Green, Color::Blue, Color::Yellow, Color::Pink, Color::Brown };

	// Setup grid

	pGraph->xLines.pushBack({ .label = "-0.5", .pos = -0.5f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "-0.25", .pos = -0.25f, .thickness = 0.5f });
	pGraph->xLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 1.f });
	pGraph->xLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });

	pGraph->yLines.pushBack({ .label = "0.0", .pos = 0.0f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.25", .pos = 0.25f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "0.5", .pos = 0.5f, .thickness = 0.5f });
	pGraph->yLines.pushBack({ .label = "1.0", .pos = 1.f, .thickness = 0.5f });

	/*
		pGraph->xLines.pushBack(WGBP(GridLine,
								_.value = -0.25f
								));
	*/

	//


	auto pTransition = CoordTransition::create(2000000, TransitionCurve::Bezier);
	pGraph->entries.back().transitionSamples(pTransition, 5, topSamples[1]);



	auto pButtonSkin = BoxSkin::create({ .color = Color8::Grey,
									  .outlineColor = Color8::Black,
									  .outlineThickness = 1,
									  .padding = 3
		});


	auto pButton = Button::create({ .label = {.text = "TRANSITION"}, .skin = pButtonSkin });

	Base::msgRouter()->addRoute(pButton, MsgType::Select, [pGraph, pTransition](Msg* pMsg)
		{
			transitionIndex = (transitionIndex + 1) % 2;
			pGraph->entries.back().transitionSamples(pTransition, 5, topSamples[transitionIndex]);
		});


	pFlex->slots.pushBack(pButton, { .pos = {105, 220 } });


	return true;
}

//____ nortonCommanderTest() ______________________________________________________

bool nortonCommanderTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pPaneSkin = BoxSkin::create( { .color = Color8::White,
										.outlineColor = Color8::Black,
										.outlineThickness = 1,
										.padding = 3,
										.spacing = 5
	});
	
	auto pPane1 = Filler::create( { .skin = pPaneSkin });
	auto pPane2 = Filler::create( { .skin = pPaneSkin });

	auto pShadowSkin = BoxSkin::create({ .color = Color8::Gray,
										 .outlineColor = HiColor{4096,0,0,2048},
										 .outlineThickness = {0,16,16,0},
										 .overflow = {0,16,16,0},
		});



	auto pButtonContentSkin = BoxSkin::create( { .color = Color8::Grey,
										  .outlineColor = Color8::Black,
										  .outlineThickness = 1,
										  .padding = 3
	});

	auto pButtonSkin = pShadowSkin; // DoubleSkin::create(pButtonContentSkin, pShadowSkin);


	auto pCopyButton = Button::create( {
		.label = { .text = "COPY" },
		.skin = pButtonSkin
	 });

	auto pMoveButton = Button::create( {
		.label = { .text = "MOVE" },
		.skin = pButtonSkin
	 });

	auto pDeleteButton = Button::create( {
		.label = { .text = "DELETE" },
		.skin = pButtonSkin
	 });

	auto pMyMenubar = Filler::create( { .defaultSize = { 20, 20}, .skin = ColorSkin::create( {.color = Color8::Grey }) });
	

	// The FlexPanel way.
/*
	auto pFlexPanel = FlexPanel::create( { .confineWidgets = true });
 
	pFlexPanel->slots.pushBack({
		{ pMyMenubar, {	.pin1 = { Placement::NorthWest },
						.pin2 = { Placement::NorthEast, {0,20} }
		}},
		{ pPane1,     {	.pin1 = { Placement::NorthWest, {5,20+5} },
						.pin2 = { Placement::South, {-50-5,-5} }
		}},
		{ pPane2,     {	.pin1 = { Placement::North, {5+50,20+5} },
						.pin2 = { Placement::SouthEast, {-5,-5} }
		}},
		{ pCopyButton,{ .origo = Placement::Center,
						.pos = {0, -(15+5+30)},
						.size = {100,30}
		}},
		{ pMoveButton, { .origo = Placement::Center,
						 .pos = {0,-15},
						 .size = {100,30}
		}},
		{ pDeleteButton, { .origo = Placement::Center,
						   .pos = {0, (15+5)},
						   .size = {100,30}
		}}
	});

	*pEntry = pFlexPanel;
*/



	// The PackPanel way.

	PackLayout_p pLayout = PackLayout::create({ .expandFactor = PackLayout::Factor::Weight,
												.shrinkFactor = PackLayout::Factor::Weight });

	PackPanel_p pWindowPanel = PackPanel::create({ .axis = Axis::Y, .layout = pLayout });
	PackPanel_p pMainSection = PackPanel::create({ .axis = Axis::X, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Transparent, .padding = 5 }), .spacing = 5 });
	PackPanel_p pButtonColumn = PackPanel::create({ .axis = Axis::Y, .layout = pLayout , .spacing = 5 });

	pButtonColumn->slots.pushBack({ {Filler::create(), {} },
									{pCopyButton, {.weight = 0.f }},
									{pMoveButton, {.weight = 0.f }},
									{pDeleteButton, {.weight = 0.f }},
									{Filler::create(), {} }
		});

	pMainSection->slots.pushBack({ {pPane1, {} },
								   {pButtonColumn, {.weight = 0.f } },
								   {pPane2, {} }
		});

	pWindowPanel->slots.pushBack({ {pMyMenubar, {.weight = 0.f}},
								   {pMainSection, {} }
		});

	*pEntry = pWindowPanel;

	return true;
}

//____ skinMarginTest() ______________________________________________________

bool skinMarginTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pWrapTextLayout = BasicTextLayout::create( { .wrap = true });

	auto pPanel = TwoSlotPanel::create();
	pPanel->setAxis(Axis::X);

	auto pContent1 = TextEditor::create({ .editor = {.layout = pWrapTextLayout } } );
	pContent1->setSkin(ColorSkin::create( { .color = Color::Red, .spacing = 20 } ));
	pPanel->slots[0] = pContent1;

	auto pContent2 = TextEditor::create({ .editor = {.layout = pWrapTextLayout } });
	pContent2->setSkin(ColorSkin::create( { .color = Color::Green, .spacing = 50 }));
	pPanel->slots[1] = pContent2;


	auto pLayout = PackLayout::create( { .expandFactor = PackLayout::Factor::One, .shrinkFactor = PackLayout::Factor::Weight } );
	pPanel->setLayout(pLayout);

	*pEntry = pPanel;

	return true;
}

//____ widgetRecording() ______________________________________________________

bool widgetRecording(ComponentPtr<DynamicSlot> pEntry)
{

	auto pBack = FlexPanel::create({ .skin = ColorSkin::create(Color::White) });
	*pEntry = pBack;

	//
	
/*
	auto pButton = Button::create({ .skin = m_pSimpleButtonSkin });
	pBack->slots.pushBack( pButton, { .pos = {20,20}, .size = { 100, 50} } );

	TextStyle_p pStyle = TextStyle::create({ .color = Color::Black, .font = m_pFont, .size = 20 });

	auto pLayout = BasicTextLayout::create({ .placement = Placement::Center });


	pButton->label.setText("EDIT");
	pButton->label.setStyle(pStyle);
	pButton->label.setLayout(pLayout);
*/


	//

	auto pBgSkin1 = PieMeterSkin::create({
											.backColor = Color::Black,
											.emptyColor = Color::DarkGrey,
											.hubColor = Color::Transparent,
											.hubSize = 0.75f,
											.length = 120,
											.min = 0.10f,
											.rotation = -60,
											.slices = {{ 1.f, Color::DarkBlue, Color::Blue }}
										});


	auto pBgSkin2 = PieMeterSkin::create({
		.backColor = Color::Black,
		.emptyColor = Color::DarkGrey,
		.hubColor = Color::Transparent,
		.hubSize = 0.75f,
		.length = 120,
		.min = 0.10f,
		.rotation = -60,
										.slices = { {1.f, Color::Green, Color::Green},
													{1.f, Color::Yellow, Color::Yellow},
													{1.f,Color::Red,Color::Red}
													}
										});

	auto pBgSkin3 = PieMeterSkin::create({
									.backColor = Color::Black,
									.emptyColor = Color::DarkGrey,
									.hubColor = Color::Yellow,
									.hubSize = 0.25f,
									.length = 120,
									.min = 0.1,
									.movingSlices = true,
									.rotation = -60,
									.slices = { {0.7f,Color::DarkBlue,Color::DarkBlue},
												{0.2f,Color::Blue,Color::Blue},
												{0.1f,Color::LightBlue,Color::LightBlue}
									}
		});


	auto pKnob1 = Knob::create( { .dragAxis = Axis::X, .skin = pBgSkin3, .value = 0.0f });
	auto pKnob2 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin3, .value = 0.5f });
	auto pKnob3 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin3, .value = 1.0f });

	pBack->slots.pushBack(pKnob1, { .pos = {10, 10}, .size = {150, 150} });
	pBack->slots.pushBack(pKnob2, { .pos = {170, 10}, .size = {150, 150} });
	pBack->slots.pushBack(pKnob3, { .pos = {330, 10}, .size = {150, 150} });
//	pBaseLayer->slots.pushBack(pKnob4, { .pos = {340, 10}, .size = {100, 200} });
//	pBaseLayer->slots.pushBack(pKnob5, { .pos = {450, 10}, .size = {200, 100} });
	
	

	return true;
}

//____ wgcombTest() ______________________________________________________

bool wgcombTest(ComponentPtr<DynamicSlot> pEntry)
{

	auto pBack = FlexPanel::create({ .skin = ColorSkin::create(Color::White) });
	*pEntry = pBack;

	//

	TextStyle_p pStyle = TextStyle::create({ .color = Color::Black, .size = 20 });
	auto pLayout = BasicTextLayout::create({ .placement = Placement::Center });

	
	Button::Blueprint bp = { .label = {.layout = pLayout, .style = pStyle },
							 .skin = m_pSimpleButtonSkin
	};

	auto pButton1 = Button::create( WGOVR( bp, _.label.text = "OK" ));
	auto pButton2 = Button::create( WGOVR( bp, _.label.text = "CANCEL" ));

	pBack->slots.pushBack( pButton1, { .pos = {20,20}, .size = { 100, 50} } );
	pBack->slots.pushBack( pButton2, { .pos = {140,20}, .size = { 100, 50} } );


/*
	PieMeterSkin::Blueprint baseBP = {
		.backColor = Color::Black,
		.emptyColor = Color::DarkGrey,
		.hubColor = Color::Transparent,
		.hubSize = 0.75f,
		.length = 120,
		.min = 0.10f,
		.rotation = -60
	};

	//

	auto pBgSkin1 = PieMeterSkin::create( WGCOMB( baseBP, _.movingSlices = true ));


	auto pBgSkin2 = PieMeterSkin::create( WGCOMB( baseBP,
		_.slices = { {1.f, Color::Green, Color::Green},
		 			 {1.f, Color::Yellow, Color::Yellow},
					 {1.f,Color::Red,Color::Red}
					}
	));

	auto pBgSkin3 = PieMeterSkin::create( WGCOMB( baseBP,
		_.hubColor = Color::Yellow,
		_.hubSize = 0.25f,
		_.movingSlices = true,
		_.slices = { {0.7f,Color::DarkBlue,Color::DarkBlue},
					{0.2f,Color::Blue,Color::Blue},
					{0.1f,Color::LightBlue,Color::LightBlue}
		}
	));


	auto pKnob1 = Knob::create( { .dragAxis = Axis::X, .skin = pBgSkin1, .value = 0.0f });
	auto pKnob2 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin2, .value = 0.5f });
	auto pKnob3 = Knob::create( { .dragAxis = Axis::Y, .skin = pBgSkin3, .value = 1.0f });

	pBack->slots.pushBack(pKnob1, { .pos = {10, 10}, .size = {150, 150} });
	pBack->slots.pushBack(pKnob2, { .pos = {170, 10}, .size = {150, 150} });
	pBack->slots.pushBack(pKnob3, { .pos = {330, 10}, .size = {150, 150} });
//	pBaseLayer->slots.pushBack(pKnob4, { .pos = {340, 10}, .size = {100, 200} });
//	pBaseLayer->slots.pushBack(pKnob5, { .pos = {450, 10}, .size = {200, 100} });
	
*/
	
	return true;
}

//____ canvasCapsuleTest() ______________________________________________________

bool canvasCapsuleTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBack = FlexPanel::create({ .skin = ColorSkin::create(Color::White) });
	*pEntry = pBack;

	
	auto pWrapTextLayout = BasicTextLayout::create( { .wrap = true });

	auto pPanel = TwoSlotPanel::create({ .skin = ColorSkin::create(Color::Pink) });
	pPanel->setAxis(Axis::X);

	auto pContent1 = TextEditor::create({ .editor = {.layout = pWrapTextLayout, .text = "TEST1" } } );
	pContent1->setSkin(ColorSkin::create( { .color = Color::Red, .spacing = 20 } ));
	pPanel->slots[0] = pContent1;

	auto pContent2 = TextEditor::create({ .editor = {.layout = pWrapTextLayout, .text = "TEST2" } });
	pContent2->setSkin(ColorSkin::create( { .color = Color::Green, .spacing = 50 }));
	pPanel->slots[1] = pContent2;

	
	auto pCanvasCapsule = CanvasCapsule::create( {  
		.scaleCanvas = true,
		.skin = BoxSkin::create({ .color = Color::Yellow,
			.outlineColor = Color::Black,
			.outlineThickness = 5,
			.padding = 6 }),
		.skinAroundCanvas = true
	});
	
	pCanvasCapsule->slot = pPanel;

	pBack->slots.pushBack(pCanvasCapsule, { .pos = {10,10}, .size = { 400, 200 } });

	
	auto pCanvasDisplay1 = CanvasDisplay::create({	.canvas = pCanvasCapsule, 
													.skin = BoxSkin::create({ .color = Color::Green,
													.outlineColor = Color::Blue, .outlineThickness = 2, .padding = 3 }),
													.skinAroundCanvas = true });

	pBack->slots.pushBack(pCanvasDisplay1, { .pos = {10, 240}, .size = {200,200} });

	pCanvasDisplay1->setTintColor(HiColor(4096,4096,4096,1024));
	pCanvasDisplay1->setTintColor(HiColor::White, ColorTransition::create(4000*1000, TransitionCurve::EaseOut));

	
	return true;
}

//____ canvasCapsuleGlowTest() ______________________________________________________

bool canvasCapsuleGlowTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBack = FlexPanel::create({ .skin = ColorSkin::create(Color::Black) });
	*pEntry = pBack;

	auto pMyStyle = TextStyle::create( { .color = Color::White, .size = 30 } );


	auto pGlowCapsule = CanvasCapsule::create();

	float blueMtx[9] = { 0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f };
	float greenMtx[9] = { 0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f };
	float redMtx[9] = { 0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f };


	auto pBlurbrush = Blurbrush::create( WGBP(Blurbrush,
			_.blue = blueMtx,
			_.green = greenMtx,
			_.red = redMtx
		));




	//	pGlowCapsule->setResolution( {64,64} );

	pGlowCapsule->glow.setBlurbrush(pBlurbrush);
	pGlowCapsule->glow.setRefreshRate(30);
	pGlowCapsule->glow.setActive(true);


	pGlowCapsule->setSkin(BoxSkin::create({ .color = HiColor::Transparent, .outlineColor = Color::Yellow, .outlineThickness = 50, .padding = 50 }));

	pBack->slots.pushBack(pGlowCapsule, { .pos = {  50, 50}, .size = {300,0} });

	auto pWrapTextLayout = BasicTextLayout::create({ .wrap = true });


	auto pContent = TextEditor::create({ .editor = {.layout = pWrapTextLayout, .style = pMyStyle, .text = "HEJ" } });
	pGlowCapsule->slot = pContent;

	auto pTintWhite = Button::create({ .label = {.text = "WHITE" }, .skin = m_pSimpleButtonSkin });
	auto pTintBlack = Button::create({ .label = {.text = "BLACK" }, .skin = m_pSimpleButtonSkin });

	auto pTransition = ColorTransition::create(1000*1000, TransitionCurve::EaseOut);

	auto pGradientWhite = Button::create({ .label = {.text = "GRADIENT WHITE" }, .skin = m_pSimpleButtonSkin });
	auto pGradientBlack = Button::create({ .label = {.text = "GRADIENT BLACK" }, .skin = m_pSimpleButtonSkin });

	Base::msgRouter()->addRoute(pTintWhite, MsgType::Select, [pGlowCapsule,pTransition](Msg* pMsg) {pGlowCapsule->setTintColor(Color::White, pTransition); });
	Base::msgRouter()->addRoute(pTintBlack, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintColor(Color::Black, pTransition); });

	Base::msgRouter()->addRoute(pGradientWhite, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintGradient(Gradient( Placement::NorthWest, HiColor::White, HiColor::Black), pTransition); });
	Base::msgRouter()->addRoute(pGradientBlack, MsgType::Select, [pGlowCapsule, pTransition](Msg* pMsg) {pGlowCapsule->setTintGradient(Gradient(Placement::SouthEast, HiColor::White, HiColor::Black), pTransition); });


	auto pButtons = PackPanel::create({ .axis = Axis::X });

	pButtons->slots << pTintWhite << pTintBlack << pGradientWhite << pGradientBlack;

	pBack->slots.pushBack( pButtons, { .origo = Placement::SouthWest } );

	return true;
}

//____ textDisplayTest() ________________________________________________________

bool textDisplayTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pSkin = BoxSkin::create( { .color = Color::White, .outlineColor = Color::Red, .outlineThickness = 1, .padding = 4 } );
	auto pInnerSkin = BoxSkin::create( { .color = Color::White, .outlineColor = Color::Black, .outlineThickness = 1, .padding = 4 } );

	
	auto pTextLayout = BasicTextLayout::create( { .wrap = true } );
	
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pScaleCapsule = ScaleCapsule::create({ .child = pBaseLayer, .scale = 128 });
	
	auto pPackPanel = PackPanel::create({ .axis = Axis::Y, .skin = pInnerSkin });

	auto pText1 = TextDisplay::create( { .display = {.layout = pTextLayout }, .skin = pSkin });
	pText1->display.setText("Och OM de skulle ge något följdfel så vill jag veta det så fort som möjligt. Men jag tror inte det, det var verkligen uppenbara fel som var lätta att fixa (använde m_scale istf parameter scale på flera ställen. Ibland behöver man räkna ut vad geometrin kommer att bli inför en kommande.");
	pPackPanel->slots << pText1;

	auto pText2 = TextDisplay::create( { .skin = pSkin });
	pText2->display.setText("TEXT2");
	pPackPanel->slots << pText2;

	auto pText3 = TextDisplay::create( { .skin = pSkin });
	pText3->display.setText("TEXT3");
	pPackPanel->slots << pText3;


	auto pSizeCapsule = SizeCapsule::create({ .child = pPackPanel, .maxSize = {200, -1}} );
	*pSlot = pScaleCapsule;

	pBaseLayer->slots.pushFront(pSizeCapsule, WGBP(FlexPanelSlot, _.pos = {10,10} /*, _.size = {width, height}*/));

	auto pSkin2 = BoxSkin::create( { .outlineColor = Color::Black, .outlineThickness = 10, .padding = 11 } );
	pPackPanel->setSkin( pSkin2 );
	
	return true;

}

//____ scrollChartTest() ________________________________________________________

bool scrollChartTest(ComponentPtr<DynamicSlot> pSlot)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pScrollChart = AreaScrollChart::create({
		.latency = 50000,
		.maxDisplayTime = 8000000,
		.skin = BoxSkin::create({ .color = Color::Yellow, .outlineColor = Color::Black, .padding = 1 })

	});
	
	static float samples[1000];
	
	for (int i = 0; i < 1000; i++)
	{
//		samples[i] = 0.5f;
		
		samples[i] = ( 0.3f + sin(i / 10.0) * 0.3f);
//		bottomSamples[i] = (int)((300 + sin(i / 20.0) * 6) * 64);
	}

	float * pBeg = samples;
	float * pCurr = samples;
	float * pEnd = samples+1000;
	
	AreaScrollChart_wp wpScrollChart = pScrollChart.rawPtr();
	
	pScrollChart->entries.pushBack({ .defaultTopSample = 0.3f, .fetcher = [wpScrollChart, pBeg, pCurr, pEnd] (int64_t lastSampleTS, int64_t firstNeededTS, int64_t lastNeededTS, int64_t currentTS) mutable
	{
		int samplesWanted = (currentTS - lastSampleTS) / 1000000.0 * 50 + 1;
		
		while(samplesWanted > 0)
		{
			if( pEnd - pCurr < samplesWanted )
			{
				int nbSamples = int(pEnd - pCurr);
				wpScrollChart->entries[0].addSamples(nbSamples, 50, pCurr, nullptr);
				samplesWanted -= nbSamples;
				pCurr = pBeg;
			}
			else
			{
				wpScrollChart->entries[0].addSamples(samplesWanted, 50, pCurr, nullptr);
				pCurr += samplesWanted;
				samplesWanted = 0;
			}
		}				
	} });
	
	
	pScrollChart->start();
	

	
//	pScrollChart->entries[0].addSamples(1000, 50, samples, nullptr);
	

	pBaseLayer->slots.pushBack(pScrollChart, { .pos = {10,10}, .size = {500,300} } );
	
	*pSlot = pBaseLayer;


	auto pButtonSkin = BoxSkin::create({ .color = Color8::Grey,
									  .outlineColor = Color8::Black,
									  .outlineThickness = 1,
									  .padding = 3
	});


	auto pTransition = ColorTransition::create(2000000);

	auto pButtonMoveLeft = Button::create({ .label = {.text = "Green"}, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonMoveLeft, { .origo = Placement::SouthWest, .pos = {5,-5} });

	Base::msgRouter()->addRoute(pButtonMoveLeft, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->entries[0].setColors(Color::Green, Color::Red, pTransition); });

	auto pButtonMoveRight = Button::create({ .label = {.text = "Blue"}, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonMoveRight, { .origo = Placement::SouthWest, .pos = {5 + 100,-5} });

	Base::msgRouter()->addRoute(pButtonMoveRight, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->entries[0].setColors(Color::Blue, Color::Black, pTransition); });

	
	auto pButtonStop = Button::create({ .label = {.text = "Stop"}, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonStop, { .origo = Placement::SouthWest, .pos = {5 + 200,-5} });

	Base::msgRouter()->addRoute(pButtonStop, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->stop(); });

	auto pButtonStart = Button::create({ .label = {.text = "Start"}, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonStart, { .origo = Placement::SouthWest, .pos = {5 + 300,-5} });

	Base::msgRouter()->addRoute(pButtonStart, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->start(); });

	auto pButtonSpeedDown = Button::create({ .label = {.text = "-" }, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonSpeedDown, { .origo = Placement::SouthWest, .pos = {5 + 400,-5} });

	Base::msgRouter()->addRoute(pButtonSpeedDown, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->setDisplayTime(pScrollChart->displayTime() + 500000); });


	auto pButtonSpeedUp = Button::create({ .label = {.text = "+" }, .skin = pButtonSkin });

	pBaseLayer->slots.pushBack(pButtonSpeedUp, { .origo = Placement::SouthWest, .pos = {5 + 450,-5} });

	Base::msgRouter()->addRoute(pButtonSpeedUp, MsgType::Select, [pScrollChart, pTransition](Msg* p) { pScrollChart->setDisplayTime(pScrollChart->displayTime()-500000); });

	//---

	auto pButtonRange = Button::create({ .label = {.text = "RANGE"}, .skin = pButtonSkin });

	Base::msgRouter()->addRoute(pButtonRange, MsgType::Select, [pScrollChart](Msg* pMsg)
	{
		static bool bExpanded = false;

		if( bExpanded )
		{
			pScrollChart->setDisplayRange(2.f, 0.f, ValueTransition::create( 400000 ) );
			bExpanded = false;
		}
		else
		{
			pScrollChart->setDisplayRange(1.f, 0.f, ValueTransition::create( 400000 ) );
			bExpanded = true;
		}
	});

	pBaseLayer->slots.pushBack(pButtonRange, { .origo = Placement::SouthWest, .pos = {5 + 500,-5} });

	
	return true;
}

//____ packPanelStressTest() ______________________________________________________

bool packPanelStressTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pPaneSkin = BoxSkin::create( { .color = Color8::White,
										.outlineColor = Color8::Black,
										.outlineThickness = 1,
										.padding = 3,
										.spacing = 5
	});
	
	auto pButtonSkin = BoxSkin::create( { .color = Color8::Grey,
										  .outlineColor = Color8::Black,
										  .outlineThickness = 1,
										  .padding = 3
	});

	auto pFitsButton = Button::create( {
		.label = { .text = "FITS" },
		.skin = pButtonSkin
	 });

	auto pDeleteButton = Button::create( {
		.label = { .text = "TOO LARGE" },
		.skin = pButtonSkin
	 });

	
	auto pWideButton = Button::create( {
		.label = { .text = "TOO WIDE" },
		.skin = pButtonSkin
	 });

	auto pHighButton = Button::create( {
		.label = { .text = "TOO HIGH" },
		.skin = pButtonSkin
	 });

	
	Surface_p pImg = Base::defaultSurfaceFactory()->createSurface({ .format = PixelFormat::BGR_8, .size = {1000,800} });
	pImg->fill( HiColor::White );
	
	
	auto pImage = Image::create( { .image = pImg, .sizePolicy = SizePolicy2D::Scale, .skin = ColorSkin::create( Color8::Yellow ) });
	
	
	auto pCapsule = SizeCapsule::create({ .child = pImage, .maxSize = {600,600} });
	
	
	PackLayout_p pLayout = PackLayout::create({ .expandFactor = PackLayout::Factor::Weight,
												.shrinkFactor = PackLayout::Factor::Weight });

	PackPanel_p pOuterPanel = PackPanel::create({ .axis = Axis::X, .id = 1, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Red}) });
	
	PackPanel_p pInnerPanel = PackPanel::create({ .axis = Axis::Y, .id = 2, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Green}) });

	pOuterPanel->slots << Filler::create() << pInnerPanel << Filler::create();
	pOuterPanel->setSlotWeight(0, 3, {1,0,1});
	
	pInnerPanel->slots << Filler::create() << Filler::create() << Filler::create();
	pInnerPanel->setSlotWeight(0, 3, {1,0,1});

	
	*pEntry = pOuterPanel;

	pInnerPanel->slots[1] = pCapsule;
	
	
	return true;
}

//____ packPanelStressTest2() ______________________________________________________

bool packPanelStressTest2(ComponentPtr<DynamicSlot> pEntry)
{
	auto pPaneSkin = BoxSkin::create({ .color = Color8::White,
										.outlineColor = Color8::Black,
										.outlineThickness = 1,
										.padding = 3,
										.spacing = 5
		});

	auto pButtonSkin = BoxSkin::create({ .color = Color8::Grey,
										  .outlineColor = Color8::Black,
										  .outlineThickness = 1,
										  .padding = 3
		});

	auto pHideLeftButton = Button::create({
		.label = {.text = "HIDE LEFT" },
		.skin = pButtonSkin
		});

	auto pUnhideLeftButton = Button::create({
		.label = {.text = "UNHIDE LEFT" },
		.skin = pButtonSkin
		});


	PackLayout_p pLayout = PackLayout::create({ .expandFactor = PackLayout::Factor::Weight,
												.shrinkFactor = PackLayout::Factor::Weight });

	PackPanel_p pButtonRow = PackPanel::create({ .axis = Axis::X, .id = 2, .layout = pLayout, .skin = ColorSkin::create({.color = Color::LawnGreen}) });

	PackPanel_p pOuterPanel = PackPanel::create({ .axis = Axis::Y, .id = 1, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Red}) });


	PackPanel_p pInnerPanel = PackPanel::create({ .axis = Axis::X, .id = 2, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Green}) });

	pOuterPanel->slots << Filler::create( { .defaultSize = {200,30}, .skin = ColorSkin::create(Color::Brown) }) << pInnerPanel << pButtonRow;
	pOuterPanel->setSlotWeight(0, 3, { 0,1,0 });

	pInnerPanel->slots << Filler::create({ .defaultSize = {200,0}, .skin = ColorSkin::create(Color::Gray) })
		<< Filler::create({ .defaultSize = {200,200}, .skin = ColorSkin::create(Color::White) })
		<< Filler::create({ .defaultSize = {200,0}, .skin = ColorSkin::create(Color::DarkGray) });

	pInnerPanel->setSlotWeight(0, 3, { 0,1,0 });

	pButtonRow->slots << pHideLeftButton << pUnhideLeftButton;

	Base::msgRouter()->addRoute(pHideLeftButton, MsgType::Select, [pInnerPanel](Msg* pMsg) { wg_dynamic_cast<Filler_p>(pInnerPanel->slots[0].widget())->setDefaultSize({ 0,0 }); });
	Base::msgRouter()->addRoute(pUnhideLeftButton, MsgType::Select, [pInnerPanel](Msg* pMsg) { wg_dynamic_cast<Filler_p>(pInnerPanel->slots[0].widget())->setDefaultSize({ 200,0 }); });


	*pEntry = pOuterPanel;


	return true;
}

//____ blockingCapsuleTest() ______________________________________________________

bool blockingCapsuleTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pPaneSkin = m_pPressablePlateSkin;
	
	auto pPane1 = Filler::create( { .skin = pPaneSkin });
	auto pPane2 = Filler::create( { .skin = pPaneSkin });

	auto pButtonSkin = m_pSimpleButtonSkin;

	auto pCopyButton = Button::create( {
		.label = { .text = "COPY" },
		.skin = pButtonSkin
	 });

	auto pMoveButton = Button::create( {
		.label = { .text = "MOVE" },
		.skin = pButtonSkin
	 });

	auto pDeleteButton = Button::create( {
		.label = { .text = "DELETE" },
		.skin = pButtonSkin
	 });

	auto pMyMenubar = Filler::create( { .defaultSize = { 20, 20}, .skin = ColorSkin::create( {.color = Color8::Grey }) });

	// The PackPanel way.
	
	PackLayout_p pLayout = PackLayout::create({ .expandFactor = PackLayout::Factor::Weight,
												.shrinkFactor = PackLayout::Factor::Weight });

	PackPanel_p pWindowPanel = PackPanel::create({ .axis = Axis::Y, .layout = pLayout });
	PackPanel_p pMainSection = PackPanel::create({ .axis = Axis::X, .layout = pLayout, .skin = ColorSkin::create({.color = Color::Transparent, .padding = 5 }), .spacing = 5 });
	PackPanel_p pButtonColumn = PackPanel::create({ .axis = Axis::Y, .layout = pLayout });

	pButtonColumn->slots.pushBack({ {Filler::create(), {} },
									{pCopyButton, {.weight = 0.f }},
									{pMoveButton, {.weight = 0.f }},
									{pDeleteButton, {.weight = 0.f }},
									{Filler::create(), {} }
		});

	pMainSection->slots.pushBack({ {pPane1, {} },
								   {pButtonColumn, {.weight = 0.f } },
								   {pPane2, {} }
		});

	pWindowPanel->slots.pushBack({ {pMyMenubar, {.weight = 0.f}},
								   {pMainSection, {} }
		});

	
	auto pBlockingCapsule = BlockingCapsule::create( { .autoDeactivate = true } );
	pBlockingCapsule->blockedWidgets = { pCopyButton.rawPtr(), pDeleteButton.rawPtr() };
	
	pBlockingCapsule->slot = pWindowPanel;
	
	*pEntry = pBlockingCapsule;

	return true;
}

//____ tablePanelTest() _______________________________________________________

bool tablePanelTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pRowSkin1 = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Blue, .outlineThickness = 4, .padding = 4 });
	auto pRowSkin2 = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Red, .outlineThickness = 4, .padding = 4 });

	auto pColumnLayout = PackLayout::create({
		
		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight,
	});
	
	auto pRowLayout = PackLayout::create({

		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight,
		});

	auto pTable = TablePanel::create( {
			
//		.columnLayout = pColumnLayout,

		.columnSpacing = 2,
		.columnSpacingAfter = 1,
		.columnSpacingBefore = 1,

//		.rowLayout = pRowLayout,

		.rowSkin = pRowSkin1,
		.rowSkin2 = pRowSkin2,

		.rowSpacing = 2,
		.rowSpacingAfter = 4,
		.rowSpacingBefore = 4,
	
		.skin = ColorSkin::create(Color::White),
	});
	
	auto pLayout = BasicTextLayout::create({ .lineSpacing = 1.2f, .paragraphSpacing = 1.f, .wrap = true });


	auto pTextEditor1 = TextEditor::create( WGBP(TextEditor,
		_.skin = ColorSkin::create( Color8::LightSalmon, 5 ),
		_.editor.layout = pLayout
	));
	pTextEditor1->editor.setText("TESTING 1");

	auto pTextEditor2 = TextEditor::create( WGBP(TextEditor,
		_.skin = ColorSkin::create( Color8::LightGreen, 5 ),
		_.editor.layout = pLayout
	));
	pTextEditor1->editor.setText("TESTING 2");

	auto pTextEditor3 = TextEditor::create( WGBP(TextEditor,
		_.skin = ColorSkin::create( Color8::LightBlue, 5 ),
		_.editor.layout = pLayout
	));
	pTextEditor1->editor.setText("TESTING 3");

	auto pTextEditor4 = TextEditor::create( WGBP(TextEditor,
		_.skin = ColorSkin::create( Color8::LightPink, 5 ),
		_.editor.layout = pLayout
	));
	pTextEditor1->editor.setText("TESTING 4");

	
	pTable->slots[0][0] = pTextEditor1;
	pTable->slots[0][1] = pTextEditor2;
	pTable->slots[1][0] = pTextEditor3;
	pTable->slots[1][1] = pTextEditor4;

	pTable->rows.insert( pTable->rows.begin() + 1,4, { .visible = true } );

	pTable->slots[1][1] = Filler::create( {
		.defaultSize = {50,0},
		.skin = ColorSkin::create( Color::Yellow ),
	} );

	pBaseLayer->slots.pushFront(pTable, { .pos = {10,10}, .size = {400, 400}});
	
	pTable->resize(7, 3);
	
	auto pColorSkin = ColorSkin::create(Color::Pink);
	auto pShadowSkin = BoxSkin::create({ .color = Color8::Gray,
										 .outlineColor = HiColor{4096,0,0,2048},
										 .outlineThickness = {0,16,16,0},
										 .overflow = {0,16,16,0},
		});
		
	auto pCombSkin = DoubleSkin::create(pColorSkin, pShadowSkin);


	pTable->slots[6][2] = Filler::create( {
		.defaultSize = {50,0},
		.skin = pCombSkin,
	} );

//	pTable->rows.erase(2, 4);
	
//	pTable->columns.erase(0,2);
	
//	pTable->resize(6,1);
	
	pTable->columns[0].setMinWidth(80);
	pTable->columns[1].setMinWidth(80);
	pTable->columns[2].setMinWidth(80);


	*pEntry = pBaseLayer;
	
	return true;
}

//____ tablePanelTest2() _______________________________________________________

bool tablePanelTest2(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pColumnLayout = PackLayout::create({

		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight,
		});

	auto pRowLayout = PackLayout::create({

		.expandFactor = PackLayout::Factor::Weight,
		.shrinkFactor = PackLayout::Factor::Weight,
		});

	auto pTable = TablePanel::create({

				.columnLayout = pColumnLayout,

				.columns = 3,

				.columnSpacing = 4,
				.columnSpacingAfter = 1,
				.columnSpacingBefore = 1,

				//		.rowLayout = pRowLayout,

				.rows = 3,

				.rowSpacing = 4,
				.rowSpacingAfter = 4,
				.rowSpacingBefore = 4,

				.skin = ColorSkin::create(Color::Green),
		});

	auto pLayout = BasicTextLayout::create({ .lineSpacing = 1.2f, .paragraphSpacing = 0.2f, .wrap = true });

	auto pCellSkin = BoxSkin::create({

		.color = Color::White,
		.outlineColor = Color::Black,
		.outlineThickness = 1,
		.padding = 4
	});

	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 3; x++)
		{
			auto pTextEditor = TextEditor::create(WGBP(TextEditor,
				_.skin = pCellSkin,
				_.editor.layout = pLayout
			));

			pTable->slots[y][x] = pTextEditor;
		}
	}


	auto pColorSkin = ColorSkin::create(Color::Pink);
	auto pShadowSkin = BoxSkin::create({ .color = Color8::Gray,
										 .outlineColor = HiColor{4096,0,0,2048},
										 .outlineThickness = {0,16,16,0},
										 .overflow = {0,16,16,0},
		});

	auto pCombSkin = DoubleSkin::create(pColorSkin, pShadowSkin);

	pTable->setMinColumnWidth(pTable->columns.begin(),pTable->columns.end(),80);

	pBaseLayer->slots.pushFront(pTable, { .pos = {10,10}, .size = {00,0} });
	*pEntry = pBaseLayer;

	return true;
}

bool dragndropTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pTargetSkin = BoxSkin::create(WGBP(BoxSkin,
											_.outlineColor = Color::Black,
											_.color = Color::Red,
											_.states = { {State::Targeted, {.color = Color::Green}} }
											));

	auto pSourceSkin = BoxSkin::create(WGBP(BoxSkin,
											_.outlineColor = Color::Black,
											_.color = Color::Blue,
											_.states = { {State::Hovered, {.color = Color::LightBlue}} }
											));


	auto pTarget1 = Filler::create(WGBP(Filler,
										_.skin = pTargetSkin,
										_.defaultSize = {50,50},
										_.dropTarget = true,
										_.id = 0x777));

	auto pTarget2 = Filler::create(WGBP(Filler,
										_.skin = pTargetSkin,
										_.defaultSize = {50,50},
										_.dropTarget = true,
										_.id = 0x777));


	auto pDragable = Filler::create(WGBP(Filler,
										 _.skin = pSourceSkin,
										 _.defaultSize = {50,50},
										 _.pickable = true ));


	pBaseLayer->slots.pushBack(pTarget1, { .pos = {20,300} } );
	pBaseLayer->slots.pushBack(pTarget2, { .pos = {300,20} } );
	pBaseLayer->slots.pushBack(pDragable, { .pos = {300,300} } );

	Base::msgRouter()->addRoute(pTarget1, MsgType::DropProbe, [](Msg*pMsg){
		static_cast<DropProbeMsg*>(pMsg)->accept();
	});

	Base::msgRouter()->addRoute(pTarget2, MsgType::DropProbe, [](Msg*pMsg){
		static_cast<DropProbeMsg*>(pMsg)->accept();
	});


	*pEntry = pBaseLayer;

	return true;
}

bool fillerTransitionTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));


	auto pMyFiller = Filler::create( { .defaultSize = {100,100}, .skin = ColorSkin::create(Color::Honeydew)  } );

	pBaseLayer->slots.pushBack(pMyFiller, { .pos = {20,40} });

	auto pButton1 = Button::create({ .label = { .text = "-" }, .skin = ColorSkin::create(Color::LightGrey) });
	auto pButton2 = Button::create({ .label = { .text = "+" }, .skin = ColorSkin::create(Color::LightGrey) });

	pBaseLayer->slots.pushBack( pButton1, { .pos = {10,10}, .size = {50,20} } );
	pBaseLayer->slots.pushBack( pButton2, { .pos = {70,10}, .size = {50,20} } );

	Base::msgRouter()->addRoute(pButton1, MsgType::Select, [pMyFiller](Msg*){ pMyFiller->setDefaultSize({30,30}, CoordTransition::create(200000, TransitionCurve::EaseInOut)); } );

	Base::msgRouter()->addRoute(pButton2, MsgType::Select, [pMyFiller](Msg*){ pMyFiller->setDefaultSize({300,160}, CoordTransition::create(2000000, TransitionCurve::EaseInOut)); } );


	*pEntry = pBaseLayer;

	return true;

}

Widget_p	createDragBoxWithHandle( Size size, Color color )
{
	auto pWidget = FlexPanel::create({ .defaultSize = size, .skin = ColorSkin::create(color) });
	auto pHandle = Filler::create({ .defaultSize = size / 4, .pickHandle = true, .skin = BoxSkin::create({.color = color, .outlineColor = Color::Black }) });

	pWidget->slots.pushBack(pHandle, { .pos = {size.w / 4, size.h / 4} });

	return pWidget;
}


bool reorderCapsuleTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pBucketSkin = BoxSkin::create( { .color = Color::White, .outlineColor = Color::Black, .padding = 1 });

	//---- First container

	{
		auto pBucket1 = ReorderCapsule::create({ .dragOutside = true, .skin = pBucketSkin, .usePickHandles = false });

//		pBucket1->setTransitionSkin(BoxSkin::create( {.spacing = 2} ) );

		auto pPackPanel = PackPanel::create();

		auto pFiller1 = createDragBoxWithHandle( {100,50},Color::Honeydew);
		auto pFiller2 = createDragBoxWithHandle({ 100,50 }, Color::Chartreuse);
		auto pFiller3 = createDragBoxWithHandle({ 100,50 }, Color::Gainsboro);
		auto pFiller4 = createDragBoxWithHandle({ 100,50 }, Color::LemonCiffon);
		auto pFiller5 = createDragBoxWithHandle({ 100,50 }, Color::Moccasin);

		pPackPanel->slots.pushBack({ pFiller1, pFiller2, pFiller3, pFiller4, pFiller5 });

		pBucket1->slot = pPackPanel;

		pBaseLayer->slots.pushBack(pBucket1, { .pos = {10,10} });
	}

	//---- Second container

	{
		auto pBucket1 = ReorderCapsule::create({ .skin = pBucketSkin });

		auto pPackPanel = PackPanel::create({ .axis = Axis::Y} );

		auto pFiller1 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Red) });
		auto pFiller2 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Green) });
		auto pFiller3 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Blue) });
		auto pFiller4 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Yellow) });
		auto pFiller5 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Pink) });

		pPackPanel->slots.pushBack({ pFiller1, pFiller2, pFiller3, pFiller4, pFiller5 });

		pBucket1->slot = pPackPanel;

		pBaseLayer->slots.pushBack(pBucket1, { .pos = {10,200} });

	}

	//---- trash can

	auto pTrashCan = LabelCapsule::create({ .child = Filler::create( { .defaultSize = {100,100}}),
											.dropTarget = true,
											.label = { .text = "BLACK HOLE" },
											.labelPlacement = Placement::North,
											.labelSkin = BoxSkin::create( { .color = Color::White, .outlineColor = Color::Black, .padding = 2 } ),
											.pickable = false,
											.skin = BoxSkin::create({ 	.color = Color::Black,
																		.outlineColor = Color::Black,
																		.padding = 4,
																		.spacing = 10 })
	});



	pBaseLayer->slots.pushBack(pTrashCan, { .pos = {300,300}});

	Base::msgRouter()->addRoute(pTrashCan, MsgType::DropProbe, [](Msg*pMsg) 
	{
		static_cast<DropProbeMsg*>(pMsg)->accept();
	});

	Base::msgRouter()->addRoute(pTrashCan, MsgType::DropDeliver, [](Msg*pMsg) 
	{
		static_cast<DropProbeMsg*>(pMsg)->accept();
	});

	//---- outside supply

	auto pSupply = LabelCapsule::create({ 	.label = { .text = "SUPPLY" },
											.labelPlacement = Placement::North,
											.labelSkin = BoxSkin::create( { .color = Color::White, .outlineColor = Color::Black, .padding = 2 } ),
											.pickable = true,
											.skin = BoxSkin::create({ 	.color = Color::White,
																		.outlineColor = Color::Black,
																		.padding = 4,
																		.spacing = 10 })
	});

	Base::msgRouter()->addRoute(pSupply, MsgType::Pick, [](Msg* _pMsg) {
		auto pMsg = static_cast<PickMsg*>(_pMsg);
		auto pMe = wg_dynamic_cast<LabelCapsule_p>(pMsg->source());

		if( !pMe->slot.isEmpty() )
		{
			auto pWidget = pMe->slot.widget();

			static Color wheel[5] = { Color::LightCyan, Color::Beige, Color::Orchid, Color::SaddleBrown, Color::Seashell };
			static int colorOfs = 0;


			pMe->slot = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create( wheel[colorOfs++]) });

			auto pDataset = ReorderCapsule::DropData::create({pWidget,0.f});
			pMsg->setContent(DropType::Widget, 0, pDataset);
			pMsg->setDragWidget(pWidget, -pMsg->pickOfs());
			pMsg->setHotspot(Placement::Center);
		}
		else
			pMsg->setContent(DropType::Undefined, 0, nullptr);

	});

	auto pFiller1 = Filler::create({ .defaultSize = {100,50}, .skin = ColorSkin::create(Color::Red) });

	pSupply->slot = pFiller1;

	pBaseLayer->slots.pushBack(pSupply, { .pos = {300, 200}, .size = {128,78}} );

	*pEntry = pBaseLayer;
	return true;

}

bool widgetMoveTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));


	auto pPackPanel = PackPanel::create();

	auto pFiller1 = Filler::create({ .defaultSize = {100,100}, .skin = ColorSkin::create(Color::Honeydew) });
	auto pFiller2 = Filler::create({ .defaultSize = {100,100}, .skin = ColorSkin::create(Color::Chartreuse) });
	auto pFiller3 = Filler::create({ .defaultSize = {100,100}, .skin = ColorSkin::create(Color::LavenderBlush) });

	pPackPanel->slots.pushBack({ pFiller3 });


	pBaseLayer->slots.pushBack(pPackPanel);

	*pEntry = pBaseLayer;

	pPackPanel->slots.insert(0, pFiller3);


	return true;

}

bool labelCapsuleTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::Honeydew));

	auto pLabelCapsuleBackSkin = BoxSkin::create({ .color = HiColor::Transparent, .outlineColor = Color::Black, .padding = {15,2,2,2}, .spacing = {10,2,2,2} });

	auto pLabelSkin = BoxSkin::create({ .color = Color::Honeydew, .outlineColor = Color::Black, .padding = 2, .spacing = { 0,10,0,10 } });


	auto pLabelCapsule = LabelCapsule::create({ .label = {.text = "Label"}, .labelPlacement = Placement::North, .labelSkin = pLabelSkin, .skin = pLabelCapsuleBackSkin});

	pLabelCapsule->slot = TextEditor::create({ .editor = {.text = "Write something here..." } });


	pBaseLayer->slots.pushBack(pLabelCapsule);
	*pEntry = pBaseLayer;
	return true;

}

bool elipsisTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::Honeydew));

	auto pTextLayout = BasicTextLayout::create({ .autoEllipsis = true });
	auto pTextLayout2 = BasicTextLayout::create({ .autoEllipsis = true, .wrap = true });

	auto pEditorSkin = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Black, .padding = 2 });

	auto pLineEditor = LineEditor::create( { .editor = {.layout = pTextLayout }, .skin = pEditorSkin  });

	auto pTextEditor = TextEditor::create( { .editor = {.layout = pTextLayout }, .skin = pEditorSkin });

	auto pTextEditor2 = TextEditor::create( { .editor = {.layout = pTextLayout2 }, .skin = pEditorSkin });


	pBaseLayer->slots.pushBack(pLineEditor);
	pBaseLayer->slots.pushBack(pTextEditor, { .pos = {10,50}, .size = {200,50}});
	pBaseLayer->slots.pushBack(pTextEditor2, { .pos = {10,300}, .size = {200,60}});
	*pEntry = pBaseLayer;
	return true;

}



bool packPanelSpacingBugTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::Honeydew));

	auto pPackPanel = PackPanel::create( { .axis = Axis::Y, .skin = ColorSkin::create(Color::ForestGreen)});



	auto pTextLayout = BasicTextLayout::create({ .autoEllipsis = true, .wrap = false });

	auto pEntrySkin = BoxSkin::create({ .color = Color::White, .outlineColor = Color::Black, .padding = 2 });

	auto pEntry1 = TextDisplay::create({ .display = {.layout = pTextLayout }, .skin = pEntrySkin });
//	auto pEntry2 = TextDisplay::create({ .display = {.layout = pTextLayout }, .skin = pEntrySkin });
//	auto pEntry3 = TextDisplay::create({ .display = {.layout = pTextLayout }, .skin = pEntrySkin });
//	auto pEntry4 = TextDisplay::create({ .display = {.layout = pTextLayout }, .skin = pEntrySkin });

	pEntry1->display.setText("This is a long text entry that should want to wrap");
//	pEntry2->display.setText("This is a long text entry that should want to wrap");
//	pEntry3->display.setText("This is a long text entry that should want to wrap");
//	pEntry4->display.setText("This is a long text entry that should want to wrap");


	pPackPanel->slots.pushBack(pEntry1);
//	pPackPanel->slots.pushBack(pEntry2);
//	pPackPanel->slots.pushBack(pEntry3);
//	pPackPanel->slots.pushBack(pEntry4);

	auto pSizeCap = SizeCapsule::create({ .child = pPackPanel, .maxSize = {200,-1} });

	pBaseLayer->slots.pushBack(pSizeCap, { .pos = {10,50} });
	*pEntry = pBaseLayer;
	return true;

}

bool bracketSkinTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pSkin = BracketSkin::create({ .color = HiColor(4096,0,0,2048), .size = {15,30}, .thickness = 5 });

	auto pFiller1 = Filler::create({ .defaultSize = {100,100}, .skin = pSkin });
	auto pFiller2 = Filler::create({ .defaultSize = {50,100}, .skin = pSkin });
	auto pFiller3 = Filler::create({ .defaultSize = {100,40}, .skin = pSkin });



	pBaseLayer->slots.pushBack(pFiller1, { .pos = {10,10} } );
	pBaseLayer->slots.pushBack(pFiller2, { .pos = {120,10} });
	pBaseLayer->slots.pushBack(pFiller3, { .pos = {10,120} });


	*pEntry = pBaseLayer;
	return true;

}


bool selectCapsuleTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pPackPanel = PackPanel::create();

	auto pSkin = ColorSkin::create({

			.states = { {State::Default,Color::Yellow}, {State::Selected,Color::Green}}

		});



	auto pFiller1 = Filler::create({ .defaultSize = {100,100}, .skin = pSkin });
	auto pFiller2 = Filler::create({ .defaultSize = {100,100}, .skin = pSkin });
	auto pFiller3 = Filler::create({ .defaultSize = {100,100}, .skin = pSkin });


	pPackPanel->slots.pushBack({ pFiller1, pFiller2, pFiller3} );
	
	auto pSelectCapsule = SelectCapsule::create( { .selectMode = SelectMode::MultiEntries });

	pSelectCapsule->slot = pPackPanel;

	pBaseLayer->slots.pushBack(pSelectCapsule, { .pos = {20,20} });

	*pEntry = pBaseLayer;
	return true;

}

bool drawerPanelTest(ComponentPtr<DynamicSlot> pEntry)
{
	auto pBaseLayer = FlexPanel::create();
	pBaseLayer->setSkin(ColorSkin::create(Color::PapayaWhip));

	auto pPanelSkin = BoxSkin::create(BoxSkin::Blueprint{ .color = Color::PapayaWhip, .outlineColor = Color::Black, .outlineThickness = 1 });

	auto pHeaderSkin = BoxSkin::create(BoxSkin::Blueprint{ .color = Color::PapayaWhip, .outlineColor = Color::Black, .outlineThickness = 1,
		.states = { {State::Checked, HiColor::White }} });

	auto pDrawerPanel = DrawerPanel::create( { .skin = pPanelSkin } );

	auto pHeader = TextDisplay::create(TextDisplay::Blueprint{ .display = {.text = "EXPAND ME" } });
	auto pContent = Filler::create({ .defaultSize = {50,100}, .skin = pPanelSkin });

	pDrawerPanel->slots[0] = pHeader;
	pDrawerPanel->slots[1] = pContent;

	pBaseLayer->slots.pushBack( pDrawerPanel, {.pos = {10,10} });

	*pEntry = pBaseLayer;

	pDrawerPanel->setTransition(ValueTransition::create(500000));
	pDrawerPanel->setOpen(true);

	return true;

}
