#include <sdl_wglib.h>
#include <wg_sdlsurface.h>
#include <wondergui.h>


namespace sdl_wglib
{
	static int				g_ticks = 0;

	//____ MapKeys() ___________________________________________________________

	void MapKeys()
	{
		Base::mapKey( Key::Shift, SDLK_LSHIFT );
		Base::mapKey( Key::Shift, SDLK_RSHIFT );
		Base::mapKey( Key::Control, SDLK_LCTRL );
		Base::mapKey( Key::Control, SDLK_RCTRL );
		Base::mapKey( Key::Alt, SDLK_LALT );
		Base::mapKey( Key::Alt, SDLK_RALT );

		Base::mapKey( Key::Left, SDLK_LEFT );
		Base::mapKey( Key::Right, SDLK_RIGHT );
		Base::mapKey( Key::Up, SDLK_UP );
		Base::mapKey( Key::Down, SDLK_DOWN );

		Base::mapKey( Key::Home, SDLK_HOME );
		Base::mapKey( Key::End, SDLK_END );
		Base::mapKey( Key::PageUp, SDLK_PAGEUP );
		Base::mapKey( Key::PageDown, SDLK_PAGEDOWN );

		Base::mapKey( Key::Return, SDLK_RETURN );
		Base::mapKey( Key::Backspace, SDLK_BACKSPACE );
		Base::mapKey( Key::Delete, SDLK_DELETE );
		Base::mapKey( Key::Tab, SDLK_TAB );
		Base::mapKey( Key::Escape, SDLK_ESCAPE );
	}

	//____ BeginEvents() _______________________________________________________

	void BeginEvents( const WgEventHandlerPtr& pHandler )
	{
		// Add a tick event first as the first.

		int ticks = SDL_GetTicks();
		pHandler->QueueEvent( WgTickEvent::create( ticks - g_ticks ) );
		g_ticks = ticks;
	}

	//____ TranslateEvent() ____________________________________________________

	void TranslateEvent(  const WgEventHandlerPtr& pHandler, SDL_Event& event )
	{
		switch (event.type)
		{
			// check for keypresses
			case SDL_KEYDOWN:
			{
				pHandler->QueueEvent( WgKeyPressEvent::create( event.key.keysym.sym ) );
				if( event.key.keysym.unicode != 0 )
					pHandler->QueueEvent( CharacterEvent::create( event.key.keysym.unicode ) );
				break;
			}

			case SDL_KEYUP:
			{
				pHandler->QueueEvent( WgKeyReleaseEvent::create( event.key.keysym.sym ) );
				break;
			}

			case	SDL_MOUSEMOTION:
			{
				pHandler->QueueEvent( WgMouseMoveEvent::create( CoordI( event.motion.x, event.motion.y ) ) );
				break;
			}

			case	SDL_MOUSEBUTTONDOWN:
				if(event.button.button == 4 )
					pHandler->QueueEvent( WgWheelRollEvent::create( 1, 1 ) );
				else if(event.button.button == 5)
					pHandler->QueueEvent( WgWheelRollEvent::create( 1, -1 ) );
				else
				{
					pHandler->QueueEvent( WgMousePressEvent::create( (MouseButton) event.button.button ) );
				}
				break;

			case	SDL_MOUSEBUTTONUP:
				if( event.button.button != 4 && event.button.button != 5 )
					pHandler->QueueEvent( WgMouseReleaseEvent::create( (MouseButton) event.button.button ) );
				break;
		}
	}

	//_____ EndEvents() ________________________________________________________

	void EndEvents( const WgEventHandlerPtr& pHandler )
	{
		pHandler->ProcessEvents();
	}

	//____ loadSurface() __________________________________________________________

