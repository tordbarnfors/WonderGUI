/*
 * This example uses SDL2 to setup a minimal framework for WonderGUI and
 * create a clickable button that closes the window. It has no
 * dependencies except SDL2 and WonderGUI.
 *
 * The intention is to show how WonderGUI easily can be integrated  with
 * any development framework with some simple bindings and pure software
 * rendering. Although SDL2 is used, no WonderGUI code outside this file
 * is dependent on SDL in anyway.
 *
 * In order to keep the example small, we are making some assumptions,
 * for example that the screen surface is either 24 or 32 bits graphics
 * with the RGB elements in the same order as SoftSurface keeps them,
 * which is usually the case on normal PC's. In environments where this
 * is not the case, either specific Surface and GfxDevice classes
 * will need to be provided or graphics needs to be rendered to an interim
 * surface from which blocks can be copied and translated to the output
 * format.
 *
 * If you want to use WonderGUI with SDL2, there are better examples to use
 * as a template. Use the SDL2 specific Surface/GfxDevice classes that
 * better integrate with SDL2's video API, giving you better performance
 * and transparent support for images and windows of other pixel formats.
 *
 * Also note that in order to keep this example short, it doesn't provide a full
 * initialization of WonderGUI. No font is initialized and no keyboard events
 * are translated. See other examples for a more full initialization of WonderGUI.
 */

#include <cstdlib>

#ifdef WIN32
#	include <SDL.h>
#else
#	include <SDL2/SDL.h>
#endif
#include <wondergui.h>

#include <wg_softsurface.h>
#include <wg_gfxdevice_gen2.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>

using namespace wg;


void 			translateEvents( RootPanel_p pRoot );
MouseButton 	translateMouseButton( uint8_t button );
void 			updateWindowRects( RootPanel_p pRoot, SDL_Window * pWindow );

bool			bQuit = false;	// Set to false by myButtonClickCallback() or translateEvents().

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 320, height = 240;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------

	// First we need to initialize the base system
	// Normally we would provide a HostBridge-interface, but 
	// for this simple example we skip it and pass null.

	Base::init(nullptr);

	// The software renderer needs a SoftSurface as its canvas,
	// so we wrap the SDL WindowSurface into a SoftSurface.

	PixelFormat format = PixelFormat::Undefined;

	if( pWinSurf->format->BitsPerPixel == 32 )
		format = PixelFormat::BGRA_8;
	else if( pWinSurf->format->BitsPerPixel == 24 )
		format = PixelFormat::BGR_8;

	Blob_p pCanvasBlob = Blob::create( pWinSurf->pixels, 0);
	SoftSurface_p pCanvas = SoftSurface::create( WGBP(Surface,
													  _.size = SizeI(pWinSurf->w,pWinSurf->h),
													  _.format = format,
													  _.canvas = true),
												 pCanvasBlob,
												 pWinSurf->pitch );


	// Wg create the GfxDevice that will be used for all rendering.

	SoftBackend_p pGfxBackend = SoftBackend::create();
	addDefaultSoftKernels(pGfxBackend);

	GfxDevice_p	pGfxDevice = GfxDeviceGen2::create(pGfxBackend);

	// We create a RootPanel. This is responsible for rendering the
	// tree of child widgets connected to it and handle their events.
	// We provide it the GfxDevice to use for rendering and the canvas
	// to render on.

	RootPanel_p pRoot = RootPanel::create( pCanvas, pGfxDevice );

	//------------------------------------------------------
	// Setup a simple GUI consisting of a filled background and
	// a button using scaled bitmaps.
	// We do this inside a C++ scope so that the temporary smartpointers
	// are automatically cleared when the scope is left and we don't
	// have to think about clearing them manually.
	//------------------------------------------------------

	{
		// First we load the 24-bit bmp containing the button graphics.
		// No error handling or such to keep this example short and simple.

		SDL_Surface * pSDLSurf = SDL_LoadBMP( "simple_button.bmp" );
		SoftSurface_p pButtonSurface = SoftSurface::create( WGBP(Surface,
																 _.size = SizeI( pSDLSurf->w, pSDLSurf->h ),
																 _.format = PixelFormat::BGR_8 ),
															(unsigned char*) pSDLSurf->pixels,
														   PixelFormat::BGR_8,
															pSDLSurf->pitch );
		SDL_FreeSurface(pSDLSurf);

		// First we create and add a FlexPanel to the RootPanel.
		// The RootPanel can only take one child, but the FlexPanel
		// provides simple and powerful ways to layout multiple children.

		FlexPanel_p pFlexPanel = FlexPanel::create();
		pRoot->slot = pFlexPanel;

		// Now we create the background by creating and setting a skin
		// for the FlexPanel.

		pFlexPanel->setSkin( ColorSkin::create(Color::Bisque) );

		// Now we create the button, using a clickable skin built from the BMP
		// with the button graphics. First we specify the Surface and a rectangle
		// for the first image. Then follows the states that we have images for,
		// the border that creates the nine stretch-zones of the image
		// (NinePatch style) and the axis in which the images are lined up.
		// When adding it to the FlexPanel we specify its geometry in
		// pixels and that it should be centered.

		Button_p pButton = Button::create();
		pButton->setSkin(BlockSkin::create( { .axis = Axis::X, .frame = Border(3,3,3,3), .states = { State::Hovered, {}, State::Pressed, {}, State::Disabled, {} }, .surface = pButtonSurface }));
		pFlexPanel->slots.pushFront(pButton, { .origo = Placement::Center, .size = {80,33} });

		// Finally we add a callback to the click-event of the button.

		Base::msgRouter()->addRoute(pButton, MsgType::Select, [&](Msg * pMsg) { bQuit = true; } );
	}


	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	while( !bQuit )
	{
		// Loop through SDL events, translate them to WonderGUI events
		// and process them.

		translateEvents( pRoot );

		// Let WonderGUI perform periodic updates

		Base::update(int64_t(SDL_GetTicks()) * 1000);

		// Let WonderGUI render any updated/dirty regions of the screen.

		SDL_LockSurface(pWinSurf);
		pRoot->render();
		SDL_UnlockSurface(pWinSurf);

		// Make SDL update any screen regions redrawn by WonderGUI.

		updateWindowRects( pRoot, pWin );

		SDL_Delay(20);
	}

	// Cleanup. We should null all our smartpointers so that all objects are
	// deleted before calling Base::exit().

	pCanvas = nullptr;
	pCanvasBlob = nullptr;
	pRoot = nullptr;

	// Exit WonderGUI

	Base::exit();

	// Exit SDL

	SDL_Quit();

	return 0;
}

//____ translateEvents() ___________________________________________________________

void translateEvents( RootPanel_p pRoot )
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
			{
				float scale = pRoot->scale()/64.f;
				Base::inputHandler()->setPointer( pRoot, Coord(e.motion.x/scale, e.motion.y/scale));
				break;
			}

			case SDL_MOUSEBUTTONDOWN:
				Base::inputHandler()->setButton( translateMouseButton(e.button.button), true );
				break;

			case SDL_MOUSEBUTTONUP:
				Base::inputHandler()->setButton( translateMouseButton(e.button.button), false );
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

//____ updateWindowRects() _____________________________________________________
//
// Updates the rectangles of the SDL Window that WonderGUI has modified.
//
void updateWindowRects( RootPanel_p pRoot, SDL_Window * pWindow )
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

