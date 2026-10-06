
#include <cstdlib>
#include <stdio.h>

#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#		include <SDL2/SDL.h>
#		include <SDL2_image/SDL_image.h>
#	else
#		include <SDL2/SDL.h>
#		include <SDL2/SDL_image.h>
#	endif
#endif

#include <wondergui.h>

#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_glgfxdevice.h>
#include <wg_enumextras.h>
#include <wg_staticcolorskin.h>
#include <testwidget.h>


using namespace wg;

void 			translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot );
MouseButton 	translateMouseButton( Uint8 button );
void 			updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow );
void 			myButtonClickCallback( const Msg_p& pMsg );
void * 			loadFile( const char * pPath );
Blob_p 			loadBlob( const char * pPath );
void			convertSDLFormat( PixelDescription * pWGFormat, const SDL_PixelFormat * pSDLFormat );

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter );
Surface_p generateTestSurface();


bool	bQuit = false;


void freeSDLSurfCallback( void * pSDLSurf )
{
	SDL_FreeSurface( (SDL_Surface*) pSDLSurf );
}

InputHandler * pDebug;

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{ 

	struct Test1
	{
		bool a;
		int		i1;
		bool b;
		int		i2;
		bool c;
		bool d;
	};

	struct Test2
	{
		int		i1;
		int		i2;
		bool a;
		bool b;
		bool c;
		bool d;
	};

	int big = sizeof(Test1);
	int small = sizeof(Test2);

	printf("Big: %d. Small: %d\n", big, small);


	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 1920/2, height = 1080/2;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(  SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(  SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
    
    SDL_GLContext context = SDL_GL_CreateContext( pWin );

#ifdef WIN32  
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif

	IMG_Init( IMG_INIT_JPG | IMG_INIT_PNG );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	Base::init(nullptr);
//	Base::initFreeType();
//	VectorFont::setSurfaceFactory( GlSurfaceFactory::create() );

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

	

	GlGfxDevice_p pGfxDevice = GlGfxDevice::create( SizeI(width,height) );

	RootPanel_p pRoot = RootPanel::create( pGfxDevice );
	
	Base::inputHandler()->setFocusedWindow( pRoot );

	// 
	
	MsgLogger_p pLogger = MsgLogger::create( std::cout );
	pLogger->logAllMsgs();
	pLogger->ignoreMsg( MsgType::Tick );
	
	Base::msgRouter()->broadcastTo( pLogger );


	// Init font

	char * pFontSpec = (char*)loadFile("../resources/anuvverbubbla_8x8.fnt");

    assert( glGetError() == 0 );

    
	SDL_Surface * pFontSurf = IMG_Load( "../resources/anuvverbubbla_8x8.png" );
	GlSurface_p pFontImg = GlSurface::create( SizeI(pFontSurf->w,pFontSurf->h), PixelFormat::ARGB_8, (uint8_t*) pFontSurf->pixels, pFontSurf->pitch );
	SDL_FreeSurface( pFontSurf );
    assert( glGetError() == 0 );
	BitmapFont_p pBmpFont = BitmapFont::create( pFontImg, pFontSpec );
    assert( glGetError() == 0 );

//	Blob_p pFontFile = loadBlob("../resources/DroidSans.ttf");
	
//	VectorFont_p pFont = VectorFont::create( pFontFile, 1 );


	TextStyle_p pStyle = TextStyle::create();
	pStyle->setFont(pBmpFont);
	pStyle->setSize(10);
	Base::setDefaultStyle(pStyle);

	// Init skins

	PixelDescription	pixelDesc;

	SDL_Surface * pSDLSurf = IMG_Load( "../resources/simple_button.bmp" );
	convertSDLFormat( &pixelDesc, pSDLSurf->format );
	GlSurface_p pButtonSurface = GlSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelFormat::XRGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &pixelDesc );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pSimpleButtonSkin = BlockSkin::createClickableFromSurface( pButtonSurface, 0, BorderI(3) );
	pSimpleButtonSkin->setContentPadding( BorderI(5) );

	pSDLSurf = IMG_Load( "../resources/state_button.bmp" );
    convertSDLFormat( &pixelDesc, pSDLSurf->format );
	GlSurface_p pStateButtonSurface = GlSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelFormat::XRGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &pixelDesc );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pStateButtonSkin = BlockSkin::createClickSelectableFromSurface( pStateButtonSurface, 0, BorderI(3) );
	pStateButtonSkin->setContentPadding( BorderI(5) );
    
	pSDLSurf = IMG_Load( "../resources/grey_pressable_plate.bmp" );
	convertSDLFormat( &pixelDesc, pSDLSurf->format );
	GlSurface_p pPressablePlateSurface = GlSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelFormat::XRGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &pixelDesc );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pPressablePlateSkin = BlockSkin::createClickableFromSurface( pPressablePlateSurface, 0, BorderI(3) );
	pPressablePlateSkin->setContentPadding( BorderI(3) );
    
	pSDLSurf = IMG_Load( "../resources/list_entry.png" );
	convertSDLFormat( &pixelDesc, pSDLSurf->format );
	GlSurface_p pListEntrySurface = GlSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelFormat::XRGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &pixelDesc );
	SDL_FreeSurface( pSDLSurf );
	Skin_p pListEntrySkin = BlockSkin::createClickableFromSurface( pListEntrySurface, 0, BorderI(3) );
    
	pSDLSurf = IMG_Load( "../resources/frog.jpg" );
	convertSDLFormat( &pixelDesc, pSDLSurf->format );
	GlSurface_p pImgSurface = GlSurface::create( SizeI( pSDLSurf->w, pSDLSurf->h ), PixelFormat::XRGB_8, (unsigned char*) pSDLSurf->pixels, pSDLSurf->pitch, &pixelDesc );
	SDL_FreeSurface( pSDLSurf );
	BlockSkin_p pImgSkin = BlockSkin::createStaticFromSurface( pImgSurface, BorderI(3) );

	//------------------------------------------------------
	// Setup a simple GUI consisting of a filled background and 
	// a button using scaled bitmaps.
	//------------------------------------------------------

	FlexPanel_p pFlexPanel = FlexPanel::create();
	pFlexPanel->setSkin( StaticColorSkin::create(Color::Beige)/*pImgSkin*/  );
	pRoot->child = pFlexPanel;



	Image_p pImage = Image::create();
	pImage->setSkin( pSimpleButtonSkin );
	pFlexPanel->children.addMovable( pImage, RectI(0,0,80,33), Origo::Center );


