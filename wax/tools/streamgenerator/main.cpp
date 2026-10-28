
#include <cstdlib>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#ifdef __APPLE__
#	include <unistd.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>

#include <wax.h>
#include <waxgfxstream.h>

#include <wax_softsurface.h>
#include <wax_softedgemap.h>
#include <wax_softedgemapfactory.h>
#include <wax_softsurfacefactory.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>
#include <wax_gfxdevice_gen2.h>

using namespace wax;
using namespace std;



void			processEvents();
void			renderOutput( GfxDevice * pDevice, Surface * pWindowCanvas, Surface * pStreamCanvas );
void * 			loadFile( const char * pPath );
Blob_p 			loadBlob( const char * pPath );
SoftSurface_p	loadImage( const char * pPath );

void playDelayFrames(GfxDevice_p pDevice, int nFrames);

void playRectangleDance(GfxDevice_p pDevice, CanvasRef canvas);
void playRectangleDanceDualScreen(GfxDevice_p pDevice, CanvasRef canvasRef1, CanvasRef canvasRef2 );
void playScroll(GfxDevice_p pDevice, RectI canvas );
void playLogoFadeIn(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory );
void playSurfaceStressTest(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory );
void playImageStreamingTest(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory );
void playBounceRects(GfxDevice_p pDevice, CanvasRef canvasRef);
void playDualScreenBounceRects(GfxDevice_p pDevice, CanvasRef canvasRef1, CanvasRef canvasRef2);
void playDirectUpdate(GfxDevice_p pDevice, CanvasRef canvasRef );


CoordSPX positionSprite(SizeSPX dimensions, int tick, int length, int nb, int amount);



bool	bQuit = false;

StreamEncoder_p	s_pEncoder;


void freeSDLSurfCallback( void * pSDLSurf )
{
	SDL_DestroySurface( (SDL_Surface*) pSDLSurf );
}



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

	int width = 1024, height = 600;
	SDL_Window * pWin = SDL_CreateWindow("Wax Stream Generator", width, height, SDL_WINDOW_HIGH_PIXEL_DENSITY);

	SDL_Surface * pWinSurf = SDL_GetWindowSurface( pWin );

	 
	//------------------------------------------------------
	// Init Wax
	//------------------------------------------------------

	Core::init();


	PixelFormat type = PixelFormat::Undefined;

	if( SDL_BYTESPERPIXEL(pWinSurf->format) == 4 )
		type = PixelFormat::ARGB_8;					// 24-bit window surfaces are not supported.
		
	Blob_p pCanvasBlob = Blob::create( pWinSurf->pixels, 0);	
	SoftSurface_p pCanvas = SoftSurface::create({ .canvas = true, .format = type, .size = SizeI(pWinSurf->w,pWinSurf->h) }, pCanvasBlob, pWinSurf->pitch);
    
	SoftBackend_p pGfxBackend = SoftBackend::create();
	addDefaultSoftKernels(pGfxBackend);

	GfxDevice_p pGfxDevice = GfxDeviceGen2::create(pGfxBackend);


	// Init font
/*
	PixelFormat	format;

	char * pFontSpec = (char*)loadFile("../resources/anuvverbubbla_8x8.fnt");

	SDL_Surface * pFontSurf = IMG_Load( "../resources/anuvverbubbla_8x8.png" );
//	convertSDLFormat( &format, pFontSurf->format );
	SoftSurface_p pFontImg = SoftSurface::create( SizeI(pFontSurf->w,pFontSurf->h), PixelFormat::ARGB_8, (unsigned char*) pFontSurf->pixels, pFontSurf->pitch);
	SDL_FreeSurface( pFontSurf );
		
	BitmapFont_p pBmpFont = BitmapFont::create( pFontImg, pFontSpec );


	Blob_p pFontFile = loadBlob("../resources/DroidSans.ttf");
	
	FreeTypeFont_p pFont = FreeTypeFont::create( pFontFile, 1 );


	TextStyle_p pStyle = TextStyle::create();
	pStyle->setFont(pFont);
	pStyle->setSize(16);
	Base::setDefaultStyle(pStyle);
*/



	//------------------------------------------------------
	// Setup streaming
	//------------------------------------------------------
    
	auto pStreamOutputCanvas1 = SoftSurface::create( WAXBP(Surface,
														  _.size = {800,480},
														  _.format = PixelFormat::ARGB_8,
														  _.canvas = true) );

