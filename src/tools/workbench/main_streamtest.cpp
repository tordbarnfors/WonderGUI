
#include <cstdlib>
#include <stdio.h>

#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#endif

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>

#include <wondergui.h>

#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wg_softgfxdevice.h>
#include <wg_streamsurface.h>
#include <wg_streamsurfacefactory.h>
#include <wg_streamgfxdevice.h>

#include <wax_freetypefont.h>
#include <wg_packlist.h>
#include <testwidget.h>
#include <wg_popupopener.h>

using namespace wg;
using namespace std;

void 			translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot );
MouseButton 	translateMouseButton( Uint8 button );
void 			updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow );
void 			myButtonClickCallback( const Msg_p& pMsg );
void * 			loadFile( const char * pPath );
Blob_p 			loadBlob( const char * pPath );
void			convertSDLFormat( PixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat );

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter );

void playDelayFrames(GfxDevice_p pDevice, int nFrames);

void playRectangleDance(GfxDevice_p pDevice, RectI canvas);

void playInitButtonRow(GfxDevice_p pDevice, RectI canvas);
void playButtonPress(GfxDevice_p pDevice, int button);
void playButtonRelease(GfxDevice_p pDevice, int button);
void playSetSlider(GfxDevice_p pDevice, float percentage);


Coord positionSprite(SizeI dimensions, int tick, int nb, int amount);



bool	bQuit = false;


void freeSDLSurfCallback( void * pSDLSurf )
{
	SDL_FreeSurface( (SDL_Surface*) pSDLSurf );
}

InputHandler * pDebug;

int sortWidgets( const Widget * p1, const Widget * p2 )
{
	return p2->id() - p1->id();
}


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{ 
	printf( "SizeOf Filler: %d\n", sizeof(Filler) );
	printf("SizeOf Object: %d\n", sizeof(Object));
	printf("SizeOf Receiver: %d\n", sizeof(Receiver));

	printf( "Slot is safe to relocate: %d\n", Slot::safe_to_relocate );
	printf( "PackListSlot is safe to relocate: %d\n", PackListSlot::safe_to_relocate);
	printf("LambdaPanelSlot is safe to relocate: %d\n", LambdaPanelSlot::safe_to_relocate);


	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 640, height = 480;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG );
	 
	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	Base::init(nullptr);