	Surface_p loadSurface( const char * path, const SurfaceFactory_p& pFactory )
	{
		SDL_Surface* bmp = IMG_Load(path);
		if (!bmp)
		{
			printf("Unable to load bitmap: %s\n", IMG_GetError());
			return 0;
		}

		WgSDLSurfacePtr	wrapper = WgSDLSurface::create( bmp );

		Surface_p pSurf = pFactory->createSurface( wrapper->size(), wrapper->isOpaque()? PixelType::RGB_8 : WG_PIXEL_ARGB_8 );

		if( !pSurf )
		{
			printf("Unable to create surface for loaded bitmap: %s\n", path);
			return 0;
		}

		if( !pSurf->copyFrom( wrapper, CoordI(0,0) ) )
		{
			printf("Unable to copy loaded bitmap '%s' to surface.\n", path);
			return 0;
		};

		return pSurf;
	}

	//____ LoadStdWidgets() _____________________________________________________

	ResDB_p LoadStdWidgets( const char * pImagePath, const SurfaceFactory_p& pFactory )
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
		const int LISTENTRY_OFS			= COMBOBOX_OFS + 35;
		const int SORT_ARROW_OFS		= LISTENTRY_OFS + 9;
		const int SMILEY_OFS			= SORT_ARROW_OFS + 8;
		const int TILES_OFS				= 256+192;

		Surface_p pSurface = loadSurface( pImagePath, pFactory );
		if( !pSurface )
			return 0;

		BlockSkin_p	pHScrollbarBwdButtonSkin = BlockSkin::createClickable( pSurface, SizeI(17,17), CoordI(1,HSLIDER_BTN_OFS), SizeI(19,0), BorderI(3) );
		pHScrollbarBwdButtonSkin->setContentPadding(BorderI(4));

		BlockSkin_p	pHScrollbarFwdButtonSkin = BlockSkin::createClickable( pSurface, SizeI(17,17), CoordI(77,HSLIDER_BTN_OFS), SizeI(19,0), BorderI(3) );
		pHScrollbarFwdButtonSkin->setContentPadding(BorderI(4));

		BlockSkin_p	pVScrollbarBwdButtonSkin = BlockSkin::createClickable( pSurface, SizeI(17,17), CoordI(1,VSLIDER_BTN_OFS), SizeI(19,0), BorderI(3) );
		pVScrollbarBwdButtonSkin->setContentPadding(BorderI(4));

		BlockSkin_p	pVScrollbarFwdButtonSkin = BlockSkin::createClickable( pSurface, SizeI(17,17), CoordI(77,VSLIDER_BTN_OFS), SizeI(19,0), BorderI(3) );
		pVScrollbarFwdButtonSkin->setContentPadding(BorderI(4));

		BlockSkin_p	pScrollbarHandleSkin = BlockSkin::createClickable( pSurface, SizeI(8,8), CoordI(1,SLIDER_OFS), SizeI(10,0), BorderI(2) );
		pScrollbarHandleSkin->setContentPadding(BorderI(3));

		BlockSkin_p	pScrollbarBackSkin = BlockSkin::createStatic( pSurface, RectI(1,SLIDER_BACK_OFS,5,5), BorderI(2) );
		pScrollbarBackSkin->setContentPadding(BorderI(2));

//		WgBlocksetPtr pResizeButtonBlocks = WgBlockset::CreateFromRow(pSurface, RectI(1,RESIZE_BUTTON_OFS,86,20), 4, 2, WG_OPAQUE);
//		pResizeButtonBlocks->setFrame(BorderI(3));

		BlockSkin_p	pCheckboxIconSkin = BlockSkin::createClickSelectableWidget( pSurface, SizeI(11,11), CoordI(1,CHECKBOX_OFS), SizeI(13,0), BorderI(2) );
		pCheckboxIconSkin->setContentPadding(BorderI(3));

		BlockSkin_p	pRadiobuttonIconSkin = BlockSkin::createClickSelectableWidget( pSurface, SizeI(11,11), CoordI(1,RADIOBUTTON_OFS), SizeI(13,0) );
		pRadiobuttonIconSkin->setContentPadding(BorderI(3));