/*
	auto pStreamOutputCanvas2 = SoftSurface::create( WAXBP(Surface,
														  _.size = {800,480},
														  _.format = PixelFormat::ARGB_8,
														  _.canvas = true) );
 */

	pGfxBackend->defineCanvas(CanvasRef::Canvas_1, pStreamOutputCanvas1);
//	pGfxDevice->defineCanvas(CanvasRef::Canvas_2, pStreamOutputCanvas2);




//	auto pStreamPlug = GfxStreamPlug::create();

//	auto pGfxPlayer = StreamPlayer::create(pGfxDevice, SoftSurfaceFactory::create());

	// Writer

	char* pBigBuffer = new char[10000000];
	char* pWrite = pBigBuffer;

	auto pStreamWriter = StreamWriter::create([&pWrite](int nBytes, const void* pData) 
		{
			memcpy(pWrite, pData, nBytes);
			pWrite += nBytes;
		});

	// Buffer

	auto pStreamBuffer = StreamBuffer::create();
	
	// Encoder with splitter, feeding writer and buffer.

//	auto pEncoder = StreamEncoder::create(pGfxPlayer->stream.ptr());
//	auto pEncoder = StreamEncoder::create(pStreamLogger->stream.ptr());

	auto pFirstSplitter = StreamSplitter::create( { StreamSink_p( pStreamBuffer, pStreamBuffer->input), StreamSink_p( pStreamWriter, pStreamWriter->input) });

	auto pEncoder = StreamEncoder::create(StreamSink_p( pFirstSplitter, pFirstSplitter->input) );

	s_pEncoder = pEncoder;

	// Logger

	auto pStreamLogger = StreamLogger::create(std::cout);

	// Player

	auto pStreamPlayer = StreamPlayer::create(pGfxBackend, SoftSurfaceFactory::create(), SoftEdgemapFactory::create() );
	 

	// Streampump taking from buffer and feeding logger and/or player

	auto pAfterBufferSplitter = StreamSplitter::create({ StreamSink_p( pStreamLogger, pStreamLogger->input ), StreamSink_p( pStreamPlayer, pStreamPlayer->input) });

	auto pStreamPump = StreamPump::create(StreamSource_p(pStreamBuffer, pStreamBuffer->output), StreamSink_p( pAfterBufferSplitter, pAfterBufferSplitter->input) );

	// StreamGfxDevice and StreamSurfaceFactory feeding encoder

	auto pLZCompressor = LZCompressor::create();
	auto pSPXCompressor = SPXCompressor::create();
	auto pQ565Compressor = Q565Compressor::create();
	auto pRLECompressor = RLECompressor::create();


	StreamBackend::Blueprint bp;

	bp.encoder = pEncoder;
	bp.commandCompressor = pLZCompressor;
//	bp.objectCompressor = pLZCompressor;
	bp.colorCompressor = pLZCompressor;
	bp.rectCompressor = pSPXCompressor;
//	bp.transformCompressor = pLZCompressor;

	auto pStreamBackend = StreamBackend::create(bp);
//    pStreamBackend->defineCanvas(CanvasRef::Canvas_1, {800,480}, PixelFormat::Undefined );

	pStreamBackend->defineCanvas(CanvasRef::Canvas_1, {240,240}, PixelFormat::Undefined );
	pStreamBackend->defineCanvas(CanvasRef::Canvas_2, {240,240}, PixelFormat::Undefined );


	auto pStreamDevice = GfxDeviceGen2::create(pStreamBackend);

	auto pSurfaceFactory = RemoteSurfaceFactory::create(pEncoder);


//	pStreamPlug->openOutput(0);
//	auto pStreamLogger = StreamLogger::create(pStreamPlug->output[0], std::cout);

//	pStreamPlug->openOutput(1);
//	auto pGfxPlayer = StreamPlayer::create(pStreamPlug->output[1], pGfxDevice, SoftSurfaceFactory::create());
    
    //------------------------------------------------------
    // Setup stream saving
    //------------------------------------------------------

	pStreamBackend->encodeCanvasList();

/*
    char * pBigBuffer = new char[10000000];

    char * pWrite = pBigBuffer;
    char * pRead = pBigBuffer;

    pStreamPlug->openOutput(2);
    
    auto pSaveInStream = &pStreamPlug->output[2];
*/
   
	//------------------------------------------------------
	// Record stream
	//------------------------------------------------------

//   playDirectUpdate( pStreamDevice, CanvasRef::Canvas_1 );
//   playRectangleDance( pStreamDevice, CanvasRef::Canvas_1 );
//	  playRectangleDanceDualScreen( pStreamDevice, CanvasRef::Canvas_1, CanvasRef::Canvas_2 );
    playLogoFadeIn( pStreamDevice, CanvasRef::Canvas_1, pSurfaceFactory );
