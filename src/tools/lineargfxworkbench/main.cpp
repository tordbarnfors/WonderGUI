
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef WIN32
#    include <SDL.h>
#    include <SDL_image.h>
#elif __APPLE__
#    include <SDL2/SDL.h>
#    include <SDL2_image/SDL_image.h>
#else
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_image.h>
#endif

#include <wondergui.h>

#include <wg_linearbackend.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softkernels_default.h>
//#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb555be_base.h>
#include <wg_softkernels_rgb555be_extras.h>

#include <wg_c_lineargfx.h>

using namespace wg;

void 			translateEvents();
MouseButton 	translateMouseButton( uint8_t button );

bool			bQuit = false;	// Set to false by myButtonClickCallback() or translateEvents().


uint8_t			canvasBuffer[640*480*4];

SDL_Window * g_pWin = nullptr;
SDL_Surface * g_pWinSurf = nullptr;

void updateScreen(CanvasRef ref, int nSegments, const LinearBackend::Segment * pSegments)
{
	
	SDL_LockSurface(g_pWinSurf);

	for( int i = 0 ; i < nSegments ; i++ )
	{
		auto& seg = pSegments[i];
		
		uint8_t * pDest = ((uint8_t*)g_pWinSurf->pixels) + seg.rect.y * g_pWinSurf->pitch + seg.rect.x * 4;
		uint8_t * pSrc = seg.pBuffer;
		
		for( int y = 0 ; y < seg.rect.h ; y++ )
		{
			memcpy( pDest, pSrc, seg.rect.w*4 );
			pDest += g_pWinSurf->pitch;
			pSrc += seg.pitch;
		}
	}
	
	SDL_UnlockSurface(g_pWinSurf);
	SDL_UpdateWindowSurface(g_pWin);
}



//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 640, height = 480;
	g_pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	g_pWinSurf = SDL_GetWindowSurface( g_pWin );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------


	Base::init(nullptr);

	PixelFormat format = PixelFormat::Undefined;



	LinearBackend_p pBackend = LinearBackend::create(
		   [](CanvasRef ref, int nBytes) { return canvasBuffer; },
		   updateScreen
    );

	addDefaultSoftKernels(pBackend);
	//	addBaseSoftKernelsForRGB565BECanvas(pBackend);
	addBaseSoftKernelsForRGB555BECanvas(pBackend);

	pBackend->defineCanvas(CanvasRef::Default, { 640 * 64,480 * 64 }, PixelFormat::BGRX_8_linear);


	auto pGfxDevice = GfxDeviceGen2::create(pBackend);
	
	//
	
	pGfxDevice->beginRender();
	pGfxDevice->beginCanvasUpdate(CanvasRef::Default);

	pGfxDevice->fill(Color8::Blue);

	pGfxDevice->endCanvasUpdate();
	pGfxDevice->endRender();

	
	
	
	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	while( !bQuit )
	{
		// Loop through SDL events, translate them to WonderGUI events
		// and process them.

		translateEvents();

		// Let WonderGUI render any updated/dirty regions of the screen.

		
		RectSPX rects[2] = { {10*64,10*64,100*64,100*64}, {120*64,120*64,100*64,100*64} };
		

		pGfxDevice->beginRender();
		pGfxDevice->beginCanvasUpdate(CanvasRef::Default,2, rects);

		pGfxDevice->fill( Color8::White);

		pGfxDevice->fill( {0,0,50*64,50*64}, Color8::Red);

		pGfxDevice->fill( {100*64,100*64,50*64,50*64}, Color8::Green);


		
/*
		for( int y = 0 ; y < 17 ; y++ )
		{
			for( int x = 0 ; x < 17 ; x++ )
			{
				pGfxDevice->fill( RectI((4+x*16)*64+y*4,(4+y*16)*64+x*4,10*64,10*64), HiColor(4096,0,0,4096) );
			}
		}

		pGfxDevice->fill( RectI((18*16)*64,0, 64*10, 64*300), HiColor(4096/16,0,0,4096) );
*/


		pGfxDevice->endCanvasUpdate();
		pGfxDevice->endRender();
		
		

		SDL_Delay(20);
	}


	// Exit WonderGUI

	Base::exit();

	// Exit SDL

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
				bQuit = true;
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

	Base::msgRouter()->dispatch();
}

//____ translateMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateMouseButton( uint8_t button )
{
	switch( button )
	{
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
		default:
			return MouseButton::None;
	}
}