//	pRoot->msgRouter()->AddCallback( MsgFilter::select(), pButton, myButtonClickCallback );


	// Test transparency issue



	{
		PackPanel_p pVert = PackPanel::create();
		PackPanel_p pHorr = PackPanel::create();
		
		Filler_p pFillerEast = Filler::create();
		Filler_p pFillerSouth = Filler::create();
		
		pFillerEast->setSkin( pPressablePlateSkin );
		pFillerSouth->setSkin( pPressablePlateSkin );
		
		pVert->setOrientation( Orientation::Vertical );
		
		pVert->children << pHorr;
		pVert->children << pFillerSouth;

		TextEditor_p pText = TextEditor::create();
		pText->setSkin( StaticColorSkin::create( Color::Black ) );
		
		
		TextStyle_p pBig = TextStyle::create();
		pBig->setSize( 16 );
		
		TextStyle_p pRed = TextStyle::create();
		pRed->setColor( Color::Red );
		pRed->setSize( 25 );

		TextStyle_p pAnuv = TextStyle::create();
		pAnuv->setFont( pBmpFont );

		TextStyle_p pLink = TextStyle::create();
		pLink->setColor(Color::Green );
		pLink->setLink( TextLink::create( "www.somewhere.net" ) );

		pText->text.set( "This is a\npiece of TEXT with LINK" );
		pText->text.setCharStyle( pBig, 5, 2 );
		pText->text.setCharStyle( pRed, 10, 3 );
		pText->text.setCharStyle( pAnuv, 19, 4 );
		pText->text.setCharStyle( pLink, 29, 4 );


		pHorr->children << pText;
		pHorr->children << pFillerEast;

		
//		pFlexPanel->addWidget( pVert, Origo::NorthWest, Origo::SouthEast );
	
		pText->grabFocus();
    }


	{
//		Image_p pImage = Image::create();
//		pImage->setImage( generateTestSurface() );
//		pFlexPanel->children.addPinned(pImage, { Origo::NorthWest,20,20 }, { Origo::SouthEast,-20,-20 } );
	}


