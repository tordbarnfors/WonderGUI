#include "sdl_wglib.h"
//#include <wg_surface_sdl.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_bitmapfont.h>
#include <wglegacygui.h>

namespace sdl_wglib
{
	static WgEventHandler * g_pHandler;
	static int				g_ticks = 0;

	//____ MapKeys() ___________________________________________________________

	void MapKeys()
	{
		WgBase::MapKey( WG_KEY_SHIFT, SDLK_LSHIFT );
		WgBase::MapKey( WG_KEY_SHIFT, SDLK_RSHIFT );
		WgBase::MapKey( WG_KEY_STD_CONTROL, SDLK_LCTRL );
		WgBase::MapKey( WG_KEY_STD_CONTROL, SDLK_RCTRL );
		WgBase::MapKey( WG_KEY_ALT, SDLK_LALT );
		WgBase::MapKey( WG_KEY_ALT, SDLK_RALT );

		WgBase::MapKey( WG_KEY_LEFT, SDLK_LEFT );
		WgBase::MapKey( WG_KEY_RIGHT, SDLK_RIGHT );
		WgBase::MapKey( WG_KEY_UP, SDLK_UP );
		WgBase::MapKey( WG_KEY_DOWN, SDLK_DOWN );

		WgBase::MapKey( WG_KEY_HOME, SDLK_HOME );
		WgBase::MapKey( WG_KEY_END, SDLK_END );
		WgBase::MapKey( WG_KEY_PAGE_UP, SDLK_PAGEUP );
		WgBase::MapKey( WG_KEY_PAGE_DOWN, SDLK_PAGEDOWN );

		WgBase::MapKey( WG_KEY_RETURN, SDLK_RETURN );
		WgBase::MapKey( WG_KEY_BACKSPACE, SDLK_BACKSPACE );
		WgBase::MapKey( WG_KEY_DELETE, SDLK_DELETE );
		WgBase::MapKey( WG_KEY_TAB, SDLK_TAB );
		WgBase::MapKey( WG_KEY_ESCAPE, SDLK_ESCAPE );
	}

	//____ BeginEvents() _______________________________________________________

	bool g_bStopScroll = false;

	void BeginEvents( WgEventHandler * pHandler )
	{
		g_pHandler = pHandler;

		// Add a tick event first as the first.

		int ticks = SDL_GetTicks();
		if( !g_bStopScroll )
			pHandler->QueueEvent( new WgEvent::Tick( ticks - g_ticks ) );
		g_ticks = ticks;
	}

	//____ TranslateEvent() ____________________________________________________

	void TranslateEvent( SDL_Event& event )
	{
		switch (event.type)
		{
			// check for keypresses
			case SDL_KEYDOWN:
			{
				g_pHandler->QueueEvent( new WgEvent::KeyPress( event.key.keysym.sym ) );
				if( event.key.keysym.sym == SDLK_RETURN )
                    g_pHandler->QueueEvent( new WgEvent::Character( 13 ) );
				break;
			}

			case SDL_KEYUP:
			{
				g_pHandler->QueueEvent( new WgEvent::KeyRelease( event.key.keysym.sym ) );
				break;
			}

			case	SDL_MOUSEMOTION:
			{
				g_pHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( event.motion.x, event.motion.y ) ) );
				break;
			}

			case	SDL_MOUSEBUTTONDOWN:

				if (event.button.button == 2)
					g_bStopScroll = !g_bStopScroll;
				if (event.button.button == 3)
				{
					g_pHandler->QueueEvent(new WgEvent::Tick(15));
				}

				if(event.button.button == 4 )
					g_pHandler->QueueEvent( new WgEvent::MouseWheelRoll( 1, 1 ) );
				else if(event.button.button == 5)
					g_pHandler->QueueEvent( new WgEvent::MouseWheelRoll( 1, -1 ) );
				else
				{
					g_pHandler->QueueEvent( new WgEvent::MouseButtonPress( event.button.button ) );
				}
				break;