//	FreeTypeFont::init( SoftSurfaceFactory::create() );

	InputHandler_p pInput = Base::inputHandler();
	
	pInput->mapKey( SDLK_LEFT, Key::Left );
	pInput->mapKey( SDLK_RIGHT, Key::Right );
	pInput->mapKey( SDLK_UP, Key::Up );
	pInput->mapKey( SDLK_DOWN, Key::Down );
	pInput->mapKey( SDLK_BACKSPACE, Key::Backspace );
	pInput->mapKey( SDLK_DELETE, Key::Delete );
	pInput->mapKey( SDLK_END, Key::End );
	pInput->mapKey( SDLK_ESCAPE, Key::Escape );
	pInput->mapKey( SDLK_HOME, Key::Home );
	pInput->mapKey( SDLK_PAGEDOWN, Key::PageDown );
	pInput->mapKey( SDLK_PAGEUP, Key::PageUp );
	pInput->mapKey( SDLK_RETURN, Key::Return );
	pInput->mapKey( SDLK_SPACE, Key::Space );
	pInput->mapKey( SDLK_TAB, Key::Tab );
	pInput->mapKey( SDLK_F1, Key::F1 );
	pInput->mapKey( SDLK_F2, Key::F2 );
	pInput->mapKey( SDLK_F3, Key::F3 );
	pInput->mapKey( SDLK_F4, Key::F4 );
	pInput->mapKey( SDLK_F5, Key::F5 );
	pInput->mapKey( SDLK_F6, Key::F6 );
	pInput->mapKey( SDLK_F7, Key::F7 );
	pInput->mapKey( SDLK_F8, Key::F8 );
	pInput->mapKey( SDLK_F9, Key::F9 );
	pInput->mapKey( SDLK_F10, Key::F10 );
	pInput->mapKey( SDLK_F11, Key::F11 );
	pInput->mapKey( SDLK_F12, Key::F12 );
	
	
	pInput->mapKey( SDLK_LCTRL, Key::Control );
	pInput->mapKey( SDLK_RCTRL, Key::Control );

	pInput->mapKey( SDLK_LSHIFT, Key::Shift );
	pInput->mapKey( SDLK_RSHIFT, Key::Shift );

	pInput->mapKey( SDLK_LALT, Key::Alt );
	pInput->mapKey( SDLK_RALT, Key::Alt );
	
	pInput->mapKey( SDLK_KP_ENTER, Key::Return );
	
	
	pInput->mapCommand( SDLK_ESCAPE, MODKEY_NONE, EditCmd::Escape );

	pInput->mapCommand( SDLK_x, MODKEY_CTRL, EditCmd::Cut );
	pInput->mapCommand( SDLK_c, MODKEY_CTRL, EditCmd::Copy );
	pInput->mapCommand( SDLK_v, MODKEY_CTRL, EditCmd::Paste );

	pInput->mapCommand( SDLK_a, MODKEY_CTRL, EditCmd::SelectAll );
	
	pInput->mapCommand( SDLK_z, MODKEY_CTRL, EditCmd::Undo );
	pInput->mapCommand( SDLK_z, MODKEY_CTRL_SHIFT, EditCmd::Redo );


	PixelType type = PixelType::Unknown;

	if( pWinSurf->format->BitsPerPixel == 32 )
		type = PixelType::BGRA_8;
	else if( pWinSurf->format->BitsPerPixel == 24 )
		type = PixelType::BGR_8;
		
	Blob_p pCanvasBlob = Blob::create( pWinSurf->pixels, 0);	
	SoftSurface_p pCanvas = SoftSurface::create( SizeI(pWinSurf->w,pWinSurf->h), type, pCanvasBlob, pWinSurf->pitch );

	SoftGfxDevice_p pGfxDevice = SoftGfxDevice::create( pCanvas );

	RootPanel_p pRoot = RootPanel::create( pGfxDevice );

	pRoot->setDebugMode(true);
	
	Base::inputHandler()->setFocusedWindow( pRoot );

	// 
	
	MsgLogger_p pLogger = MsgLogger::create( std::cout );
	pLogger->logAllMsgs();
	pLogger->ignoreMsg( MsgType::Tick );
	
//	Base::msgRouter()->broadcastTo( pLogger );

	// Init font
/*
	PixelFormat	format;

	char * pFontSpec = (char*)loadFile("../resources/anuvverbubbla_8x8.fnt");

	SDL_Surface * pFontSurf = IMG_Load( "../resources/anuvverbubbla_8x8.png" );
//	convertSDLFormat( &format, pFontSurf->format );
	SoftSurface_p pFontImg = SoftSurface::create( SizeI(pFontSurf->w,pFontSurf->h), PixelType::BGRA_8, (unsigned char*) pFontSurf->pixels, pFontSurf->pitch);
	SDL_FreeSurface( pFontSurf );
		
	BitmapFont_p pBmpFont = BitmapFont::create( pFontImg, pFontSpec );


	Blob_p pFontFile = loadBlob("../resources/DroidSans.ttf");
	
	FreeTypeFont_p pFont = FreeTypeFont::create( pFontFile, 1 );


	TextStyle_p pStyle = TextStyle::create();
	pStyle->setFont(pFont);
	pStyle->setSize(16);
	Base::setDefaultStyle(pStyle);
*/
/*
	StdTextMapper_p pMapper = StdTextMapper::create();
	pMapper->setSelectionBackColor(Color(0,0,255,128), BlendMode::Blend);
	Base::setDefaultTextMapper(pMapper);
*/

//	StdTextMapper::cast(Base::defaultTextMapper())->setSelectionBack(Color(255,255,255,255), BlendMode::Invert);

	// Init skins

