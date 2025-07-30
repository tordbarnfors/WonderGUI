

#ifdef WIN32
#	include <SDL.h>
#else
#	include <SDL2/SDL.h>
#endif

#include <wg_c_gfxtypes.h>
#include <wg_c_memheap.h>
#include <wg_c_gfxbase.h>
#include <wg_c_blob.h>
#include <wg_c_geo.h>
#include <wg_c_object.h>
#include <wg_c_softgfx.h>
#include <wg_c_gfxdevice.h>
#include <wg_c_softkernels_default.h>

#include "cpp_necessities.h"

void 			translateEvents();
void 			updateWindowRects( int nRects, wg_rect * pRects, SDL_Window * pWindow );

int				bQuit = 0;	// Set to false by myButtonClickCallback() or translateEvents().


#define HEAPSIZE 4000000

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 240, height = 240;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	void * pMemHeap = malloc( HEAPSIZE );

	wg_obj heap = wg_initMemHeap( pMemHeap, HEAPSIZE );

 //   init_cpp_necessities(heap); 

	wg_initGfxBase();



	wg_pixelFormat format = WG_PIXFMT_UNDEFINED;

	if( pWinSurf->format->BitsPerPixel == 32 )
		format = WG_PIXFMT_BGRA_8;
	else if( pWinSurf->format->BitsPerPixel == 24 )
		format = WG_PIXFMT_BGR_8;

	wg_sizeI canvasSize;
	canvasSize.w = pWinSurf->w;
	canvasSize.h = pWinSurf->h;

	SDL_LockSurface(pWinSurf);

    wg_obj hBlob = wg_createBlobFromData( pWinSurf->pixels, 0, NULL );
	wg_obj hCanvas = wg_createSoftSurfaceFromBlob( &(wg_surfaceBP){ .canvas = 1, .size = canvasSize, .format = format }, hBlob, pWinSurf->pitch );
	wg_release( hBlob );

	SDL_UnlockSurface(pWinSurf);

	wg_obj hBackend = wg_createSoftBackend();
	wg_addDefaultSoftKernels(hBackend);

	wg_obj hGfxDevice = wg_createGfxDevice(hBackend);

	//------------------------------------------------------
	

	wg_sizeI imgSize;
	
	// First we load the 24-bit bmp containing the button graphics.
	// No error handling or such to keep this example short and simple.

	SDL_Surface * pLogoSDLSurf = SDL_LoadBMP( "wondergui.bmp" );

	imgSize.w = pLogoSDLSurf->w;
	imgSize.h = pLogoSDLSurf->h;


	hBlob = wg_createBlobFromData( pLogoSDLSurf->pixels, 0, NULL );
	wg_obj hLogoSurface = wg_createSoftSurfaceFromBlob( &(wg_surfaceBP){ .size = imgSize, .format = WG_PIXFMT_BGRX_8 }, hBlob, pLogoSDLSurf->pitch );
    wg_release(hBlob);


	
	SDL_Surface * pSymbolSDLSurf = SDL_LoadBMP( "wg-symbol.bmp" );

	imgSize.w = pSymbolSDLSurf->w;
	imgSize.h = pSymbolSDLSurf->h;

    hBlob = wg_createBlobFromData( pSymbolSDLSurf->pixels, 0, NULL );
	wg_obj hSymbolSurface = wg_createSoftSurfaceFromBlob( &(wg_surfaceBP){ .size = imgSize, .format = WG_PIXFMT_BGRX_8 }, hBlob, pSymbolSDLSurf->pitch );
    wg_release(hBlob);

	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	while( bQuit == 0 )
	{
		// Loop through SDL events, translate them to WonderGUI events
		// and process them.

		translateEvents();

		// Let WonderGUI perform periodic updates

//		Base::update(int64_t(SDL_GetTicks()) * 1000);

		// Let WonderGUI render any updated/dirty regions of the screen.

		SDL_LockSurface(pWinSurf);

		wg_beginRender( hGfxDevice );

		wg_beginCanvasUpdateWithSurface( hGfxDevice, hCanvas, 0, NULL, NULL, -1 );


		wg_color fillColor;
		fillColor.r = 4096;
		fillColor.g = 4096;
		fillColor.b = 4096;
		fillColor.a = 4096;
		wg_fill( hGfxDevice, fillColor );


		wg_coordSPX symbolDest;
		symbolDest.x = (240-150)/2*64;
		symbolDest.y = 30*64;
		wg_setBlitSource( hGfxDevice, hSymbolSurface );
		wg_blit( hGfxDevice, symbolDest );


		wg_coordSPX logoDest;
		logoDest.x = 0*64;
		logoDest.y = (240-40)*64;
		wg_setBlitSource( hGfxDevice, hLogoSurface );
		wg_blit( hGfxDevice, logoDest );


		wg_endCanvasUpdate( hGfxDevice );

		wg_endRender( hGfxDevice );


//		pRoot->render();
		SDL_UnlockSurface(pWinSurf);

		// Make SDL update any screen regions redrawn by WonderGUI.

		wg_rect dirty;
		dirty.x = 0;
		dirty.y = 0;
		dirty.w = 240;
		dirty.h = 240;

		updateWindowRects( 1, &dirty, pWin );

		SDL_Delay(20);
	}

	// Cleanup. We should null all our smartpointers so that all objects are
	// deleted before calling Base::exit().


	wg_release( hLogoSurface );
	wg_release( hSymbolSurface );

	wg_release( hCanvas );
	wg_release( hGfxDevice );

	// Exit WonderGUI

	wg_exitGfxBase();
	exit_cpp_necessities();
	wg_exitMemHeap(heap);

	// Exit SDL

	SDL_FreeSurface(pLogoSDLSurf);
	SDL_FreeSurface(pSymbolSDLSurf);

	SDL_Quit();

	return 0;
}

//____ translateEvents() ___________________________________________________________

void translateEvents()
{
	// Process all the SDL events in a loop.
	// In this example we only use mouse input, but typically you
	// would also need to translate keyboard events.

	SDL_Event e;
	while(SDL_PollEvent(&e)) {

		switch( e.type )
		{
			case SDL_QUIT:
				bQuit = 1;
				break;

			case SDL_MOUSEMOTION:
//				Base::inputHandler()->setPointer( pRoot, Coord(MU::fromPX(e.motion.x), MU::fromPX(e.motion.y)));
				break;

			case SDL_MOUSEBUTTONDOWN:
//				Base::inputHandler()->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_MOUSEBUTTONUP:
//				Base::inputHandler()->setButton( translateMouseButton(e.button.button), false );
				break;

			default:
				break;
		}
	}

//	Base::msgRouter()->dispatch();
}


//____ updateWindowRects() _____________________________________________________
//
// Updates the rectangles of the SDL Window that WonderGUI has modified.
//
void updateWindowRects(int nRects, wg_rect * pUpdatedRects, SDL_Window * pWindow )
{
	if( nRects == 0 )
		return;

	SDL_UpdateWindowSurfaceRects( pWindow, (SDL_Rect*) pUpdatedRects, nRects );

}