//    playSurfaceStressTest( pStreamDevice, CanvasRef::Canvas_1, pSurfaceFactory );
//	playBounceRects( pStreamDevice, CanvasRef::Canvas_1 );
//	playDualScreenBounceRects( pStreamDevice, CanvasRef::Canvas_1, CanvasRef::Canvas_2 );

//	playImageStreamingTest( pStreamDevice, CanvasRef::Canvas_1, pSurfaceFactory );

	//------------------------------------------------------
	// Save stream to file
	//------------------------------------------------------

	ofstream save("output.wax", ios::out | ios::binary | ios::trunc);

    save.write( (char*) pBigBuffer, pWrite - pBigBuffer);
    
	save.close();


	//------------------------------------------------------
	// Setup graphics
	//------------------------------------------------------

//	auto pSoftSurfaceFactory = SoftSurfaceFactory::create();


	SoftSurface_p pSplashImage = loadImage("resources/splash.png");
	Surface_p pSplashSurface = pSurfaceFactory->createSurface({ .format = PixelFormat::ARGB_8, .size = pSplashImage->pixelSize() });
	pSplashSurface->copy( CoordI(), pSplashImage );



/*
	pStreamDevice->beginRender();
    pStreamDevice->beginCanvasUpdate(CanvasRef::Canvas_1);
    
	pStreamDevice->fill( RectI( 0,0,width,height ), Color::Blue);
	pStreamDevice->fill( RectI( 10,10,100,100 ), Color::Red);
    pStreamDevice->setBlitSource(pSplashSurface);
    
	pStreamDevice->blit( { 120,10 } );
    pStreamDevice->endCanvasUpdate();
    pStreamDevice->endRender();

    pStreamDevice->beginRender();
    pStreamDevice->beginCanvasUpdate(CanvasRef::Canvas_1);
    
    pStreamDevice->fill( RectI( 0,0,width,height ), Color::Green);
    pStreamDevice->fill( RectI( 10,10,100,100 ), Color::Yellow);
    
    pStreamDevice->endCanvasUpdate();
    pStreamDevice->endRender();
*/
	//------------------------------------------------------
	// Program Main Loop
	//------------------------------------------------------

	pStreamPlayer->setStoreDirtyRects(true);
	
	while( !bQuit )
	{
		processEvents();
        
        
       pStreamPump->pumpFrame();
//		pStreamPump->pumpAllFramesWithOptimization();


		
		SDL_LockSurface(pWinSurf);
		renderOutput( pGfxDevice, pCanvas, pStreamOutputCanvas1 );
		SDL_UnlockSurface(pWinSurf);

		int				nRects1,nRects2;
		const RectI	* pRects1, *pRects2;
		
//		std::tie(nRects1,pRects1) = pStreamPlayer->dirtyRects(wax::CanvasRef::Canvas_1);
//		std::tie(nRects2,pRects2) = pStreamPlayer->dirtyRects(wax::CanvasRef::Canvas_2);

		
		
		
		SDL_Rect	r;
		r.x = 0;
		r.y = 0;
		r.w = width;
		r.h = height;
		SDL_UpdateWindowSurfaceRects(pWin, &r, 1);

		pStreamPlayer->clearDirtyRects();



//		pStreamLogger->logAll();

		SDL_Delay(10);
    }

	// Exit Wax

	Core::exit();

	// Exit SDL

	SDL_Quit();

    return 0;
}


//____ processEvents() _________________________________________________________

void processEvents()
{
	SDL_Event e;
	while( SDL_PollEvent(&e) )
	{
		if( e.type == SDL_EVENT_QUIT || (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) )
			bQuit = true;
	}
}

//____ renderOutput() __________________________________________________________
//
// Draws the stream output on the window: a background with two framed output
// areas, the first one showing the stream canvas.