/*

	SDL_Surface * pSDLSurf = IMG_Load( "../resources/simple_button.bmp" );
	convertSDLFormat( &format, pSDLSurf->format );
	SoftSurface_p pButtonSurface = SoftSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelType::BGR_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &format );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pSimpleButtonSkin = BlockSkin::createClickableFromSurface( pButtonSurface, 0, BorderI(3) );
	pSimpleButtonSkin->setContentPadding( BorderI(5) );

	pSDLSurf = IMG_Load( "../resources/splash.png" );
	convertSDLFormat( &format, pSDLSurf->format );
	SoftSurface_p pSplashSurface = SoftSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelType::BGR_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &format );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pSplashSkin = BlockSkin::createStaticFromSurface( pSplashSurface );
*/

	//------------------------------------------------------
	// Setup a simple GUI consisting of a filled background and 
	// a button using scaled bitmaps.
	//------------------------------------------------------

	PopupLayer_p pPopupLayer = PopupLayer::create();
	pRoot->child = pPopupLayer;



	LambdaPanel_p pBasePanel = LambdaPanel::create();
	pBasePanel->setSkin( ColorSkin::create(Color::Burlywood) );
	pPopupLayer->base = pBasePanel;




/*

   {
        TestWidget_p pTest = TestWidget::create();
		pBasePanel->children.add(pTest, [](Widget* pWidget, SizeI size) { return RectI(10, 10, 100, 100); });
        pTest->start();
    }
	*/

	//------------------------------------------------------
	// Setup streaming
	//------------------------------------------------------

	auto pStreamPlug = GfxStreamPlug::create();

	auto pStreamDevice = StreamGfxDevice::create({ width,height }, pStreamPlug->input.ptr());
	auto pSurfaceFactory = StreamSurfaceFactory::create(pStreamPlug->input.ptr());

	pStreamPlug->openOutput(0);
	auto pStreamLogger = GfxStreamLogger::create(pStreamPlug->output[0], std::cout);

	pStreamPlug->openOutput(1);
	auto pGfxPlayer = GfxStreamPlayer::create(pStreamPlug->output[1], pGfxDevice, SoftSurfaceFactory::create());


	//------------------------------------------------------
	// Setup streaming
	//------------------------------------------------------
/*
	char * pBigBuffer = new char[10000000];

	char * pWrite = pBigBuffer;
	char * pRead = pBigBuffer;


	GfxStreamWriter_p pWriter = GfxStreamWriter::create([&pWrite](int bytes, const void* pData) { std::memcpy(pWrite, pData, bytes); pWrite += bytes; });
	GfxStreamReader_p pReader = GfxStreamReader::create([&pRead,&pWrite](int bytes, void * pDest) 
	{ 
		int nBytes = std::min((int)(pWrite - pRead), bytes);
		std::memcpy(pDest, pRead, nBytes); 
		pRead += nBytes; 
		return nBytes; 
	});

	StreamGfxDevice_p pStreamDevice = StreamGfxDevice::create({ width,height }, pWriter->stream.ptr());

	GfxStreamPlayer_p pGfxPlayer = GfxStreamPlayer::create(pReader->stream, pGfxDevice, SoftSurfaceFactory::create());
*/
	//------------------------------------------------------
	// Record stream
	//------------------------------------------------------