			case	SDL_MOUSEBUTTONUP:
				if( event.button.button != 4 && event.button.button != 5 )
					g_pHandler->QueueEvent( new WgEvent::MouseButtonRelease( event.button.button ) );	
				break;
			case SDL_TEXTINPUT:
				g_pHandler->QueueEvent(new WgEvent::Character((short)event.text.text[0]));
				break;

		}
	}

	//_____ EndEvents() ________________________________________________________

	void EndEvents()
	{
		g_pHandler->ProcessEvents();
	}

	//____ SavePNG() ________________________________________________________

    bool SavePNG(wg::Surface * pSurface, const char * path)
	{
		WgSize size = pSurface->pixelSize();
		const WgPixelFormat * pFmt = pSurface->pixelDescription();

		SDL_Surface * pOutput = SDL_CreateRGBSurface(0, size.w, size.h, pFmt->bits, (unsigned int) pFmt->R_mask, (unsigned int) pFmt->G_mask, (unsigned int) pFmt->B_mask, (unsigned int) pFmt->A_mask);


		int err = SDL_LockSurface(pOutput);
		if (err != 0)
			return false;

        wg::PixelBuffer pixbuf = pSurface->allocPixelBuffer();
        pSurface->pushPixels(pixbuf);

		for (int y = 0; y < size.h; y++)
		{
			uint8_t * pDest = ((uint8_t *) pOutput->pixels) + pOutput->pitch * y;
            uint8_t * pSource = pixbuf.pixels + pixbuf.pitch * y;
			memcpy(pDest, pSource, size.w * pFmt->bits / 8);
		}

        pSurface->freePixelBuffer(pixbuf);

		SDL_UnlockSurface(pOutput);


		int err2 = IMG_SavePNG(pOutput, path);
		SDL_FreeSurface(pOutput);

		return true;
	}


	//____ LoadSurface() __________________________________________________________

    wg::Surface_p LoadSurface( const char * path, wg::SurfaceFactory * factory, const wg::Surface::Blueprint& bp )
	{
		SDL_Surface* bmp = IMG_Load(path);
		if (!bmp)
		{
			printf("Unable to load bitmap: %s\n", IMG_GetError());
			return 0;
		}

//		WgSurfaceSDL	wrapper( bmp );

		WgPixelType type;
		
		if( bmp->format->Amask == 0 )
            type = WgPixelType::BGR_8;
		else
            type = WgPixelType::BGRA_8;

		WgSize dimensions( bmp->w, bmp->h );

		WgPixelFormat	pixelFormat;
		ConvertPixelFormat( &pixelFormat, bmp->format );

		wg::Surface::Blueprint bp2 = bp;
		bp2.size = dimensions;
		bp2.format = type;
		
		
        wg::Surface_p pSurf = factory->createSurface( bp2, (uint8_t*) bmp->pixels, pixelFormat, bmp->pitch );

		if( !pSurf )
		{
			printf("Unable to create surface for loaded bitmap: %s\n", path);
			return 0;
		}

		SDL_FreeSurface( bmp );
		return pSurf;
	}

	//____ LoadStdWidgets() _____________________________________________________

    std::map<std::string,WgWidget*> * LoadStdWidgets( const char * pImagePath, const char * pImagePathX2, const char * pImagePathX4, wg::SurfaceFactory* factory )
	{
		const int HSLIDER_BTN_OFS 		= 1;
		const int VSLIDER_BTN_OFS 		= HSLIDER_BTN_OFS + 19;
		const int SLIDER_OFS 			= VSLIDER_BTN_OFS + 19;
		const int SLIDER_BACK_OFS		= SLIDER_OFS + 10;
		const int RESIZE_BUTTON_OFS 	= SLIDER_BACK_OFS + 13;
		const int CHECKBOX_OFS 			= RESIZE_BUTTON_OFS + 22;
		const int RADIOBUTTON_OFS 		= CHECKBOX_OFS + 13;
		const int BUTTON_OFS			= RADIOBUTTON_OFS + 13;
		const int PLATE_OFS				= BUTTON_OFS + 10;
		const int SPLITS_AND_FRAME_OFS	= PLATE_OFS + 10;
		const int COMBOBOX_OFS			= SPLITS_AND_FRAME_OFS + 10;
		const int TILES_OFS				= 192;

        wg::Surface_p surfaces[3];

        wg::Surface_p pSurface = LoadSurface( pImagePath, factory );
		if( !pSurface )
			return 0;

		surfaces[0] = pSurface;
		surfaces[1] = pSurface;
		surfaces[2] = pSurface;

		if( pImagePathX2 )
		{
			auto pSurfaceX2 = LoadSurface( pImagePathX2, factory );
			if( !pSurfaceX2 )
				return 0;
				
			surfaces[1] = pSurfaceX2;
			surfaces[2] = pSurfaceX2;
		}


		if( pImagePathX4 )
		{
			auto pSurfaceX4 = LoadSurface( pImagePathX4, factory );
			if( !pSurfaceX4 )
				return 0;
				
			surfaces[2] = pSurfaceX4;
		}

		wg::BlockSkin::Blueprint	sliderBtnBP {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.frame = 3,
			.padding = 4,
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface
		};
		
		sliderBtnBP.firstBlock = wg::Rect(1,HSLIDER_BTN_OFS,17,17);
		auto pHSliderBtnBwdBlocks = wg::BlockSkin::create( sliderBtnBP );
		
		sliderBtnBP.firstBlock = wg::Rect(77,HSLIDER_BTN_OFS,17,17);
		auto pHSliderBtnFwdBlocks = wg::BlockSkin::create( sliderBtnBP );

		sliderBtnBP.firstBlock = wg::Rect(1,VSLIDER_BTN_OFS,17,17);
		auto pVSliderBtnBwdBlocks = wg::BlockSkin::create( sliderBtnBP );

		sliderBtnBP.firstBlock = wg::Rect(77,VSLIDER_BTN_OFS,17,17);
		auto pVSliderBtnFwdBlocks = wg::BlockSkin::create( sliderBtnBP );

		
//		auto pHSliderBtnBwdBlocks	= WgBlockset::CreateFromRow(pSurface, WgRect(1,HSLIDER_BTN_OFS,74,17), 4, 2, WG_OPAQUE);
//		pHSliderBtnBwdBlocks->setFrame(wg::Border(3));
//		pHSliderBtnBwdBlocks->setContentPadding(WgBorders(4));


		auto pSliderBlocks	= wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,SLIDER_OFS,8,8),
			.frame = 2,
			.padding = 3,
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface
			} );
		
		auto pSliderBackBlocks	= wg::BlockSkin::create( { .firstBlock = wg::Rect(1,SLIDER_BACK_OFS,5,5), .frame = 2, .padding = 2, .surface = pSurface } );
		
		auto pResizeButtonBlocks = wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,RESIZE_BUTTON_OFS,20,20),
			.frame = 3,
			.padding = 3,
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface
			} );
		
		auto pCheckboxIconSkin = wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,CHECKBOX_OFS,11,11),
			.frame = 2,
			.padding = 3,
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {},
				wg::StateEnum::Checked, {},
				wg::StateEnum::HoveredChecked, {},
				wg::StateEnum::PressedChecked, {},
				wg::StateEnum::DisabledChecked, {},
			},
			.surface = pSurface
		} );
		
		auto pRadiobuttonIconSkin = wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,RADIOBUTTON_OFS,11,11),
			.frame = 2,
			.padding = 3,
			.states = {
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {},
				wg::StateEnum::Checked, {},
				wg::StateEnum::HoveredChecked, {},
				wg::StateEnum::PressedChecked, {},
				wg::StateEnum::DisabledChecked, {}
			},
			.surface = pSurface
		});
		
		auto pButtonBlocks = wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect( 1, BUTTON_OFS, 8, 8 ),
			.frame = 3,
			.padding = 4,
			.states = {
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface,
		});
		