/*
    {
        TestWidget_p pTest = TestWidget::create();
        pFlexPanel->addWidget( pTest, Origo::NorthWest, Origo::SouthEast, BorderI(20) );
        pTest->start();
    }
*/

/*	
	{
		FlexPanel_p pExtraFlex = FlexPanel::create();
		pExtraFlex->setSkin( ColorSkin::create( Color(0,0,0,128)));

		TextEditor_p pText = TextEditor::create();
		
		TextStyle_p pBig = TextStyle::create();
		pBig->setSize( 16 );
		
		TextStyle_p pRed = TextStyle::create();
		pRed->setColor( Color::Red );
		pRed->setSize( 25 );

		TextStyle_p pAnuv = TextStyle::create();
		pAnuv->setFont( pBmpFont );

		pText->text.set( "This is a\npiece of TEXT" );
		pText->text.setCharStyle( pBig, 5, 2 );
		pText->text.setCharStyle( pRed, 10, 3 );
		pText->text.setCharStyle( pAnuv, 19, 20 );
		
		pExtraFlex->addWidget( pText, RectI( 10,10,100,100) );

		addResizablePanel( pFlexPanel, pExtraFlex, Base::msgRouter() );

		pText->grabFocus();

		translateEvents( pInput, pRoot );

		pText->text.caretTextBegin();
		pText->text.caretLineEnd();
	}
*/



	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

    int tick = 0;

	/*
		I need the following crap on my Windows dev machine to
		make something that resembles single buffering work :(

		Apparently single buffering in SDL2/Windows doesn't work on my dev machine, 
		so I get double buffering although I specify single buffering when initializing.
		(don't know if it is SDL's, Windows' or crappy device drivers fault, 
		it works fine in SDL2/Linux on same hardware)

		So I render to the front buffer instead, but apparently nothing will be displayed
		until the rendering pipeline has been filled and flushed twice... so we just clear the
		color buffer and flush() to make sure that everything is displayed on first render call.
	*/

	glDrawBuffer(GL_FRONT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );
	glFlush();

	auto pMyCanvas = GlSurface::create({ 400,400 }, PixelFormat::XRGB_8);
//	pMyCanvas->setScaleMode(ScaleMode::Interpolate);
	pMyCanvas->setScaleMode(ScaleMode::Nearest);


	while( !bQuit ) 
	{

		translateEvents( pInput, pRoot );

//		pGfxDevice->setCanvas(pMyCanvas);

//		pRoot->render();

//		pGfxDevice->setCanvas({ 400,400 });

		pGfxDevice->beginRender();

//        pGfxDevice->setClipList(1, &clipRect)
        
		pImgSkin->render(pGfxDevice, { 0,0,1920,1080 }, StateEnum::Normal );

//		pGfxDevice->drawWave({ 0,500 }, 1900, &topLine, &bottomLine, { 0,0,255,128 }, Color::Yellow);

//		pGfxDevice->stretchBlitSubPixelWithInvert(pMyCanvas, 0,0,400,400, 0,0, 200, 200 );
//		pGfxDevice->blit(pMyCanvas, { 0,0,400,400 }, { 0,0 });
//		pGfxDevice->stretchBlit(pMyCanvas, { 0,0,400,400 }, { 0,0,200,200 });
		pGfxDevice->endRender();


//		updateWindowRects( pRoot, pWin );

 //       glClearColor(0.1f, 0.3f, 0.3f, 1.0f );
 //       glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//		glFlush();
/*
         pGfxDevice->beginRender();

//        pGfxDevice->setTintColor(Color::Red );
        
//        pGfxDevice->fill( RectI(1,1,320-2,400-2), Color::Aquamarine);

        
//       pGfxDevice->wg::GfxDevice::blit( pButtonSurface, 50, 50 );
//        pGfxDevice->wg::GfxDevice::stretchBlit(pButtonSurface, RectI(50,50,160,40));

        

        Color plotColors[400];
        CoordI plotCoords[400];
        
        for( int i = 0 ; i < 400 ; i++ )
        {
            plotColors[i] = Color::Red;
            plotCoords[i] = CoordI(i,i);
        }
        
        
//       pGfxDevice->clipPlotPixels( RectI(0,0,width,height), 100, plotCoords, plotColors);

        
//        for(int x = 0 ; x < 200 ; x++ )
//        {
//            pGfxDevice->clipDrawVertLine(RectI(0,0,width,height), CoordI(x*2,x), 40, Color::Aquamarine);
//        }

        
//        pGfxDevice->clipDrawHorrLine(RectI(10,10,width-20,height-20), CoordI(0,20), 800, Color::Aquamarine);
//        pGfxDevice->clipDrawVertLine(RectI(10,10,width-20,height-20), CoordI(20,0), 800, Color::Aquamarine);

        
        
//        pGfxDevice->drawLine( CoordI(10,10), CoordI(200,40), Color::Red, 3.f );
        
//        pGfxDevice->fill(RectI(21, 21, 100, 100 ), Color::AntiqueWhite );
//        pGfxDevice->fillSubPixel(RectF(21.8f, 21.5f, 0.4f, 0.4f ), Color::AntiqueWhite );
        
        pGfxDevice->endRender();
*/        
		SDL_Delay(20);
        tick++;
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
//				if( e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED )
//					distance *= -1;
			
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
	size_t size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	Blob_p pBlob = Blob::create( int(size) );
		
    size_t nRead = fread( pBlob->data(), 1, size, fp );
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


Coord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const Msg_p& _pMsg, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);

	dragStartPos = pWidget->pos();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const Msg_p& _pMsg, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);
	
	if( _pMsg->type() != MsgType::MouseDrag || !pWidget->parent() )
		return;

	const MouseDragMsg_p pMsg = MouseDragMsg::cast(_pMsg);



	CoordI	dragDistance = pMsg->draggedTotal();

	CoordI	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	FlexPanel_p pFlex = FlexPanel::cast(pWidget->parent());
	int index = pFlex->children.index(pWidget);

	pFlex->children.move(index, dragStartPos + dragDistance);
}