/*
	char * pWaxBeg = pWrite;

	playInitButtonRow(pStreamDevice, { 0,0,width,height });
	ofstream save("indicators_init.wax", ios::out | ios::binary | ios::trunc);
	save.write(pWaxBeg, pWrite - pWaxBeg);
	save.close();

	{
		char * pWaxBeg = pWrite;
		playSetSlider(pStreamDevice, 1.f);
		ofstream save("set_slider.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
*/	
	/*
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonPress(pStreamDevice, 0);
		ofstream save("buttonrow_press_1.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonPress(pStreamDevice, 1);
		ofstream save("buttonrow_press_2.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonPress(pStreamDevice, 2);
		ofstream save("buttonrow_press_3.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonPress(pStreamDevice, 3);
		ofstream save("buttonrow_press_4.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonRelease(pStreamDevice, 0);
		ofstream save("buttonrow_release_1.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonRelease(pStreamDevice, 1);
		ofstream save("buttonrow_release_2.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonRelease(pStreamDevice, 2);
		ofstream save("buttonrow_release_3.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
	playDelayFrames(pStreamDevice, 100);
	{
		char * pWaxBeg = pWrite;
		playButtonRelease(pStreamDevice, 3);
		ofstream save("buttonrow_release_4.wax", ios::out | ios::binary | ios::trunc);
		save.write(pWaxBeg, pWrite - pWaxBeg);
		save.close();
	}
*/
	//------------------------------------------------------
	// Save stream to file
	//------------------------------------------------------
/*
	ofstream save("indicators_init.wax", ios::out | ios::binary | ios::trunc);
	save.write(pWaxBeg, pWrite - pWaxBeg);
	save.close();
*/

	//------------------------------------------------------
	// Setup graphics
	//------------------------------------------------------

//	auto pSoftSurfaceFactory = SoftSurfaceFactory::create();


	PixelFormat	format;

	auto pSDLSurf = IMG_Load("../resources/splash.png");
	convertSDLFormat(&format, pSDLSurf->format);
	Surface_p pSplashSurface = pSurfaceFactory->createSurface(SizeI(pSDLSurf->w, pSDLSurf->h), PixelType::BGRA_8, (unsigned char*)pSDLSurf->pixels, pSDLSurf->pitch, &format);
	SDL_FreeSurface(pSDLSurf);




	pStreamDevice->beginRender();
	pStreamDevice->fill({ 0,0,width,height }, Color::Black);
	pStreamDevice->fill({ 10,10,100,100 }, Color::Red);
	pStreamDevice->clipBlit({ 0,0,width,height }, pSplashSurface, pSplashSurface->size(), { 120,10 });
	pStreamDevice->endRender();

	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	while( !bQuit )
	{
		translateEvents( pInput, pRoot );

		SDL_LockSurface(pWinSurf);
//		pRoot->render();
 /*
		pDevice->beginRender();

		pDevice->fill({ 0,0,width,height }, Color::Black);
		pDevice->fill({ positionSprite(moveDim, ticker, 0, 3),spriteSize }, Color::Red);
		pDevice->fill({ positionSprite(moveDim, ticker, 1, 3),spriteSize }, Color::Green);
		pDevice->fill({ positionSprite(moveDim, ticker, 2, 3),spriteSize }, Color::Blue);

		pDevice->setBlendMode(BlendMode::Invert);
		pDevice->fill({ 0,0,width/2,height/2 }, Color::White);
		pDevice->setBlendMode(BlendMode::Blend);

		pDevice->drawLine({ 20,20 }, { width - 20,height - 20 }, Color::Beige, 5.f);
		pDevice->clipDrawLine({ 20,128,100,100 }, { 20, 120 }, { width - 20,height - 20 + 100 }, Color::Purple, 20.f);

		pDevice->clipDrawHorrLine({ 0,0,width,height }, { 400,400 }, 50, Color::LawnGreen );
		pDevice->clipDrawVertLine({ 0,0,width,height }, { 400,410 }, 50, Color::LawnGreen);

		pDevice->fillSubPixel({ 10.5f,500.3f,50.f,49.5f }, Color::CadetBlue);

		pDevice->endRender();
*/

		pGfxPlayer->playAll();
/*
		pGfxDevice->beginRender();
		pGfxDevice->fill({ 0,0,width,height }, Color::Black);
		pGfxDevice->fill({ 10,10,100,100 }, Color::Red);
		pGfxDevice->clipBlit({ 0,0,width,height }, pSplashSurface, pSplashSurface->size(), { 120,10 });
		pGfxDevice->endRender();
*/

		SDL_UnlockSurface(pWinSurf);

		SDL_Rect	r;
		r.x = 0;
		r.y = 0;
		r.w = width;
		r.h = height;
		SDL_UpdateWindowSurfaceRects(pWin, &r, 1);


//		updateWindowRects( pRoot, pWin );

		pStreamLogger->logAll();

		SDL_Delay(10);
    }

	// Exit WonderGUI

	Base::exit();

	// Exit SDL

	IMG_Quit();
	SDL_Quit();

    return 0;
}