//		auto pButtonBlocks = WgBlockset::CreateFromRects(pSurface, buttonRects[0], buttonRects[1], buttonRects[2], buttonRects[3], WG_OPAQUE);
//		pButtonBlocks->setFrame(buttonFrame);
//		pButtonBlocks->setContentPadding(buttonPadding);
/*
		for( int i = 0 ; i < 2 ; i++ )
		{
			for( int r = 0 ; r < 4 ; r++ )
			{
				buttonRects[r].x *= 2;
				buttonRects[r].y *= 2;
				buttonRects[r].w *= 2;
				buttonRects[r].h *= 2;
			}
			
			buttonFrame = WgBorders( buttonFrame.left*2, buttonFrame.top*2, buttonFrame.right*2, buttonFrame.bottom*2 );
			buttonPadding = WgBorders( buttonPadding.left*2, buttonPadding.top*2, buttonPadding.right*2, buttonPadding.bottom*2 );
			
			pButtonBlocks->AddAlternative( WG_SCALE_BASE << (i+1), surfaces[i+1], buttonRects[0], buttonRects[1], buttonRects[2], buttonRects[3], buttonRects[3], 
					buttonFrame, buttonPadding, WgCoord(), WgCoord() );
		
		}
*/

		auto pPlateBlocks	= wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,PLATE_OFS,8,8),
			.frame = 3,
			.padding = 4,
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface
			} );


		auto pHSplitBlocks = wg::BlockSkin::create( { .firstBlock = wg::Rect( 1,SPLITS_AND_FRAME_OFS,8,2 ), .surface = pSurface } );

		auto pVSplitBlocks = wg::BlockSkin::create( { .firstBlock =  { 11,SPLITS_AND_FRAME_OFS,2,8}, .surface = pSurface } );

		auto pFrameBlocks = wg::BlockSkin::create( { .firstBlock =  { 1,SPLITS_AND_FRAME_OFS,8,8 }, .frame = 2, .padding = 3, .surface = pSurface });

		auto pComboBlocks	= wg::BlockSkin::create( {
			.axis = wg::Axis::X,
			.blockSpacing = 2,
			.firstBlock = wg::Rect(1,COMBOBOX_OFS,23,20),
			.frame = {1,1,20,1},
			.padding = {2,2,21,2},
			.states = {
				wg::StateEnum::Default, {},
				wg::StateEnum::Hovered, {},
				wg::StateEnum::Pressed, {},
				wg::StateEnum::Disabled, {}
			},
			.surface = pSurface
			} );

