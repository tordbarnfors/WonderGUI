
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
#include <wondergfx_c.h>
#include <wondergfxstream_c.h>

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softgfxdevice.h>
#include <wg_softkernels_default.h>
#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb555be_base.h>
#include <wg_softkernels_rgb555be_extras.h>

#include <wg_gfxdevice_gen2.h>
#include <wg_gfxbackend.h>
#include <wg_backendlogger.h>
#include <wg_softbackend.h>
#include <wg_softbackend_kernels.h>

#include <wg_gradyent.h>


using namespace wg;

void 			translateEvents();
MouseButton 	translateMouseButton( uint8_t button );

bool			bQuit = false;	// Set to false by myButtonClickCallback() or translateEvents().




//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{

	for (int i = 0; i < GfxFlip_size; i++)
	{
		GfxFlip flip = GfxFlip(i);

		SizeI orgCanvas = { 768, 512 };
		
		CoordI org = { 320,200 };
		CoordI flipped = Util::flipCoord(org, flip, orgCanvas);

		SizeI flippedCanvas = Util::flipSize(orgCanvas, flip);
		CoordI unflipped = Util::unflipCoord(flipped, flip, flippedCanvas);

		SizeI unflippedCanvas = Util::unflipSize(flippedCanvas, flip);

		assert(unflipped == org);
		assert(unflippedCanvas == orgCanvas);

	}


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


	auto pBackendLogger = BackendLogger::create(std::cout, pSoftBackend);

	GfxDevice_p pGfxDevice = GfxDeviceGen2::create(pBackendLogger);



	
//	addDefaultSoftKernels(pGfxDevice);
//	addBaseSoftKernelsForRGB565BECanvas(pGfxDevice);
//	addBaseSoftKernelsForRGB555BECanvas(pGfxDevice);

	
	// First we load the 24-bit bmp containing the button graphics.
	// No error handling or such to keep this example short and simple.

	SDL_Surface * pSDLSurf = SDL_LoadBMP( "resources/simple_button.bmp" );
	SoftSurface_p pButtonSurface = SoftSurface::create({ .format = PixelFormat::BGR_8, .size = SizeI(pSDLSurf->w, pSDLSurf->h) }, (unsigned char*)pSDLSurf->pixels, PixelFormat::BGR_8, pSDLSurf->pitch, 0);
	SDL_FreeSurface(pSDLSurf);


	// Setup a bitmap font
/*
	SDL_Surface * pSDLFontSurf = IMG_Load( "resources/droid-20-ascii.png" );
	SoftSurface_p pFontSurface = SoftSurface::create( SizeI( pSDLFontSurf->w, pSDLFontSurf->h ), PixelFormat::BGRA_8, (unsigned char*) pSDLFontSurf->pixels, pSDLFontSurf->pitch, 0 );
	SDL_FreeSurface(pSDLFontSurf);
*/

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
	auto pSelectedHoverAndFocusSkin = ColorSkin::create({ .states = { {State::Default, Color::Red}, {State::Selected, Color::Pink}, {State::Hovered, Color::Yellow}, {State::Focused, Color::Blue} } });

	auto pHoverWithFocusSkin = ColorSkin::create({ .states = {  {State::Selected, Color::Purple}, {State::Default, Color::Red}, {State::HoveredFocused, Color::Red}, {State::Hovered, Color::Yellow}, {State::Focused, Color::Blue} } });