//____ cbMoveResize() _________________________________________________________

void cbMoveResize( const Msg * _pMsg )
{
	static CoordI posAtPress[(int)MouseButton_max];
	
//	Widget_p _pWidget = _pMsg->source();
	Object_p _pWidget = _pMsg->source();

	auto	pWidget = Widget::cast(_pWidget);
	auto	pParent = FlexPanel::cast(pWidget->parent());

	int index = pParent->children.index(pWidget);

	switch( _pMsg->type() )
	{
		case MsgType::MousePress:
		{
			auto pMsg = static_cast<const MousePressMsg*>(_pMsg);
			posAtPress[(int)pMsg->button()] = pWidget->pos();
			 
		}
		break;
		case MsgType::MouseDrag:
		{
			auto pMsg = static_cast<const MouseDragMsg*>(_pMsg);
			if( pMsg->button() == MouseButton::Right )
			{
				pParent->children.setSize( index, pWidget->size() + pMsg->draggedNow().toSize() );
			}
			else if( pMsg->button() == MouseButton::Middle )
			{
				pParent->children.setOfs( index, posAtPress[(int)MouseButton::Middle] + pMsg->draggedTotal() );
			}
		}
		break;
		case MsgType::MouseRelease:
		break;
		
		default:
		break;
	}
}


//____ addResizablePanel() _________________________________________________

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter )
{
	pParent->children.add(pChild);

//	pHook->setSizePolicy(SizePolicy::Bound, SizePolicy::Bound);

	pMsgRouter->addRoute( pChild, cbMoveResize );
}

//____ convertSDLFormat() ______________________________________________________

void convertSDLFormat( PixelDescription * pWGFormat, const SDL_PixelFormat * pSDLFormat )
{
	pWGFormat->format = PixelFormat::Custom;
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

//____ generateTestSurface () _____________________________________________________

Surface_p generateTestSurface()
{
	uint8_t tex[3*3*4] = {	255,0,0, 0,255,0, 0,0,255, 0,0,
								255,128,128, 128,255,128, 128,128,255,0,0,
								255,128,0, 128,255,0, 0,128,255,0,0 };

	Surface_p p = GlSurface::create( SizeI(3,3), PixelFormat::XRGB_8, tex, 11 );
	p->setScaleMode(ScaleMode::Interpolate);
	return p;
}