void renderOutput( GfxDevice * pDevice, Surface * pWindowCanvas, Surface * pStreamCanvas )
{
	const RectI	frames[2] = { {10,10,804,484}, {244 + 20,10,804,484} };
	const int	border = 2;

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(pWindowCanvas);

	pDevice->fill( Color::Burlywood );

	for( int i = 0 ; i < 2 ; i++ )
	{
		RectI frame = frames[i];
		RectI content = { frame.x + border, frame.y + border, frame.w - border*2, frame.h - border*2 };

		pDevice->fill( frame*64, Color::Black );
		pDevice->fill( content*64, Color::HotPink );

		if( i == 0 )
		{
			pDevice->setBlitSource( pStreamCanvas );
			pDevice->blit( content.pos()*64 );
		}
	}

	pDevice->endCanvasUpdate();
	pDevice->endRender();
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

//____ loadImage() _____________________________________________________________
//
// Loads an image in any format stb_image supports as an ARGB_8 SoftSurface.

SoftSurface_p loadImage( const char * pPath )
{
	int width, height, channels;
	stbi_uc * pPixels = stbi_load( pPath, &width, &height, &channels, 4 );
	if( !pPixels )
	{
		printf( "ERROR: Could not load image '%s'\n", pPath );
		exit(1);
	}

	// stb_image gives R, G, B, A in that order, which little endian masks describe.

	PixelDescription desc;
	desc.type = PixelType::Chunky;
	desc.bits = 32;
	desc.bigEndian = false;
	desc.R_mask = 0x000000FF;
	desc.G_mask = 0x0000FF00;
	desc.B_mask = 0x00FF0000;
	desc.A_mask = 0xFF000000;

	auto pSurface = SoftSurface::create( { .format = PixelFormat::ARGB_8, .size = {width,height} }, pPixels, desc, width*4 );
	stbi_image_free( pPixels );
	return pSurface;
}

//____ playImageStreamingTest() _______________________________________________

void playImageStreamingTest(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory )
{
	SoftSurface_p pOrgSurf = loadImage( "Flow Studio Boot Image.png" );
	SizeI logoSize = pOrgSurf->pixelSize();

	Surface_p pLogoSurf = pFactory->createSurface({ .format = PixelFormat::ARGB_8, .size = logoSize });

	pLogoSurf->copy( CoordI(), pOrgSurf );
	
	SizeI canvasSize = pDevice->canvas(canvasRef).size;

	
	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvasRef);
	
	pDevice->setBlitSource(pLogoSurf);
	pDevice->blit( {0,0} );
	
	pDevice->endCanvasUpdate();
	
	pDevice->endRender();
}


//____ playSurfaceStressTest() _________________________________________________

void playSurfaceStressTest(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory )
{
	SoftSurface_p pOrgSurf = loadImage( "resources/logo-200.png" );
	SizeI logoSize = pOrgSurf->pixelSize();

	Surface_p pLogoSurf = pFactory->createSurface({ .format = PixelFormat::Alpha_8, .size = logoSize });

    pLogoSurf->copy( CoordI(), pOrgSurf );
    
    SizeI canvasSize = pDevice->canvas(canvasRef).size/64;

    int ticker = 0;
    int length = 600;
    
    Surface_p pOldCanvas;
    
    SizeI backCanvasSize( 800,480 );

    while (ticker < length)
    {
		auto pCanvas = pFactory->createSurface( WAXBP(Surface, _.size = backCanvasSize, _.format = PixelFormat::BGR_565, _.colorSpace = ColorSpace::Linear, _.bigEndian = true, _.canvas = true ) );
        
        
        
        pDevice->beginRender();
        pDevice->beginCanvasUpdate(pCanvas);
        
        pDevice->fill(Color::Black);

        if( pOldCanvas )
        {
            pDevice->setBlitSource(pOldCanvas);
            pDevice->setTintColor(HiColor(4096,3800,3500,3930));
//            pDevice->blit( CoordI( (canvasSize.w - logoSize.w)/2*64, (canvasSize.h - logoSize.h)/2*64 ));
            pDevice->rotScaleBlit( backCanvasSize*64, 3, 1.02f );
        }

        
        pDevice->setBlitSource(pLogoSurf);
        pDevice->setTintColor(HiColor(4096,4096,4096,4096));
//        pDevice->rotScaleBlit( canvasSize, -ticker, 1.f );
//        pDevice->blit( CoordI( (canvasSize.w - logoSize.w)/2, (canvasSize.h - logoSize.h)/2 ));

        float val = sin(ticker*0.1f);
        
        SizeI wantedSize( logoSize.w + (logoSize.w/2 * val), logoSize.h );
        
        pDevice->stretchBlit( RectI( (canvasSize.w - wantedSize.w)/2, (canvasSize.h - wantedSize.h)/2, wantedSize )*64 );
        
        pDevice->endCanvasUpdate();
        pOldCanvas = pCanvas;

        pDevice->beginCanvasUpdate(canvasRef);
        
        
        pDevice->setBlitSource(pCanvas);
        pDevice->blit( {0,0}, RectI((backCanvasSize.w-canvasSize.w)/2,(backCanvasSize.h-canvasSize.h)/2, canvasSize)*64 );
        
        pDevice->endCanvasUpdate();
        
        pDevice->endRender();
        ticker++;
    }
}