/*
	float redMtx[9] = {	0.14f,0.1f,0.14f,
						0.1f, 0.0f, 0.1f,
						0.14f, 0.1f, 0.14f};
	
	float blueMtx[9] = {0.15f, 0.15f, 0.15f, 0, 0.4f, 0, 0,0,0};
	float greenMtx[9] = {0,0,0, 0,0.7f,0, 0,0,0};

	auto pCanvas1 = SoftSurface::create({ .canvas = true, .format = PixelFormat::BGRX_8_sRGB, .size = {320,200} } );
	auto pCanvas2 = SoftSurface::create({ .canvas = true, .format = PixelFormat::BGRX_8_sRGB, .size = {320,200} } );

	pCanvas1->fill( Color::Black );
	pCanvas2->fill( Color::Black );

	
	int dirX = 64, dirY = 64;

	CoordSPX ball = {0,0};
*/

	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	auto pGradient = Gradyent::create(Color::Black, Color::White, Color::White, Color::Red );
	
	
	while( !bQuit )
	{
		// Loop through SDL events, translate them to WonderGUI events
		// and process them.
		
		translateEvents();
		
		// Let WonderGUI render any updated/dirty regions of the screen.
		
		SDL_LockSurface(pWinSurf);
		
		pGfxDevice->beginRender();
		pGfxDevice->beginCanvasUpdate(pCanvas);
		
		pGfxDevice->fill(Color8::Black);
		
		//

		pGfxDevice->setTint( RectSPX(0,0,512,512)*64, pGradient);
//		pGfxDevice->fill( RectSPX(50,50,100,100)*64, Color::White );
		
/*
		 pFocusSkin->_render(pGfxDevice, RectSPX(5, 5, 20, 20) * 64, 64, State::HoveredFocused);
		 pHoverSkin->_render(pGfxDevice, RectSPX(5, 30, 20, 20) * 64, 64, State::HoveredFocused);
		 pHoverAndFocusSkin->_render(pGfxDevice, RectSPX(5, 55, 20, 20) * 64, 64, State::SelectedHoveredFocused);
		 pSelectedHoverAndFocusSkin->_render(pGfxDevice, RectSPX(5, 80, 20, 20) * 64, 64, State::SelectedHoveredFocused);
		 pHoverWithFocusSkin->_render(pGfxDevice, RectSPX(5, 105, 20, 20) * 64, 64, State::SelectedHoveredFocused);
*/
		
		 pGfxDevice->setBlitSource(pSRGBSurface);
//		 pGfxDevice->blit({ 0,0 });

		 pGfxDevice->rotScaleBlit(RectSPX(128, 128, 256, 256) * 64, 45, 0.5f);


		//
		
/*
		ball.x += dirX;
		ball.y += dirY;
		
		if( ball.x > 320*64 || ball.x < 0 )
			dirX = -dirX;

		if( ball.y > 200*64 || ball.y < 0 )
			dirY = -dirY;

		
		
		pGfxDevice->setBlurMatrices(64*4, redMtx, greenMtx, blueMtx);
		
		pGfxDevice->beginCanvasUpdate(pCanvas1);
		pGfxDevice->fill( RectSPX(ball.x,ball.y,20*64,20*64), Color::White );

//		pGfxDevice->fill( RectSPX(0,1,20,20)*64, Color::White );

		pGfxDevice->endCanvasUpdate();
		
		pGfxDevice->beginCanvasUpdate(pCanvas2);
		pGfxDevice->setBlitSource(pCanvas1);
		
		pGfxDevice->rotScaleBlur({0,0,320*64,200*64}, 7.f, 1.06f);
		
		//		pGfxDevice->blur( {0,0} );
		pGfxDevice->endCanvasUpdate();

		pGfxDevice->setBlitSource(pCanvas2);
		pGfxDevice->blit( {0,0} );
		
		std::swap(pCanvas1, pCanvas2);

*/
		//
/*
		for( int i = 0 ; i < GfxFlip_size ; i++ )
		{
			RectI canvas = RectSPX( 4, i * 22, 20, 16 )*64;
			
			pGfxDevice->fill( canvas, Color8::Blue );
	
			RectI flippedCanvas = { canvas.x+22*64, canvas.y, Util::flipSize({canvas.w, canvas.h}, (GfxFlip) i ) };

			pGfxDevice->fill( flippedCanvas, Color8::Blue );

			RectI unflippedCanvas = { canvas.x+44*64, canvas.y, Util::unflipSize(flippedCanvas, (GfxFlip) i ) };

			pGfxDevice->fill( unflippedCanvas, Color8::Blue );

			
			RectI org1 = RectSPX( 2,2,5,5 )*64;
			RectI org2 = RectSPX( 13,2,5,5 )*64;

			pGfxDevice->fill( org1 + canvas.pos(), Color8::Red );
			pGfxDevice->fill( org2 + canvas.pos(), Color8::Green );

			RectI flipped1 = Util::flipRect(org1, (GfxFlip)i, canvas);
			RectI flipped2 = Util::flipRect(org2, (GfxFlip)i, canvas);

			pGfxDevice->fill( flipped1 + flippedCanvas.pos(), Color8::Red );
			pGfxDevice->fill( flipped2 + flippedCanvas.pos(), Color8::Green );

			RectI unflipped1 = Util::unflipRect(flipped1, (GfxFlip)i, flippedCanvas);
			RectI unflipped2 = Util::unflipRect(flipped2, (GfxFlip)i, flippedCanvas);

			pGfxDevice->fill( unflipped1 + unflippedCanvas.pos(), Color8::Red );
			pGfxDevice->fill( unflipped2 + unflippedCanvas.pos(), Color8::Green );

			
		}
*/		
		
/*
 
 		pFont->setSize(10*64);
		
		pPrinter->setLineWidth(640*3*64);
		pPrinter->resetCursor();
		pPrinter->print("Hello from Printer!\n");
		pPrinter->printAligned(Placement::Center, "This text should be centered, one line down!");

		pPrinter->crlf();
		pPrinter->print("First part of text,");
		pPrinter->print(" second part of text on same line.");
		
		RectSPX box = { 20*64, 300*64, 640*64, 60*64 };

		pGfxDevice->fill(box, HiColor::Black);
		
		pPrinter->printInBox(box, Placement::Center, "Multiline text that is\naligned inside a box.");
		
		pPrinter->crlf();
		pPrinter->print("Yet another line.\n");
		
		pPrinter->printAt(box.pos(), "A little text");
		
		pPrinter->print("And another (one).");

		pPrinter->setLineWidth(100*64);
		pPrinter->printWrapping("This not so very long texts should be wrapping automatically every 100 pixels.\nThis is line two.", 20*64);
 */

		
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
		
/*
		auto pHWCanvas = SoftSurface::create( { .canvas = true, .format = PixelFormat::RGB_555_bigendian, .size = {256,256}} );
		pHWCanvas->fill(Color8::Green);

		pGfxDevice->beginCanvasUpdate(pHWCanvas);
		pGfxDevice->setTintGradient( {0,0,256*64,256*64}, Gradient(Placement::West, HiColor::Black, HiColor::White) );
		pGfxDevice->fill(HiColor::White);
		pGfxDevice->endCanvasUpdate();

		auto pHWCanvas2 = SoftSurface::create( { .canvas = true, .format = PixelFormat::RGB_565_bigendian, .size = {256,256}} );
		pHWCanvas2->fill(Color8::Green);

 
 		pGfxDevice->beginCanvasUpdate(pHWCanvas2);
		pGfxDevice->setTintGradient( {0,0,256*64,256*64}, Gradient(Placement::West, HiColor::Black, HiColor::White) );
		pGfxDevice->fill(HiColor::White);
		pGfxDevice->endCanvasUpdate();


		pGfxDevice->setBlitSource(pHWCanvas);
		pGfxDevice->blit({0,0});
		
		pGfxDevice->setBlitSource(pHWCanvas2);
		pGfxDevice->blit({0,256*64});

		pGfxDevice->setBlitSource( pSRGBSurface );
		pGfxDevice->blit({0,10*64});
*/
		
		pGfxDevice->endCanvasUpdate();
		pGfxDevice->endRender();

/*
		uint8_t * pScreen = (uint8_t*) pWinSurf->pixels;
		for( int y = 0 ; y < 10 ; y++ )
		{
			for( int x = 0 ; x < 256 ; x++ )
			{
				* pScreen++ = x;
				* pScreen++ = x;
				* pScreen++ = x;
				* pScreen++ = x;
			}
			pScreen += pWinSurf->pitch - 256*4;
		}
*/
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