//		auto pBgCheckeredGreyBlocks = WgBlockset::CreateFromRect( pSurface, WgRect(0,TILES_OFS,64,64), WG_OPAQUE | WG_TILE_ALL );
//		auto pBgBlueGradientBlocks = WgBlockset::CreateFromRect( pSurface, WgRect(1*64,TILES_OFS,64,64), WG_OPAQUE );

		WgColorsetPtr pSelectionColors = WgColorset::Create( WgColor(0x0), WgColor(0x40FFFFFF), WgColor(0x80FFFFFF), WgColor(0x40000000), WgColor(0x0) );


        std::map<std::string,WgWidget*> * pDB = new std::map<std::string,WgWidget*>;



		// Create standard button

		WgButton * pButton = new WgButton();
		pButton->SetSource( pButtonBlocks );
        (*pDB)["button"] = pButton;

		// Create standard plate

		WgImage * pPlate = new WgImage();
		pPlate->SetSource( pPlateBlocks );
        (*pDB)["plate"] = pPlate;

		// Create standard checkbox

		WgCheckBox * pCheckbox = new WgCheckBox();
		pCheckbox->SetIcon( pCheckboxIconSkin );
        (*pDB)["checkbox"] = pCheckbox;

		// Create standard radiobutton

		WgRadioButton * pRadiobutton = new WgRadioButton();
		pRadiobutton->SetIcon( pRadiobuttonIconSkin );
        (*pDB)["radiobutton"] = pRadiobutton;

		// Create standard horizontal slider

		WgHSlider * pHSlider = new WgHSlider();
		pHSlider->SetSource( pSliderBackBlocks, pSliderBlocks, pHSliderBtnBwdBlocks, pHSliderBtnFwdBlocks );
        (*pDB)["hslider"] = pHSlider;

		// Create standard vertical slider

		WgVSlider * pVSlider = new WgVSlider();
		pVSlider->SetSource( pSliderBackBlocks, pSliderBlocks, pVSliderBtnBwdBlocks, pVSliderBtnFwdBlocks );
        (*pDB)["vslider"] = pVSlider;

		// Create standard menubar
/*
		WgMenubar * pMenubar = new WgMenubar();
		pMenubar->SetBgSource( pPlateBlocks );
		pDB->AddWidget( "menubar", pMenubar );
*/
		// Create Background bitmaps