//____ playLogoFadeIn() _________________________________________________

void playLogoFadeIn(GfxDevice_p pDevice, CanvasRef canvasRef, SurfaceFactory_p pFactory )
{
	SoftSurface_p pOrgSurf = loadImage( "resources/logo-200.png" );
	SizeI logoSize = pOrgSurf->pixelSize();

	Surface_p pLogoSurf = pFactory->createSurface({ .format = PixelFormat::Alpha_8, .size = logoSize });

    pLogoSurf->copy( CoordI(), pOrgSurf );
    
    SizeI canvasSize = pDevice->canvas(canvasRef).size/64;

    int ticker = 0;
    int length = 30;
    while (ticker < length)
    {
        pDevice->beginRender();
        pDevice->beginCanvasUpdate(canvasRef);
        
        pDevice->fill(Color::Black);
        
        pDevice->setBlitSource(pLogoSurf);
        pDevice->setTintColor(HiColor(4096,4096,4096,ticker*4096/(length-1)));
        pDevice->blit( CoordSPX( (canvasSize.w - logoSize.w)/2*64, (canvasSize.h - logoSize.h)/2*64 ));
        
        pDevice->endCanvasUpdate();
        pDevice->endRender();
        ticker++;
    }
}

//____ playRectangleDance() _________________________________________________

void playRectangleDance(GfxDevice_p pDevice, CanvasRef canvasRef )
{
	int ticker = 0;
	const int length = 150;
	SizeSPX spriteSize(30*64, 30*64);
    SizeSPX canvasSize = pDevice->canvas(canvasRef).size;
	SizeSPX moveDim(canvasSize.w - spriteSize.w, canvasSize.h - spriteSize.h);

	while (ticker < length)
	{
		pDevice->beginRender();
        pDevice->beginCanvasUpdate(canvasRef);
        
		pDevice->fill(Color::Black);
		pDevice->fill({ positionSprite(moveDim, ticker, length, 0, 3),spriteSize }, Color::Red);
		pDevice->fill({ positionSprite(moveDim, ticker, length, 1, 3),spriteSize }, Color::Green);
		pDevice->fill({ positionSprite(moveDim, ticker, length, 2, 3),spriteSize }, Color::Blue);

        pDevice->endCanvasUpdate();
		pDevice->endRender();
		ticker++;
	}
}

//____ playDirectUpdate() _________________________________________________

void playDirectUpdate(GfxDevice_p pDevice, CanvasRef canvasRef )
{
	SoftBackend_p pBackend = SoftBackend::create();
	addDefaultSoftKernels( pBackend );

	GfxDevice_p	pMyDevice = GfxDeviceGen2::create(pBackend);

	auto& canvasInfo = pDevice->canvas(canvasRef);

	SoftSurface_p	pMyCanvas = SoftSurface::create( { 	.canvas = true,
														.format = canvasInfo.format,
														.size = canvasInfo.size / 64 });

	MirrorSurface_p	pMyMirror = MirrorSurface::create({
		.compressor = LZCompressor::create(),
		.encoder = s_pEncoder,
		.surface = pMyCanvas
	});


	int ticker = 0;
	const int length = 15;
	SizeSPX spriteSize(30*64, 30*64);
	SizeSPX canvasSize = pDevice->canvas(canvasRef).size;
	SizeSPX moveDim(canvasSize.w - spriteSize.w, canvasSize.h - spriteSize.h);

	while (ticker < length)
	{

		pMyDevice->beginRender();
		pMyDevice->beginCanvasUpdate(pMyCanvas);

		pMyDevice->fill(Color::Black);
		pMyDevice->fill({ positionSprite(moveDim, ticker*100, length*100, 0, 3),spriteSize }, Color::Red);
		pMyDevice->fill({ positionSprite(moveDim, ticker*100, length*100, 1, 3),spriteSize }, Color::Green);
		pMyDevice->fill({ positionSprite(moveDim, ticker*100, length*100, 2, 3),spriteSize }, Color::Blue);

		pMyDevice->endCanvasUpdate();
		pMyDevice->endRender();

		pDevice->beginRender();

		pDevice->beginCanvasUpdate(canvasRef);
		pDevice->setBlitSource(pMyMirror);
		pDevice->blit({ 0,0 });
		pDevice->endCanvasUpdate();

		pDevice->endRender();
		
		ticker++;


	}
}



