

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <wax_c_types.h>
#include <wax_c_core.h>
#include <wax_c_blob.h>
#include <wax_c_geo.h>
#include <wax_c_object.h>
#include <wax_c_softgfx.h>
#include <wax_c_gfxdevice.h>
#include <wax_c_softkernels_default.h>

#include "cpp_necessities.h"

void 			translateEvents();
void 			updateWindowRects( int nRects, wax_rect * pRects, SDL_Window * pWindow );

int				bQuit = 0;	// Set to false by myButtonClickCallback() or translateEvents().



//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int width = 240, height = 240;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", width, height, 0);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	wax_initCore();



	wax_pixelFormat format = WAX_PIXFMT_UNDEFINED;

	if( SDL_BYTESPERPIXEL(pWinSurf->format) == 4 )
		format = WAX_PIXFMT_ARGB_8;					// 24-bit window surfaces are not supported.

	wax_sizeI canvasSize;
	canvasSize.w = pWinSurf->w;
	canvasSize.h = pWinSurf->h;

	SDL_LockSurface(pWinSurf);

    wax_obj hBlob = wax_createBlobFromData( pWinSurf->pixels, 0, NULL );
	wax_obj hCanvas = wax_createSoftSurfaceFromBlob( &(wax_surfaceBP){ .canvas = 1, .size = canvasSize, .format = format }, hBlob, pWinSurf->pitch );
	wax_release( hBlob );

	SDL_UnlockSurface(pWinSurf);

	wax_obj hBackend = wax_createSoftBackend();
	wax_addDefaultSoftKernels(hBackend);

	wax_obj hGfxDevice = wax_createGfxDevice(hBackend);

	//------------------------------------------------------
	

	wax_sizeI imgSize;
	
	// First we load the 24-bit bmp containing the button graphics.
	// No error handling or such to keep this example short and simple.

	SDL_Surface * pLogoSDLSurf = SDL_LoadBMP( "wondergui.bmp" );

	imgSize.w = pLogoSDLSurf->w;
	imgSize.h = pLogoSDLSurf->h;


	hBlob = wax_createBlobFromData( pLogoSDLSurf->pixels, 0, NULL );
	wax_obj hLogoSurface = wax_createSoftSurfaceFromBlob( &(wax_surfaceBP){ .size = imgSize, .format = WAX_PIXFMT_XRGB_8 }, hBlob, pLogoSDLSurf->pitch );
    wax_release(hBlob);


	
	SDL_Surface * pSymbolSDLSurf = SDL_LoadBMP( "wg-symbol.bmp" );

	imgSize.w = pSymbolSDLSurf->w;
	imgSize.h = pSymbolSDLSurf->h;

    hBlob = wax_createBlobFromData( pSymbolSDLSurf->pixels, 0, NULL );
	wax_obj hSymbolSurface = wax_createSoftSurfaceFromBlob( &(wax_surfaceBP){ .size = imgSize, .format = WAX_PIXFMT_XRGB_8 }, hBlob, pSymbolSDLSurf->pitch );
    wax_release(hBlob);

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

		wax_beginRender( hGfxDevice );

		wax_beginCanvasUpdateWithSurface( hGfxDevice, hCanvas, 0, NULL, NULL, -1 );


		wax_color fillColor;
		fillColor.r = 4096;
		fillColor.g = 4096;
		fillColor.b = 4096;
		fillColor.a = 4096;
		wax_fill( hGfxDevice, fillColor );


		wax_coordSPX symbolDest;
		symbolDest.x = (240-150)/2*64;
		symbolDest.y = 30*64;
		wax_setBlitSource( hGfxDevice, hSymbolSurface );
		wax_blit( hGfxDevice, symbolDest );


		wax_coordSPX logoDest;
		logoDest.x = 0*64;
		logoDest.y = (240-40)*64;
		wax_setBlitSource( hGfxDevice, hLogoSurface );
		wax_blit( hGfxDevice, logoDest );


		wax_endCanvasUpdate( hGfxDevice );

		wax_endRender( hGfxDevice );


//		pRoot->render();
		SDL_UnlockSurface(pWinSurf);

		// Make SDL update any screen regions redrawn by WonderGUI.

		wax_rect dirty;
		dirty.x = 0;
		dirty.y = 0;
		dirty.w = 240;
		dirty.h = 240;

		updateWindowRects( 1, &dirty, pWin );

		SDL_Delay(20);
	}

	// Cleanup. We should null all our smartpointers so that all objects are
	// deleted before calling Base::exit().


	wax_release( hLogoSurface );
	wax_release( hSymbolSurface );

	wax_release( hCanvas );
	wax_release( hGfxDevice );

	// Exit WonderGUI

	wax_exitCore();
	exit_cpp_necessities();

	// Exit SDL

	SDL_DestroySurface(pLogoSDLSurf);
	SDL_DestroySurface(pSymbolSDLSurf);

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
			case SDL_EVENT_QUIT:
				bQuit = 1;
				break;

			case SDL_EVENT_MOUSE_MOTION:
//				Base::inputHandler()->setPointer( pRoot, Coord(MU::fromPX(e.motion.x), MU::fromPX(e.motion.y)));
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
//				Base::inputHandler()->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_EVENT_MOUSE_BUTTON_UP:
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
void updateWindowRects(int nRects, wax_rect * pUpdatedRects, SDL_Window * pWindow )
{
	if( nRects == 0 )
		return;

	SDL_UpdateWindowSurfaceRects( pWindow, (SDL_Rect*) pUpdatedRects, nRects );

}


