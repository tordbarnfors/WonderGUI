
#include "streamplayer.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <wg_softbackend.h>
#include <extensions/gfxstream/c-api/wondergfxstream_c.h>
#include <wg_c_softgfx.h>
#include <string>
#include <fstream>


using namespace wg;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(Visitor* pVisitor)
{
	m_pAppVisitor = pVisitor;

	std::string	streampath;

	auto arguments = pVisitor->programArguments();

	if( arguments.size() >= 1 )
		streampath = arguments[0];
	else
	{
		std::vector<std::string>	filetypes;

		streampath = pVisitor->openFileDialog("Load gfxstream", "", filetypes, "");
	}

	if( !_loadStream(streampath) )
	{
		printf("ERROR: Could not load stream '%s'\n", streampath.c_str() );
		return false;
	};

	if (!_setupGUI(pVisitor))
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
	wg_defineSoftBackendCanvas(m_streamGfxDevice, WG_CANVAS_1, m_streamBackCanvas);

	if( wg_pumpUntilFrame(m_pump) )
	{
		wg_pumpFrame(m_pump);
//		wg_pumpAllFramesOptimizeClipping(m_pump, 5);		// TODO: Set to CONFIG_USB_DISPLAY_FRAME_OPTIMIZATION_DEPTH

		const wg_rectSPX* pDirtyRects;
		int				nDirtyRects;

		nDirtyRects = wg_getStreamPlayerDirtyRects(m_player, WG_CANVAS_1, (const wg_rectI**) &pDirtyRects );

		if( nDirtyRects > 0 )
		{
			if( wg_patchesIsEmpty(m_streamBackPatches) == 1 )
			{
				for( int i = 0 ; i < nDirtyRects ; i++ )
					wg_pushPatch(m_streamBackPatches, pDirtyRects[i] );
			}
			else
			{
				for( int i = 0 ; i < nDirtyRects ; i++ )
					wg_addPatch(m_streamBackPatches, pDirtyRects[i] );
			}

			wg_clearStreamPlayerDirtyRects(m_player);
		}
	};

	if( wg_patchesIsEmpty(m_streamFrontPatches) == 0 )
	{
		wg_subPatches(m_streamFrontPatches, m_streamBackPatches);

		if( wg_patchesIsEmpty(m_streamFrontPatches) ==0 )
		{
			int pixelBytes = 4;

			wg_pixelBuffer frontBuffer;
			wg_pixelBuffer backBuffer;

			frontBuffer = wg_allocPixelBuffer(m_streamFrontCanvas);
			backBuffer = wg_allocPixelBuffer(m_streamBackCanvas);

			wg_pushPixels(m_streamFrontCanvas, &frontBuffer);
			wg_pushPixels(m_streamBackCanvas, &backBuffer);

			const wg_rectSPX * pRect = wg_patchesBegin(m_streamFrontPatches);
			const wg_rectSPX * pEnd = wg_patchesEnd(m_streamFrontPatches);

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


			wg_pullPixels(m_streamBackCanvas, &backBuffer);
			wg_freePixelBuffer(m_streamFrontCanvas, &frontBuffer);
			wg_freePixelBuffer(m_streamBackCanvas, &backBuffer);

			wg_clearPatches(m_streamFrontPatches);
		}
	}

	int screenIsReady = 1;
	if( wg_patchesIsEmpty(m_streamBackPatches) == 0 && screenIsReady == 1 )
	{
		// Swap front and back

		wg_obj temp = m_streamFrontCanvas;
		m_streamFrontCanvas = m_streamBackCanvas;
		m_streamBackCanvas = temp;

		wg_patches temp2 = m_streamFrontPatches;
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
		wg_release(m_pump);
	if( m_player )
		wg_release(m_player);
	if( m_wrapper )
		wg_release(m_wrapper);

	if( m_streamFrontCanvas )
		wg_release(m_streamFrontCanvas);

	if( m_streamBackCanvas )
		wg_release(m_streamBackCanvas);

	if( m_streamFrontPatches )
		wg_deletePatches(m_streamFrontPatches);

	if( m_streamBackPatches )
		wg_deletePatches(m_streamBackPatches);
}

//____ _loadStream() __________________________________________________________

bool MyApp::_loadStream(std::string path)
{
	m_pStreamBlob = m_pAppVisitor->loadBlob(path);

	if( !m_pStreamBlob )
		return false;

	return true;
}

//____ _setupStreamPlaying() __________________________________________________

bool MyApp::_setupStreamPlaying()
{
	// We use the C-interface here just to try it! B-)

	wg_surfaceBP bp = { .canvas = 1, .size = {800,480} };

	m_streamFrontCanvas		= wg_createSoftSurface( &bp );
	m_streamFrontPatches	= wg_createPatches();

	m_streamBackCanvas		= wg_createSoftSurface( &bp );
	m_streamBackPatches		= wg_createPatches();


	m_pDisplay->setSurface( static_cast<SoftSurface*>(reinterpret_cast<Object*>(m_streamFrontCanvas)) );

	m_streamGfxDevice = static_cast<Object*>(Base::defaultGfxDevice());

	wg_defineSoftBackendCanvas(m_streamGfxDevice, WG_CANVAS_1, m_streamFrontCanvas);


	wg_obj	gfxDevice = static_cast<Object*>(Base::defaultGfxDevice());
	wg_obj	surfaceFactory = static_cast<Object*>(Base::defaultSurfaceFactory());
	wg_obj	edgemapFactory = static_cast<Object*>(Base::defaultEdgemapFactory());

	

	m_wrapper = wg_createStreamWrapper(m_pStreamBlob->begin(), m_pStreamBlob->end());
	m_player = wg_createStreamPlayer(gfxDevice, surfaceFactory, edgemapFactory );

	wg_component tap = wg_getStreamWrapperOutput(m_wrapper);
	wg_component sink =wg_getStreamPlayerInput(m_player);


	m_pump = wg_createStreamPumpWithInputOutput(tap, sink);

	wg_setStreamPlayerStoreDirtyRects(m_player, 1);
	wg_setStreamPlayerMaxDirtyRects(m_player, 60);

	return true;
}



//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,480}, .title = "WonderGUI Stream Player" });

	//

	auto pFontBlob = pVisitor->loadBlob("resources/DroidSans.ttf");
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

	m_pWindow->setContent(m_pDisplay);
	return true;
}