//____ playBounceRects() _________________________________________________

void playBounceRects(GfxDevice_p pDevice, CanvasRef canvasRef)
{
	int ticker = 0;
	const int length = 1000;
	SizeSPX spriteSize(30 * 64, 30 * 64);
	SizeSPX canvasSize = pDevice->canvas(canvasRef).size;
	SizeSPX moveDim(canvasSize.w - spriteSize.w, canvasSize.h - spriteSize.h);

	RectSPX spriteRects[8] = {	{100 * 64,100 * 64,50*3 * 64,50*3 * 64},
								{100 * 64,100 * 64,40*2 * 64,10*2 * 64},
								{100 * 64,100 * 64,30*2 * 64,30*2 * 64},
								{100 * 64,100 * 64,30*2 * 64,20*2 * 64},
								{100 * 64,100 * 64,20*2 * 64,15*2 * 64},
								{100 * 64,100 * 64,20*2 * 64,20*2 * 64},
								{100 * 64,100 * 64,15*2 * 64,15*2 * 64},
								{100 * 64,100 * 64,10*2 * 64,10*2 * 64} };

	Color8	spriteColors[8] = { Color::Red, Color::Green, Color::Blue, Color::Pink, Color::Purple, Color::Brown, Color::Yellow, Color::Magenta };


	CoordSPX spriteMovement[8] = { { 64,128 },
									{ 128,64 },
									{ 32,96 },
									{ 250,103 },
									{ 64,250 },
									{ 48,120 },
									{ -40,-78 },
									{ -80,-150 } };


	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvasRef);

	pDevice->fill(Color::Black);

	pDevice->endCanvasUpdate();
	pDevice->endRender();

	while (ticker < length)
	{
		PatchesSPX clip;

		const int nSprites = 8;

		for (int i = 0; i < nSprites; i++)
			clip.add( GfxUtil::alignUp(spriteRects[i]));

		for (int i = 0; i < nSprites; i++)
		{
			spriteRects[i].x += spriteMovement[i].x;
			spriteRects[i].y += spriteMovement[i].y;

			if (spriteRects[i].x < 0)
			{
				 spriteMovement[i].x = -spriteMovement[i].x;
				 spriteRects[i].x = -spriteRects[i].x;
			}

			if (spriteRects[i].y < 0)
			{
				spriteMovement[i].y = -spriteMovement[i].y;
				spriteRects[i].y = -spriteRects[i].y;
			}

			if (spriteRects[i].x + spriteRects[i].w > canvasSize.w )
			{
				spriteMovement[i].x = -spriteMovement[i].x;
				spriteRects[i].x = canvasSize.w - spriteRects[i].w - (spriteRects[i].x + spriteRects[i].w - canvasSize.w);
			}

			if (spriteRects[i].y + spriteRects[i].h > canvasSize.h)
			{
				spriteMovement[i].y = -spriteMovement[i].y;
				spriteRects[i].y = canvasSize.h - spriteRects[i].h - (spriteRects[i].y + spriteRects[i].h - canvasSize.h);
			}

		}

		for (int i = 0; i < nSprites; i++)
		{
			RectSPX r = GfxUtil::alignUp(spriteRects[i]);

			// Align x to even pixels
/*
			spx x2 = (r.x + r.w + 64) & 0xFFFFFF80;
			r.x = r.x & 0xFFFFFF80;
			r.w = x2 - r.x;
*/
			// Align x to 4 pixel boundaries.

			spx x2 = (r.x + r.w + 64*3) & 0xFFFFFF00;
			r.x = r.x & 0xFFFFFF00;
			r.w = x2 - r.x;

			clip.add(r);
		}

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvasRef, clip.size(), clip.begin() );

		pDevice->fill(Color::Black);

		for (int i = 0 ; i < nSprites; i++)
			pDevice->fill(spriteRects[i], spriteColors[i]);

		pDevice->endCanvasUpdate();
		pDevice->endRender();

		ticker++;

	}
}


//____ playDualScreenBounceRects() _________________________________________________