//____ translateEvents() ___________________________________________________________

void translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot )
{
	// WonderGUI needs Tick-events to keep track of time passed for things such
	// key-repeat, double-click detection, animations etc.  So we create one
	// and put it on the event queue.
	
	static unsigned int oldTicks = 0;
	
	unsigned int ticks = SDL_GetTicks();
	int tickDiff;

	if( oldTicks == 0 )
		tickDiff = 0;
	else
		tickDiff = (int) (ticks - oldTicks);		
	oldTicks = ticks;

	Base::msgRouter()->post( TickMsg::create(ticks, tickDiff) );

	// Process all the SDL events in a loop

	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		
		switch( e.type )
		{
			case SDL_QUIT:
				bQuit = true;
				break;
				
			case SDL_MOUSEMOTION:
				pInput->setPointer( pRoot, CoordI(e.motion.x,e.motion.y) );
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				pInput->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_MOUSEBUTTONUP:
				pInput->setButton( translateMouseButton(e.button.button), false );
				break;
				
			case SDL_MOUSEWHEEL:
			{
				CoordI distance( e.wheel.x, e.wheel.y );
				if( e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED )
					distance *= -1;
			
				pInput->setWheelRoll( 1, distance );
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
	
	const RectI * pUpdatedRects = pRoot->firstUpdatedRect();
	SDL_Rect * pSDLRects = (SDL_Rect*) Base::memStackAlloc( sizeof(SDL_Rect) * nRects );

	for( int i = 0 ; i < nRects ; i++ )
	{
		pSDLRects[i].x = pUpdatedRects[i].x;
		pSDLRects[i].y = pUpdatedRects[i].y;
		pSDLRects[i].w = pUpdatedRects[i].w;
		pSDLRects[i].h = pUpdatedRects[i].h;
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
	FILE * fp = fopen( pPath, "rb" );
	if( !fp )
		return 0;

	fseek( fp, 0, SEEK_END );
	int size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	Blob_p pBlob = Blob::create( size );
		
	int nRead = fread( pBlob->data(), 1, size, fp );
	fclose( fp );

	if( nRead < size )
		return 0;

	return pBlob;

}


//____ loadFile() _____________________________________________________________

void * loadFile( const char * pPath )
{
	FILE * fp = fopen( pPath, "rb" );
	if( !fp )
		return 0;

	fseek( fp, 0, SEEK_END );
	int size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	char * pMem = (char*) malloc( size+1 );
	pMem[size] = 0;
	int nRead = fread( pMem, 1, size, fp );
	fclose( fp );

	if( nRead < size )
	{
		free( pMem );
		return 0;
	}

	return pMem;

}

//____ convertSDLFormat() ______________________________________________________

void convertSDLFormat( PixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat )
{
	pWGFormat->type = PixelType::Custom;
	pWGFormat->bits = pSDLFormat->BitsPerPixel;

	pWGFormat->R_mask = pSDLFormat->Rmask;
	pWGFormat->G_mask = pSDLFormat->Gmask;
	pWGFormat->B_mask = pSDLFormat->Bmask;
	pWGFormat->A_mask = pSDLFormat->Amask;

	pWGFormat->R_shift = pSDLFormat->Rshift;
	pWGFormat->G_shift = pSDLFormat->Gshift;
	pWGFormat->B_shift = pSDLFormat->Bshift;
	pWGFormat->A_shift = pSDLFormat->Ashift;

	pWGFormat->R_bits = 8 - pSDLFormat->Rloss;
	pWGFormat->G_bits = 8 - pSDLFormat->Gloss;
	pWGFormat->B_bits = 8 - pSDLFormat->Bloss;
	pWGFormat->A_bits = 8 - pSDLFormat->Aloss;
}

//____ playRectangleDance() _________________________________________________

void playRectangleDance(GfxDevice_p pDevice, RectI canvas )
{
	int ticker = 0;
	SizeI spriteSize(100, 100);
	SizeI moveDim(canvas.w - spriteSize.w, canvas.h - spriteSize.h);

	while (ticker < 600)
	{
		pDevice->beginRender();

		pDevice->fill( canvas, Color::Black);
		pDevice->fill({ canvas.pos() + positionSprite(moveDim, ticker, 0, 3),spriteSize }, Color::Red);
		pDevice->fill({ canvas.pos() + positionSprite(moveDim, ticker, 1, 3),spriteSize }, Color::Green);
		pDevice->fill({ canvas.pos() + positionSprite(moveDim, ticker, 2, 3),spriteSize }, Color::Blue);

		pDevice->endRender();
		ticker++;
	}
}

Coord positionSprite(SizeI dimensions, int tick, int nb, int amount)
{
	const float PI = 3.14159265f;

	CoordI	radius = { dimensions.w / 2, dimensions.h / 2 };

	if (tick < 90)
		radius *= sin(tick*PI / 180);

	if (tick > 600 - 90)
		radius *= 1.f - sin((tick - (600 - 90))*PI / 180);


	CoordI c;
	c.x = (int)(cos((tick + nb*360.f / amount)*PI / 180)*radius.x + dimensions.w / 2);
	c.y = (int)(sin((tick + nb*360.f / amount)*PI / 180)*radius.y + dimensions.h / 2);
	return c;
}

//____ play general stuff ________________________________________________

void playDelayFrames(GfxDevice_p pDevice, int nFrames)
{
	for (int i = 0; i < nFrames; i++)
	{
		pDevice->beginRender();
		pDevice->endRender();
	}
}


//____ button row play functions ______________________________________________


Rect	buttonRect;
Coord	buttonPitch;
Rect	sliderRect;

void playInitButtonRow(GfxDevice_p pDevice, RectI canvas)
{
	// Init lamps

	buttonRect = { canvas.w / 32, canvas.h / 32,canvas.size() / 4 - canvas.size() / 16 };
	buttonPitch = { canvas.w / 4, 0 };

	pDevice->beginRender();

	pDevice->fill({ 0,0,canvas.w,canvas.h / 4 }, Color::LightGray);

	RectI r = buttonRect;
	for (int i = 0; i < 4; i++)
	{
		pDevice->fill(r, Color::White);
		pDevice->fill(r - BorderI(4), Color::DarkBlue);
		r += buttonPitch;
	}

	// Init slider

	sliderRect = { canvas.w / 32,canvas.h / 4, 263, canvas.h / 8 - canvas.h / 32 };

	pDevice->fill({0, canvas.h / 4, canvas.w, canvas.h / 8}, Color::LightGray);

	pDevice->fill( sliderRect, Color::White);
	sliderRect -= BorderI(4);
	pDevice->fill( sliderRect, Color::DarkBlue);

	pDevice->endRender();
}

void playButtonPress(GfxDevice_p pDevice, int button)
{
	pDevice->beginRender();
	pDevice->fill(buttonRect + buttonPitch*button - BorderI(4), Color::LightBlue);
	pDevice->endRender();
}

void playButtonRelease(GfxDevice_p pDevice, int button)
{
	pDevice->beginRender();
	pDevice->fill(buttonRect + buttonPitch*button - BorderI(4), Color::DarkBlue);
	pDevice->endRender();
}
void playSetSlider(GfxDevice_p pDevice, float percentage)
{
	int w = (int) (sliderRect.w*percentage);

	pDevice->beginRender();
	pDevice->fill( sliderRect, Color::DarkBlue);
	pDevice->fill({ sliderRect.x,sliderRect.y,w,sliderRect.h }, Color::Yellow);
	pDevice->endRender();

}

