
#include <cstdlib>
#include <stdio.h>

#ifdef WIN32
#	include <SDL.h>
#elif __APPLE__
#	include <SDL2/SDL.h>
#else
#	include <SDL2/SDL.h>
#endif

#include <wax.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_gfxdevice_gen2.h>

#include <wax_c.h>
#include <waxgfxstream_c.h>
#include <wax_c_softgfx.h>

#include <string>


using namespace wax;
using namespace std;

//____ StreamPlayer ____________________________________________________________
//
// Plays a gfxstream into a pair of SoftSurface canvases, swapping them as frames
// complete, and shows the front canvas in an SDL window.

class StreamPlayer
{
public:
	bool		init( const char * pStreamPath );
	void		update();
	void		exit();

	Surface*	frontCanvas() const { return static_cast<SoftSurface*>(reinterpret_cast<Object*>(m_streamFrontCanvas)); }
	bool		hasNewFrame() { bool b = m_bNewFrame; m_bNewFrame = false; return b; }

private:

	bool		_loadStream( const char * pPath );
	bool		_setupStreamPlaying();

	Blob_p			m_pStreamBlob;

	SoftBackend_p	m_pBackend;

	wax_obj 		m_wrapper = nullptr;
	wax_obj 		m_player = nullptr;
	wax_obj 		m_pump = nullptr;

	wax_obj			m_streamFrontCanvas = nullptr;
	wax_obj			m_streamBackCanvas = nullptr;

	wax_patches		m_streamFrontPatches = nullptr;
	wax_patches		m_streamBackPatches = nullptr;

	bool			m_bNewFrame = true;
};

static const int	c_canvasWidth = 800;
static const int	c_canvasHeight = 480;

static void presentCanvas( Surface * pCanvas, SDL_Window * pWindow );

//____ main() _________________________________________________________________