void playDualScreenBounceRects(GfxDevice_p pDevice, CanvasRef canvasRef1, CanvasRef canvasRef2)
{
	int ticker = 0;
	const int length = 1000;
	SizeSPX spriteSize(30 * 64, 30 * 64);
	SizeSPX canvasSize = pDevice->canvas(canvasRef1).size;
	SizeSPX moveDim(canvasSize.w - spriteSize.w, canvasSize.h - spriteSize.h);

	RectSPX spriteRects[2][8] = {{	{10 * 64,10 * 64,50*3 * 64,50*3 * 64},
									{10 * 64,10 * 64,40*2 * 64,10*2 * 64},
									{10 * 64,10 * 64,30*2 * 64,30*2 * 64},
									{10 * 64,10 * 64,30*2 * 64,20*2 * 64},
									{10 * 64,10 * 64,20*2 * 64,15*2 * 64},
									{10 * 64,10 * 64,20*2 * 64,20*2 * 64},
									{10 * 64,10 * 64,15*2 * 64,15*2 * 64},
									{10 * 64,10 * 64,10*2 * 64,10*2 * 64} },
								{	{10 * 64,10 * 64,50*3 * 64,50*3 * 64},
									{10 * 64,10 * 64,40*2 * 64,10*2 * 64},
									{10 * 64,10 * 64,30*2 * 64,30*2 * 64},
									{10 * 64,10 * 64,30*2 * 64,20*2 * 64},
									{10 * 64,10 * 64,20*2 * 64,15*2 * 64},
									{10 * 64,10 * 64,20*2 * 64,20*2 * 64},
									{10 * 64,10 * 64,15*2 * 64,15*2 * 64},
									{10 * 64,10 * 64,10*2 * 64,10*2 * 64} }};

	
	Color8	spriteColors[8] = { Color::Red, Color::Green, Color::Blue, Color::Pink, Color::Purple, Color::Brown, Color::Yellow, Color::Magenta };


	CoordSPX spriteMovement[2][8] = { { { 64,128 },
									{ 128,64 },
									{ 32,96 },
									{ 250,103 },
									{ 64,250 },
									{ 48,120 },
									{ -40,-78 },
									{ -80,-150 } },

									{ { 164,28 },
									{ 64,128 },
									{ 96,32 },
									{ 103,250 },
									{ 264,50 },
									{ 120,48 },
									{ -78,-40 },
									{ -180,-50 } } };
	
	pDevice->beginRender();

	pDevice->beginCanvasUpdate(canvasRef1);
	pDevice->fill(Color::Black);
	pDevice->endCanvasUpdate();

	pDevice->beginCanvasUpdate(canvasRef2);
	pDevice->fill(Color::Black);
	pDevice->endCanvasUpdate();

	pDevice->endRender();

	while (ticker < length)
	{
		PatchesSPX clip;

		const int nSprites = 8;

		pDevice->beginRender();

		for( int s = 0 ; s < 2 ; s++ )
		{
//			for (int i = 0; i < nSprites; i++)
//				clip.add( GfxUtil::alignUp(spriteRects[s][i]));

			for (int i = 0; i < nSprites; i++)
			{
				RectSPX r = GfxUtil::alignUp(spriteRects[s][i]);

				// Align x to even pixels
	/*
				spx x2 = (r.x + r.w + 64) & 0xFFFFFF80;
				r.x = r.x & 0xFFFFFF80;
				r.w = x2 - r.x;
	*/
				// Align x to 4 pixel boundaries.

				spx x2 = (r.x + r.w + 64*3) & 0xFFFFFF00;
				r.x = r.x & 0xFFFFFF00;
				r.w = x2 - r.x;

				clip.add(r);
			}

			for (int i = 0; i < nSprites; i++)
			{
				spriteRects[s][i].x += spriteMovement[s][i].x;
				spriteRects[s][i].y += spriteMovement[s][i].y;

				if (spriteRects[s][i].x < 0)
				{
					 spriteMovement[s][i].x = -spriteMovement[s][i].x;
					 spriteRects[s][i].x = -spriteRects[s][i].x;
				}

				if (spriteRects[s][i].y < 0)
				{
					spriteMovement[s][i].y = -spriteMovement[s][i].y;
					spriteRects[s][i].y = -spriteRects[s][i].y;
				}

				if (spriteRects[s][i].x + spriteRects[s][i].w > canvasSize.w )
				{
					spriteMovement[s][i].x = -spriteMovement[s][i].x;
					spriteRects[s][i].x = canvasSize.w - spriteRects[s][i].w - (spriteRects[s][i].x + spriteRects[s][i].w - canvasSize.w);
				}

				if (spriteRects[s][i].y + spriteRects[s][i].h > canvasSize.h)
				{
					spriteMovement[s][i].y = -spriteMovement[s][i].y;
					spriteRects[s][i].y = canvasSize.h - spriteRects[s][i].h - (spriteRects[s][i].y + spriteRects[s][i].h - canvasSize.h);
				}

			}

			for (int i = 0; i < nSprites; i++)
			{
				RectSPX r = GfxUtil::alignUp(spriteRects[s][i]);

				// Align x to even pixels
	/*
				spx x2 = (r.x + r.w + 64) & 0xFFFFFF80;
				r.x = r.x & 0xFFFFFF80;
				r.w = x2 - r.x;
	*/
				// Align x to 4 pixel boundaries.

				spx x2 = (r.x + r.w + 64*3) & 0xFFFFFF00;
				r.x = r.x & 0xFFFFFF00;
				r.w = x2 - r.x;

				clip.add(r);
			}

//			for (int i = 0; i < nSprites; i++)
//				clip.add(GfxUtil::alignUp(spriteRects[s][i]));

			CanvasRef canvas = s == 0 ? canvasRef1 : canvasRef2;
			pDevice->beginCanvasUpdate(canvas, clip.size(), clip.begin() );

			pDevice->fill(Color::Black);

			for (int i = 0 ; i < nSprites; i++)
				pDevice->fill(spriteRects[s][i], spriteColors[i]);

			pDevice->endCanvasUpdate();
		}
		
		pDevice->endRender();
		ticker++;

	}
}