		BlockSkin_p pButtonSkin = BlockSkin::createClickable( pSurface, SizeI(8,8), CoordI(1,BUTTON_OFS), SizeI(10,0), BorderI(3) );
		pButtonSkin->setContentPadding(BorderI(4));
		pButtonSkin->setContentShift( StateEnum::Pressed, CoordI(2,2) );

		BlockSkin_p pHSplitSkin = BlockSkin::createStatic(pSurface, RectI(1,SPLITS_AND_FRAME_OFS,8,2), BorderI(0) );

		BlockSkin_p pVSplitBlocks = BlockSkin::createStatic(pSurface, RectI(11,SPLITS_AND_FRAME_OFS,2,8), BorderI(0) );

		BlockSkin_p pFrameSkin = BlockSkin::createClickable( pSurface, SizeI(8,8), CoordI(1, SPLITS_AND_FRAME_OFS), SizeI(10,0), BorderI(2) );
		pFrameSkin->setContentPadding(BorderI(3));

//		WgBlocksetPtr pComboboxBlocks = WgBlockset::CreateFromRow(pSurface, RectI(1,COMBOBOX_OFS,98,20), 4, 2, WG_OPAQUE);
//		pComboboxBlocks->setFrame(BorderI(1,1,20,1));
//		pComboboxBlocks->setPadding(BorderI(2,2,21,2));

		BlockSkin_p pListEntrySkin = BlockSkin::createClickSelectable( pSurface, SizeI(8,6), CoordI(1,LISTENTRY_OFS), SizeI(10,0), BorderI(1) );
		pListEntrySkin->setContentPadding(BorderI(4));

		BlockSkin_p pSortArrowSkin = BlockSkin::createClickSelectable( pSurface, SizeI(11,6), CoordI(1,SORT_ARROW_OFS), SizeI(13,0), BorderI(0) );

		BlockSkin_p pSmileyIconSkin = BlockSkin::createClickSelectable( pSurface, SizeI(12,12), CoordI(1, SMILEY_OFS), SizeI(14,0), BorderI(0) );


		BlockSkin_p pBgCheckeredGreySkin = BlockSkin::createStatic( pSurface, RectI(0,TILES_OFS,64,64) );
		pBgCheckeredGreySkin->setTiledCenter(true);
		pBgCheckeredGreySkin->optimizeRenderMethods();

		BlockSkin_p pBgBlueGradientSkin = BlockSkin::createStatic( pSurface, RectI(1*64,TILES_OFS,64,64) );
		pBgBlueGradientSkin->optimizeRenderMethods();


//		ColorsetPtr pSelectionColors = Colorset::create( Color(0x0), Color(0x40FFFFFF), Color(0x80FFFFFF), Color(0x40000000), Color(0x0) );

		BlockSkin_p pPlateSkin = BlockSkin::createClickable( pSurface, SizeI(8,8), CoordI(1,PLATE_OFS), SizeI(10,0), BorderI(3) );
		pPlateSkin->setContentPadding(BorderI(4));
		pPlateSkin->optimizeRenderMethods();

		ResDB_p pDB = ResDB::create();

		pDB->addSkin( "listentry", pListEntrySkin );
		pDB->addSkin( "plate", pPlateSkin );
		pDB->addSkin( "sortarrow", pSortArrowSkin );
		pDB->addSkin( "smiley", pSmileyIconSkin );

		// Create standard button

		Button_p pButton = Button::create();
		pButton->setSkin( pButtonSkin );
		pDB->addWidget( "button", pButton );

		// Create standard plate

		Image_p pPlate = Image::create();
		pPlate->setSkin( pPlateSkin );
		pDB->addWidget( "plate", pPlate );

		// Create standard checkbox

		CheckBoxPtr pCheckbox = CheckBox::create();
		pCheckbox->Icon()->set( pCheckboxIconSkin );
		pDB->addWidget( "checkbox", pCheckbox );

		// Create standard radiobutton

