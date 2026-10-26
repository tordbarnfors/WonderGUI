
#include "streamplayer.h"

#include <wondergui.h>
#include <wax_freetypefont.h>
#include <wax_softbackend.h>
#include <waxgfxstream_c.h>
#include <wax_c_softgfx.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace wax;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(wapp::API* pAPI)
{
	m_pAppAPI = pAPI;

	std::string	streampath;

	auto arguments = pAPI->programArguments();

	if( arguments.size() >= 1 )
		streampath = arguments[0];
	else
	{
		std::vector<std::string>	filetypes;

		streampath = pAPI->openFileDialog("Load gfxstream", "", "", filetypes, "");
	}

	if( !_loadStream(streampath) )
	{
		printf("ERROR: Could not load stream '%s'\n", streampath.c_str() );
		return false;
	};

	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}


	_setupStreamPlaying();

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	wax_defineSoftBackendCanvas(m_streamGfxDevice, WAX_CANVAS_1, m_streamBackCanvas);

	if( wax_pumpUntilFrame(m_pump) )
	{
		wax_pumpFrame(m_pump);
//		wg_pumpAllFramesOptimizeClipping(m_pump, 5);		// TODO: Set to CONFIG_USB_DISPLAY_FRAME_OPTIMIZATION_DEPTH

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

		// Send content of front to screen.

		m_pDisplay->setSurface( static_cast<SoftSurface*>(reinterpret_cast<Object*>(m_streamFrontCanvas)) );
	}


	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
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
}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(wapp::Window* pWindow)
{
	if (pWindow == m_pWindow)
		m_pWindow = nullptr;
}


//____ _loadStream() __________________________________________________________

bool MyApp::_loadStream(std::string path)
{
	m_pStreamBlob = m_pAppAPI->loadBlob(path);

	if( !m_pStreamBlob )
		return false;

	return true;
}

//____ _setupStreamPlaying() __________________________________________________

bool MyApp::_setupStreamPlaying()
{
	// We use the C-interface here just to try it! B-)

	wax_surfaceBP bp = { .canvas = 1, .size = {800,480} };

	m_streamFrontCanvas		= wax_createSoftSurface( &bp );
	m_streamFrontPatches	= wax_createPatches();

	m_streamBackCanvas		= wax_createSoftSurface( &bp );
	m_streamBackPatches		= wax_createPatches();


	m_pDisplay->setSurface( static_cast<SoftSurface*>(reinterpret_cast<Object*>(m_streamFrontCanvas)) );

	m_streamGfxDevice = static_cast<Object*>(Base::defaultGfxDevice());

	wax_defineSoftBackendCanvas(m_streamGfxDevice, WAX_CANVAS_1, m_streamFrontCanvas);


	wax_obj	gfxDevice = static_cast<Object*>(Base::defaultGfxDevice());
	wax_obj	surfaceFactory = static_cast<Object*>(Base::defaultSurfaceFactory());
	wax_obj	edgemapFactory = static_cast<Object*>(Base::defaultEdgemapFactory());

	

	m_wrapper = wax_createStreamWrapper(m_pStreamBlob->begin(), m_pStreamBlob->end());
	m_player = wax_createStreamPlayer(gfxDevice, surfaceFactory, edgemapFactory );

	wax_component tap = wax_getStreamWrapperOutput(m_wrapper);
	wax_component sink =wax_getStreamPlayerInput(m_player);


	m_pump = wax_createStreamPumpWithInputOutput(tap, sink);

	wax_setStreamPlayerStoreDirtyRects(m_player, 1);
	wax_setStreamPlayerMaxDirtyRects(m_player, 60);

	return true;
}



//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(wapp::API* pAPI)
{
	m_pWindow = wapp::Window::create(pAPI, { .size = {800,480}, .title = "WonderGUI Stream Player" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}
	} ));

	Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = BasicTextLayout::create(WGBP(BasicTextLayout,
		_.placement = Placement::Center));


	//

	auto pBgSkin = ColorSkin::create( Color::PapayaWhip );

	m_pDisplay = SurfaceDisplay::create( WGBP(SurfaceDisplay,
											  _.skin = pBgSkin));

	m_pWindow->mainCapsule()->slot = m_pDisplay;
	return true;
}
