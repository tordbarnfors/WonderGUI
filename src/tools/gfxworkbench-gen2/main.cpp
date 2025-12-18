
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>



#ifdef WIN32
#    include <SDL.h>
#    include <SDL_image.h>
#elif __APPLE__
#include <unistd.h>
#    include <SDL2/SDL.h>
#    include <SDL2_image/SDL_image.h>
#else
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_image.h>
#endif

#include <wondergui.h>
#include <wondergfx.h>
#include <wondergfxstream.h>

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softkernels_default.h>
#include <wg_softkernels_rgb555be_base.h>
#include <wg_softkernels_rgb555be_extras.h>

#include <wg_gfxdevice_gen2.h>
#include <wg_gfxbackend.h>
#include <wg_backendlogger.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>

#include <wg_gradyent.h>


using namespace wg;

void 			translateEvents();
MouseButton 	translateMouseButton( uint8_t button );

bool			bQuit = false;	// Set to false by myButtonClickCallback() or translateEvents().




//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
#ifdef __APPLE__
	sleep(1);
#endif


	//------------------------------------------------------
	// Init SDL
	//------------------------------------------------------

	SDL_Init(SDL_INIT_VIDEO);

	int posX = 100, posY = 100, width = 640, height = 480;
	SDL_Window * pWin = SDL_CreateWindow("Hello WonderGUI", posX, posY, width, height, 0);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	//------------------------------------------------------
	// Init WonderGUI
	//------------------------------------------------------


	Base::init(nullptr);

	PixelFormat format = PixelFormat::Undefined;

	if( pWinSurf->format->BitsPerPixel == 32 )
		format = PixelFormat::BGRA_8_sRGB;
	else if( pWinSurf->format->BitsPerPixel == 24 )
		format = PixelFormat::BGR_8_sRGB;

	Blob_p pCanvasBlob = Blob::create( pWinSurf->pixels, 0);
	SoftSurface_p pCanvas = SoftSurface::create({ .canvas = true, .format = format, .size = SizeI(pWinSurf->w,pWinSurf->h) }, pCanvasBlob, pWinSurf->pitch);

	auto pSoftBackend = SoftBackend::create();
	addDefaultSoftKernels(pSoftBackend);
	addBaseSoftKernelsForRGB555BECanvas(pSoftBackend);
	addExtraSoftKernelsForRGB555BECanvas(pSoftBackend);



	auto pBackendLogger = BackendLogger::create(&std::cout, pSoftBackend);

	GfxDevice_p pGfxDevice = GfxDeviceGen2::create(pBackendLogger);


	// First we load the 24-bit bmp containing the button graphics.
	// No error handling or such to keep this example short and simple.

	SDL_Surface * pSDLSurf = SDL_LoadBMP( "resources/simple_button.bmp" );
	SoftSurface_p pButtonSurface = SoftSurface::create({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, PixelFormat::BGR_8, pSDLSurf->pitch, 0);
	SDL_FreeSurface(pSDLSurf);


	// Setup a bitmap font

	std::ifstream input("resources/droid_16_ascii_indexed.surf", std::ios::binary );
	auto pReader = SurfaceReader::create( WGBP(SurfaceReader, _.factory = SoftSurfaceFactory::create() ));
	Surface_p pFontSurface = pReader->readSurfaceFromStream(input);
	input.close();


	std::ifstream file("resources/droid_16_ascii_indexed.fnt");
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fontSpec = buffer.str();


/*
	SDL_Surface * pSDLFontSurf = IMG_Load( "resources/test.png" );
	SoftSurface_p pFontSurface = SoftSurface::create( SizeI( pSDLFontSurf->w, pSDLFontSurf->h ), PixelFormat::BGRA_8, (unsigned char*) pSDLFontSurf->pixels, pSDLFontSurf->pitch, 0 );
	SDL_FreeSurface(pSDLFontSurf);

	std::ifstream file( "resources/test.fnt" );
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string fontSpec = buffer.str();
*/

	
	auto pFont = BitmapFont::create(pFontSurface, fontSpec.c_str() );
	
	//
	
	auto pPrinter = Printer::create();
	pPrinter->setFont(pFont);
	pPrinter->setGfxDevice(pGfxDevice);
	pPrinter->setCursorOrigo({0,20*64});
	
	pSDLSurf = SDL_LoadBMP( "resources/What-Goes-Up-3.bmp" );
//	pSDLSurf = SDL_LoadBMP( "resources/simple_button.bmp" );
	SoftSurface_p pSRGBSurface = SoftSurface::create({ .format = PixelFormat::BGRA_8_sRGB, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, PixelFormat::BGR_8, pSDLSurf->pitch, 0);
	SDL_FreeSurface(pSDLSurf);


	//
/*
	{
		
	wg_obj buffer = wg_createStreamBuffer( 600000 );
	
	wg_component input = wg_getStreamBufferInput(buffer);
	wg_component output = wg_getStreamBufferOutput(buffer);

	
	std::ifstream file( "softube_hw_stream_channel_FA0900000005.dat", std::ios::binary );

	// copies all data into buffer
	std::vector<unsigned char> filebuffer(std::istreambuf_iterator<char>(file), {});


	
	wg_processStreamChunks(input, filebuffer.data(), filebuffer.data() + 5000);
	
	
	wg_obj buffer2 = wg_createStreamBuffer( 800000 );
	wg_component input2 = wg_getStreamBufferInput(buffer2);

	
	
	wg_obj pump = wg_createStreamPumpWithInputOutput(output, input2);
	
	wg_pumpAll(pump);
	
	}
 */

	auto pFocusSkin = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Focused, Color::Green} } });
	auto pHoverSkin = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Hovered, Color::Green} } });
	auto pHoverAndFocusSkin = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Hovered, Color::Yellow}, {State::Focused, Color::Blue} } });
	auto pSelectedHoverAndFocusSkin = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Selekted, Color::Pink}, {State::Hovered, Color::Yellow}, {State::Focused, Color::Blue} } });

	auto pHoverWithFocusSkin = ColorSkin::create({ .states = {  {State::Selekted, Color::Purple}, {State::Default, Color::Red}, {State::HoveredFocused, Color::Red}, {State::Hovered, Color::Yellow}, {State::Focused, Color::Blue} } });


	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	auto pGradient = Gradyent::create(Color::Black, Color::White, Color::White, Color::Red );

	auto p16bitCanvas = SoftSurface::create({ .canvas = true, .format = PixelFormat::RGB_555_bigendian, .size = {240,240} });

	while( !bQuit )
	{
		// Loop through SDL events, translate them to WonderGUI events
		// and process them.
		
		translateEvents();
		
		// Let WonderGUI render any updated/dirty regions of the screen.
		
		SDL_LockSurface(pWinSurf);
		
		pGfxDevice->beginRender();
		pGfxDevice->beginCanvasUpdate(pCanvas);

		pGfxDevice->fill(Color8::White);

		//================= BEGIN TEST CODE ==================================

		RectSPX updateRects[2] = {	{ 0,0,240*64,120*64},
									{ 0,120*64,240*64,120*64}
		};

		RectSPX clippedRects[2] = {	{ 32*64,123*64,100*64,100*64},
									{ 160*64,150*64,70*64,60*64}
		};


		pGfxDevice->beginCanvasUpdate(p16bitCanvas, 2, updateRects);
		pGfxDevice->setBlendMode(BlendMode::Replace);
		pGfxDevice->fill(HiColor::Transparent);
		pGfxDevice->setBlendMode(BlendMode::Blend);
		pGfxDevice->setTintGradient({0,0,240*64,240*64}, Gradient(Color::Black, Color::White, Color::White, Color::Black));
		pGfxDevice->drawElipse({0,0,240*64,240*64}, 100*64, HiColor(0,0,2048));

		pGfxDevice->setClipList(2, clippedRects);
		pGfxDevice->drawElipse({0,0,240*64,240*64}, 100*64, HiColor(0,0,2048));

		pGfxDevice->endCanvasUpdate();

		pGfxDevice->setBlitSource(p16bitCanvas);
		pGfxDevice->blit({0,0});


		//================== END TEST CODE ====================================

		pGfxDevice->endCanvasUpdate();
		pGfxDevice->endRender();


		SDL_UnlockSurface(pWinSurf);

		SDL_UpdateWindowSurface( pWin );

		SDL_Delay(20);
	}

	// Cleanup. We should null all our smartpointers so that all objects are
	// deleted before calling Base::exit().

	pCanvas = nullptr;
	pCanvasBlob = nullptr;

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