		RadioButtonPtr pRadiobutton = RadioButton::create();
		pRadiobutton->Icon()->set( pRadiobuttonIconSkin );
		pDB->addWidget( "radiobutton", pRadiobutton );

		// Create standard horizontal scrollbar

		Scrollbar_p pHScrollbar = Scrollbar::create();
		pHScrollbar->setSkins( pScrollbarBackSkin, pScrollbarHandleSkin, pHScrollbarBwdButtonSkin, pHScrollbarFwdButtonSkin );
		pDB->addWidget( "hscrollbar", pHScrollbar );

		// Create standard vertical scrollbar

		Scrollbar_p pVScrollbar = Scrollbar::create();
		pVScrollbar->setSkins( pScrollbarBackSkin, pScrollbarHandleSkin, pVScrollbarBwdButtonSkin, pVScrollbarFwdButtonSkin );
		pDB->addWidget( "vscrollbar", pVScrollbar );

		// Create standard menubar
/*
		Menubar * pMenubar = new Menubar();
		pMenubar->SetBgSource( pPlateBlocks );
		pDB->addWidget( "menubar", pMenubar );
*/
		// Create Background bitmaps

		Image_p pBgCheckeredGrey = Image::create();
		pBgCheckeredGrey->setSkin( pBgCheckeredGreySkin );
		pDB->addWidget( "bg_checkered_grey", pBgCheckeredGrey );

		Image_p pBgBlueGradient = Image::create();
		pBgBlueGradient->setSkin( pBgBlueGradientSkin );
		pDB->addWidget( "bg_blue_gradient", pBgBlueGradient );

		// Create standard menu

		Menu_p pMenu = Menu::create();
		pMenu->setSkin( pPlateSkin, 16, 16 );
		pMenu->setSeparatorSkin( pHSplitSkin, BorderI(1) );
		pMenu->setCheckBoxSkin( pCheckboxIconSkin );
		pMenu->setRadioButtonSkin( pRadiobuttonIconSkin );
		pMenu->setScrollbarSkins( pScrollbarBackSkin, pScrollbarHandleSkin, pVScrollbarBwdButtonSkin, pVScrollbarFwdButtonSkin );
		pMenu->setEntrySkin( pPlateSkin );
		pDB->addWidget( "menu", pMenu );

		// Create standard combobox
/*
		Combobox * pCombobox = new Combobox();
		pCombobox->SetSource( pComboboxBlocks );
		pDB->addWidget( "combobox", pCombobox );
*/
		// Create standard view

		{
			ScrollPanel_p pView = ScrollPanel::create();

			Widget_p pHScrollbar = pDB->cloneWidget( "hscrollbar" );
			Widget_p pVScrollbar = pDB->cloneWidget( "vscrollbar" );

			if( pHScrollbar )
			{
				Scrollbar_p pH = Scrollbar::cast(pHScrollbar);
				pView->setHorizontalScrollbar( pH );
			}
			if( pVScrollbar )
			{
				Scrollbar_p pV = Scrollbar::cast(pVScrollbar);
				pView->setVerticalScrollbar( pV );
			}
			pView->setSkin( pBgCheckeredGreySkin );
			pView->setCornerSkin( pPlateSkin );
			pDB->addWidget( "view", pView );
		}

		return pDB;
	}

	//____ LoadBitmapFont() ____________________________________________________

	Font_p LoadBitmapFont( const char * pImgPath, const char * pSpecPath, const SurfaceFactory_p& factory )
	{
		//TODO: This leaks memory until we have ref-counted

		Surface_p pFontImg = sdl_wglib::loadSurface( pImgPath, factory );

		char * pFontSpec = (char*) LoadFile( pSpecPath );

		BitmapGlyphs_p pGlyphs = BitmapGlyphs::create( pFontImg, pFontSpec );

		Font_p pFont = Font::create();
		pFont->setBitmapGlyphs( pGlyphs, WG_STYLE_NORMAL, 0 );

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
		int size = ftell(fp);
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


};