int main( int argc, char** argv )
{
	if( argc < 2 )
	{
		printf("Usage: streamplayer <stream file>\n");
		return 1;
	}

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window * pWindow = SDL_CreateWindow("Wax Stream Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
											c_canvasWidth, c_canvasHeight, 0);
	if( !pWindow )
	{
		printf("ERROR: Could not create window: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	Core::init();

	int result = 0;

	{
		StreamPlayer player;

		if( player.init(argv[1]) )
		{
			bool bQuit = false;
			while( !bQuit )
			{
				SDL_Event e;
				while( SDL_PollEvent(&e) )
				{
					if( e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) )
						bQuit = true;
				}

				player.update();

				if( player.hasNewFrame() )
					presentCanvas( player.frontCanvas(), pWindow );

				SDL_Delay(10);
			}
		}
		else
			result = 1;

		player.exit();
	}

	Core::exit();

	SDL_DestroyWindow(pWindow);
	SDL_Quit();
	return result;
}

//____ presentCanvas() ________________________________________________________

static void presentCanvas( Surface * pCanvas, SDL_Window * pWindow )
{
	SDL_Surface * pWinSurf = SDL_GetWindowSurface(pWindow);
	if( !pWinSurf )
		return;

	auto pixbuf = pCanvas->allocPixelBuffer();
	pCanvas->pushPixels(pixbuf);

	// ARGB_8 is stored as B, G, R, A in memory on little endian machines,
	// which is what SDL calls ARGB8888.

	SDL_Surface * pSrc = SDL_CreateRGBSurfaceWithFormatFrom( pixbuf.pixels, pixbuf.rect.w, pixbuf.rect.h, 32,
															  pixbuf.pitch, SDL_PIXELFORMAT_ARGB8888 );
	if( pSrc )
	{
		SDL_BlitSurface( pSrc, nullptr, pWinSurf, nullptr );
		SDL_FreeSurface( pSrc );
		SDL_UpdateWindowSurface(pWindow);
	}

	pCanvas->freePixelBuffer(pixbuf);
}

//____ StreamPlayer::init() ___________________________________________________

bool StreamPlayer::init( const char * pStreamPath )
{
	if( !_loadStream(pStreamPath) )
	{
		printf("ERROR: Could not load stream '%s'\n", pStreamPath );
		return false;
	};

	return _setupStreamPlaying();
}

//____ StreamPlayer::update() _________________________________________________

void StreamPlayer::update()
{
	wax_defineSoftBackendCanvas(static_cast<Object*>(m_pBackend.rawPtr()), WAX_CANVAS_1, m_streamBackCanvas);

	if( wax_pumpUntilFrame(m_pump) )
	{
		wax_pumpFrame(m_pump);

		const wax_rectSPX* pDirtyRects;
		int				nDirtyRects;

		nDirtyRects = wax_getStreamPlayerDirtyRects(m_player, WAX_CANVAS_1, (const wax_rectI**) &pDirtyRects );

		if( nDirtyRects > 0 )
		{
			if( wax_patchesIsEmpty(m_streamBackPatches) == 1 )
			{
				for( int i = 0 ; i < nDirtyRects ; i++ )
					wax_pushPatch(m_streamBackPatches, pDirtyRects[i] );
			}
			else
			{
				for( int i = 0 ; i < nDirtyRects ; i++ )
					wax_addPatch(m_streamBackPatches, pDirtyRects[i] );
			}

			wax_clearStreamPlayerDirtyRects(m_player);
		}
	};

	if( wax_patchesIsEmpty(m_streamFrontPatches) == 0 )
	{
		wax_subPatches(m_streamFrontPatches, m_streamBackPatches);

		if( wax_patchesIsEmpty(m_streamFrontPatches) ==0 )
		{
			int pixelBytes = 4;

			wax_pixelBuffer frontBuffer;
			wax_pixelBuffer backBuffer;

			frontBuffer = wax_allocPixelBuffer(m_streamFrontCanvas);
			backBuffer = wax_allocPixelBuffer(m_streamBackCanvas);

			wax_pushPixels(m_streamFrontCanvas, &frontBuffer);
			wax_pushPixels(m_streamBackCanvas, &backBuffer);

			const wax_rectSPX * pRect = wax_patchesBegin(m_streamFrontPatches);
			const wax_rectSPX * pEnd = wax_patchesEnd(m_streamFrontPatches);

			while( pRect < pEnd )
			{
				RectI rect = { pRect->x/64, pRect->y/64, pRect->w/64, pRect->h/64 };

				int nBlocks = (rect.w*pixelBytes) / 8;
				int nExtraShorts = ((rect.w*pixelBytes) % 8) / 2;

				int pitch = frontBuffer.pitch - rect.w*pixelBytes;

				uint8_t * pSrc = frontBuffer.pixels + frontBuffer.pitch * rect.y + rect.x * pixelBytes;
				uint8_t * pDst = backBuffer.pixels + frontBuffer.pitch * rect.y + rect.x * pixelBytes;


				// Copy rect from front to back

				for( int y = 0 ; y < rect.h ; y++ )
				{

					for( int x = 0 ; x < nBlocks ; x++ )
					{
						((uint32_t *)pDst)[0] = ((uint32_t *)pSrc)[0];
						((uint32_t *)pDst)[1] = ((uint32_t *)pSrc)[1];
						pSrc += 8;
						pDst += 8;
					}

					for( int x = 0 ; x < nExtraShorts ; x++ )
					{
						((uint16_t *)pDst)[0] = ((uint16_t *)pSrc)[0];
						pSrc += 2;
						pDst += 2;
					}

					pSrc += pitch;
					pDst += pitch;
				}

				pRect++;
			}


			wax_pullPixels(m_streamBackCanvas, &backBuffer);
			wax_freePixelBuffer(m_streamFrontCanvas, &frontBuffer);
			wax_freePixelBuffer(m_streamBackCanvas, &backBuffer);

			wax_clearPatches(m_streamFrontPatches);
		}
	}

	int screenIsReady = 1;
	if( wax_patchesIsEmpty(m_streamBackPatches) == 0 && screenIsReady == 1 )
	{
		// Swap front and back

		wax_obj temp = m_streamFrontCanvas;
		m_streamFrontCanvas = m_streamBackCanvas;
		m_streamBackCanvas = temp;

		wax_patches temp2 = m_streamFrontPatches;
		m_streamFrontPatches = m_streamBackPatches;
		m_streamBackPatches = temp2;

		// Front now holds the new frame, which goes to the screen.

		m_bNewFrame = true;
	}
}

//____ StreamPlayer::exit() ___________________________________________________

void StreamPlayer::exit()
{
	if( m_pump )
		wax_release(m_pump);
	if( m_player )
		wax_release(m_player);
	if( m_wrapper )
		wax_release(m_wrapper);

	if( m_streamFrontCanvas )
		wax_release(m_streamFrontCanvas);

	if( m_streamBackCanvas )
		wax_release(m_streamBackCanvas);

	if( m_streamFrontPatches )
		wax_deletePatches(m_streamFrontPatches);

	if( m_streamBackPatches )
		wax_deletePatches(m_streamBackPatches);

	m_pump = m_player = m_wrapper = m_streamFrontCanvas = m_streamBackCanvas = nullptr;
	m_streamFrontPatches = m_streamBackPatches = nullptr;

	m_pBackend = nullptr;
	m_pStreamBlob = nullptr;
}

//____ StreamPlayer::_loadStream() ____________________________________________

bool StreamPlayer::_loadStream( const char * pPath )
{
	FILE * fp = fopen( pPath, "rb" );
	if( !fp )
		return false;

	fseek( fp, 0, SEEK_END );
	int size = (int) ftell(fp);
	fseek( fp, 0, SEEK_SET );

	m_pStreamBlob = Blob::create( size );

	int nRead = (int) fread( m_pStreamBlob->data(), 1, size, fp );
	fclose( fp );

	if( nRead < size )
	{
		m_pStreamBlob = nullptr;
		return false;
	}

	return true;
}

//____ StreamPlayer::_setupStreamPlaying() ____________________________________

bool StreamPlayer::_setupStreamPlaying()
{
	m_pBackend = SoftBackend::create();
	addDefaultSoftKernels(m_pBackend);

	auto pSurfaceFactory = SoftSurfaceFactory::create();
	auto pEdgemapFactory = SoftEdgemapFactory::create();

	// We use the C-interface here just to try it! B-)

	wax_surfaceBP bp = { .canvas = 1, .format = WAX_PIXFMT_ARGB_8, .size = {c_canvasWidth,c_canvasHeight} };

	m_streamFrontCanvas		= wax_createSoftSurface( &bp );
	m_streamFrontPatches	= wax_createPatches();

	m_streamBackCanvas		= wax_createSoftSurface( &bp );
	m_streamBackPatches		= wax_createPatches();

	wax_obj	backend = static_cast<Object*>(m_pBackend.rawPtr());
	wax_obj	surfaceFactory = static_cast<Object*>(pSurfaceFactory.rawPtr());
	wax_obj	edgemapFactory = static_cast<Object*>(pEdgemapFactory.rawPtr());

	wax_defineSoftBackendCanvas(backend, WAX_CANVAS_1, m_streamFrontCanvas);

	m_wrapper = wax_createStreamWrapper(m_pStreamBlob->begin(), m_pStreamBlob->end());
	m_player = wax_createStreamPlayer(backend, surfaceFactory, edgemapFactory );

	wax_component tap = wax_getStreamWrapperOutput(m_wrapper);
	wax_component sink =wax_getStreamPlayerInput(m_player);

	m_pump = wax_createStreamPumpWithInputOutput(tap, sink);

	wax_setStreamPlayerStoreDirtyRects(m_player, 1);
	wax_setStreamPlayerMaxDirtyRects(m_player, 60);

	return true;
}
