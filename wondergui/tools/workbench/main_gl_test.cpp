
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

#include <wax_glsurface.h>
#include <wax_glsurfacefactory.h>
#include <wax_glgfxdevice.h>
#include <testwidget.h>


using namespace wg;
using namespace wax;

void 			translateEvents( const InputHandler_p& pInput, const RootPanel_p& pRoot );
MouseButton 	translateMouseButton( Uint8 button );
void 			updateWindowRects( const RootPanel_p& pRoot, SDL_Window * pWindow );
void 			myButtonClickCallback( const Msg_p& pMsg );
void * 			loadFile( const char * pPath );
Blob_p 			loadBlob( const char * pPath );
void			convertSDLFormat( PixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat );

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
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 400, height = 400;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(  SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(  SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
    
    SDL_GLContext context = SDL_GL_CreateContext( pWin );

#ifdef WIN32  
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
#endif

	glDrawBuffer(GL_FRONT);
	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

    int tick = 0;
    
	while( !bQuit ) 
	{
		SDL_Event e;
		while (SDL_PollEvent(&e)) {

			switch (e.type)
			{
			case SDL_QUIT:
				bQuit = true;
				break;
			default:
				break;
			}
		}

        glClearColor(0.1f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//		SDL_GL_SwapWindow(pWin);
		glFlush();
//		glFinish();
        
		SDL_Delay(20);
        tick++;
    }

	// Exit WonderGUI
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(pWin);
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
	int size = ftell(fp);
	fseek( fp, 0, SEEK_SET );

	Blob_p pBlob = Blob::create( size );
		
	int nRead = fread( pBlob->content(), 1, size, fp );
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


Coord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const Msg_p& _pMsg, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());


	dragStartPos = pHook->floatOfs();
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

	FlexHook_p pHook = FlexHook::cast(pWidget->hook());
	pHook->setOfs(dragStartPos+dragDistance);
}

//____ cbMoveResize() _________________________________________________________

void cbMoveResize( const Msg_p& _pMsg, const Object_p& _pWidget )
{
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
}


//____ addResizablePanel() _________________________________________________

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const MsgRouter_p& pMsgRouter )
{
	FlexHook * pHook = pParent->addWidget( pChild );
	pHook->setSizePolicy(SizePolicy::Bound, SizePolicy::Bound);

	pMsgRouter->addRoute( pChild, MsgFunc::create(cbMoveResize, pChild) );
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

//____ generateTestSurface () _____________________________________________________

Surface_p generateTestSurface()
{
	uint8_t tex[3*3*4] = {	255,0,0, 0,255,0, 0,0,255, 0,0,
								255,128,128, 128,255,128, 128,128,255,0,0,
								255,128,0, 128,255,0, 0,128,255,0,0 };

	Surface_p p = GlSurface::create( SizeI(3,3), PixelType::BGR_8, tex, 11 );
	p->setScaleMode(ScaleMode::Interpolate);
	return p;
}