//____ playRectangleDanceDualScreen() _________________________________________________

void playRectangleDanceDualScreen(GfxDevice_p pDevice, CanvasRef canvasRef1, CanvasRef canvasRef2 )
{
	int ticker = 0;
	const int length = 600;
	SizeSPX spriteSize1(40*64, 40*64);
	SizeSPX canvasSize1 = pDevice->canvas(canvasRef1).size;
	SizeSPX moveDim1(canvasSize1.w - spriteSize1.w, canvasSize1.h - spriteSize1.h);

	SizeSPX spriteSize2(20*64, 20*64);
	SizeSPX canvasSize2 = pDevice->canvas(canvasRef2).size;
	SizeSPX moveDim2(canvasSize2.w - spriteSize2.w, canvasSize2.h - spriteSize2.h);

	while (ticker < length)
	{
		pDevice->beginRender();

		pDevice->beginCanvasUpdate(canvasRef1);
		
		pDevice->fill(Color::Black);
		pDevice->fill({ positionSprite(moveDim1, ticker, length, 0, 3),spriteSize1 }, Color::Red);
		pDevice->fill({ positionSprite(moveDim1, ticker, length, 1, 3),spriteSize1 }, Color::Green);
		pDevice->fill({ positionSprite(moveDim1, ticker, length, 2, 3),spriteSize1 }, Color::Blue);

		pDevice->endCanvasUpdate();

		pDevice->beginCanvasUpdate(canvasRef2);
		
		pDevice->fill(Color::Black);
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 8, 9),spriteSize2 }, Color(255,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 7, 9),spriteSize2 }, Color(224,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 6, 9),spriteSize2 }, Color(192,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 5, 9),spriteSize2 }, Color(160,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 4, 9),spriteSize2 }, Color(128,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 3, 9),spriteSize2 }, Color(96,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 2, 9),spriteSize2 }, Color(64,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 1, 9),spriteSize2 }, Color(32,0,0) );
		pDevice->fill({ positionSprite(moveDim2, ticker, length, 0, 9),spriteSize2 }, Color(0,0,0) );

		pDevice->endCanvasUpdate();


		pDevice->endRender();
		ticker++;
	}
}


//____ playScroll() _________________________________________________

void playScroll(GfxDevice_p pDevice, RectI canvas )
{
    int ticker = 0;
    SizeI spriteSize(30, 30);
    SizeI moveDim(canvas.w - spriteSize.w, canvas.h - spriteSize.h);

    
    while (ticker < 600)
    {
        CoordI pos = canvas.pos();
        pos.x += ticker;
        pos.y += (canvas.h - spriteSize.h) / 2;
        
        pDevice->beginRender();
        pDevice->beginCanvasUpdate(CanvasRef::Canvas_1);
        
        pDevice->fill( canvas, Color::Black);
        pDevice->fill( RectI( pos, spriteSize ), Color::Red);

        pDevice->endCanvasUpdate();
        pDevice->endRender();
        ticker++;
    }
}


CoordSPX positionSprite(SizeSPX dimensions, int tick, int length, int nb, int amount)
{
	const float PI = 3.14159265f;

	CoordSPX	radius = { dimensions.w / 2, dimensions.h / 2 };

	if (tick < 90)
		radius *= sin(tick*PI / 180);

	if (tick > length - 90)
		radius *= 1.f - sin((tick - (length - 90))*PI / 180);


	CoordSPX c;
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