/*
		WgImage * pBgCheckeredGrey = new WgImage();
		pBgCheckeredGrey->SetSource( pBgCheckeredGreyBlocks );
        (*pDB)["bg_checkered_grey"] = pBgCheckeredGrey;

		WgImage * pBgBlueGradient = new WgImage();
		pBgBlueGradient->SetSource( pBgBlueGradientBlocks );
        (*pDB)["bg_blue_gradient"] = pBgBlueGradient;
*/
		// Create standard menu
/*
		WgMenu * pMenu = new WgMenu();
		pMenu->SetBgSource( pPlateBlocks, 16, 16 );
		pMenu->SetSeparatorSource( pHSplitBlocks, WgBorders(1) );
		pMenu->SetCheckBoxSource( pCheckboxUncheckedBlocks, pCheckboxCheckedBlocks);
		pMenu->SetRadioButtonSource( pRadiobuttonUncheckedBlocks, pRadiobuttonCheckedBlocks);
		pMenu->SetSliderSource( pSliderBackBlocks, pSliderBlocks, pVSliderBtnBwdBlocks, pVSliderBtnFwdBlocks );
		pMenu->SetTileColors( pSelectionColors );
		pDB->AddWidget( "menu", pMenu );
*/
		// Create standard combobox
/*
		WgCombobox * pCombobox = new WgCombobox();
		pCombobox->SetSource( pComboboxBlocks );
		pDB->AddWidget( "combobox", pCombobox );
*/
		// Create standard view
		
		{
			WgScrollPanel * pView = new WgScrollPanel();
			
            auto pHSlider2 = new WgHSlider();
            pHSlider2->CloneContent(pHSlider);

            auto pVSlider2 = new WgVSlider();
            pVSlider2->CloneContent(pVSlider);

			if( pHSlider )
				pView->SetHSlider( pHSlider2 );
			if( pVSlider )
				pView->SetVSlider( pVSlider2 );

			pView->SetFillerBlocks( pPlateBlocks );
            (*pDB)["view"] = pView;
		}

		return pDB;
	}

	//____ LoadBitmapFont() ____________________________________________________

    wg::Font_p LoadBitmapFont( const char * pImgPath, const char * pSpecPath, wg::SurfaceFactory * factory )
	{
		//TODO: This leaks memory until we have ref-counted

		auto pFontSurf = sdl_wglib::LoadSurface( pImgPath, factory );

		auto pFontSpec = (char*) LoadFile( pSpecPath );

        auto pFont = wg::BitmapFont::create( pFontSurf, pFontSpec );

		free( pFontSpec );
		return pFont;
	}

	//____ FileSize() _____________________________________________________________

	int FileSize( const char * pPath )
	{
		FILE * fp = fopen( pPath, "rb" );
		if( !fp )
			return 0;

		fseek( fp, 0, SEEK_END );
		int size = (int) ftell(fp);
		fseek( fp, 0, SEEK_SET );
		fclose( fp );

		return size;
	}

	//____ LoadFile() _____________________________________________________________

	void * LoadFile( const char * pPath )
	{
		FILE * fp = fopen( pPath, "rb" );
		if( !fp )
			return 0;

		fseek( fp, 0, SEEK_END );
		int size = (int) ftell(fp);
		fseek( fp, 0, SEEK_SET );

		char * pMem = (char*) malloc( size+1 );
		pMem[size] = 0;
		int nRead = (int) fread( pMem, 1, size, fp );
		fclose( fp );

		if( nRead < size )
		{
			free( pMem );
			return 0;
		}

		return pMem;
	}
	
	//____ ConvertPixelFormat() ______________________________________________________

	void ConvertPixelFormat( WgPixelFormat * pWGFormat, const SDL_PixelFormat * pSDLFormat )
	{
		pWGFormat->bits = pSDLFormat->BitsPerPixel;
		pWGFormat->type = wg::PixelType::Chunky;

		pWGFormat->R_mask = pSDLFormat->Rmask;
		pWGFormat->G_mask = pSDLFormat->Gmask;
		pWGFormat->B_mask = pSDLFormat->Bmask;
		pWGFormat->A_mask = pSDLFormat->Amask;
	}
	
	
};
