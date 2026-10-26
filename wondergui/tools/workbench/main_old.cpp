#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#include <wondergui.h>
#include <wg_bitmapglyphs.h>
#include <wg_vectorglyphs.h>
#include <wg_knob.h>
#include <iostream>

#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wg_softgfxdevice.h>
#include <sdl_wglib.h>

extern std::ostream cout;

SDL_Surface *	initSDL( int w, int h );
bool			eventLoop( const WgEventHandlerPtr& pHandler );
RootPanel_p	setupGUI( const GfxDevice_p& pDevice );
void			printWidgetSizes();

void cbInitDrag( const WgEventPtr& _pEvent, const Object_p& pWidget );
void cbDragWidget( const WgEventPtr& _pEvent, const Object_p& pWidget );

void cbOpenModal( const WgEventPtr& _pEvent, const Object_p& pWidget );
void cbCloseModal( const WgEventPtr& _pEvent, const Object_p& pWidget );

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const WgEventHandlerPtr& pEventHandler );
void cbResize( const WgEventPtr& _pEvent, const Object_p& pWidget );
void cbIncreaseEntryPadding( const WgEventPtr& _pEvent, const Object_p& pWidget );
void cbAddEntryText( const WgEventPtr& _pEvent, const Object_p& pWidget );

ModalLayer * g_pModal = 0;

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	// Dump some info
	
	printWidgetSizes();
	
	
	// Init SDL

	SDL_Surface * pScreen = initSDL(640,480);
	if(!pScreen )
		return 1;

	IMG_Init( IMG_INIT_PNG | IMG_INIT_JPG );

	// Init WonderGUI

	Base::init(nullptr);
	sdl_wglib::MapKeys();

//	Base::initFreeType();

	// Setup gfxdevice and gui

	SoftSurface_p pCanvas = SoftSurface::create( SizeI(640,480), WG_PIXEL_ARGB_8, (unsigned char *) pScreen->pixels, pScreen->pitch, 0 );
	SoftGfxDevice_p pGfxDevice = SoftGfxDevice::create( pCanvas );
	pGfxDevice->setBilinearFiltering( true );

	// Load TTF-font
/*
	VectorGlyphs::setSurfaceFactory( new WgSDLSurfaceFactory() );

	char	ttfname[] = { "a.ttf" };

	int size = fileSize( ttfname );
	char * pFontFile = (char*) loadFile( ttfname );
	VectorGlyphs_p pGlyphs = VectorGlyphs::create( pFontFile , size, 0 );

	Font_p pFont = Font::create();
	pFont->setDefaultVectorGlyphs( pGlyphs );
*/
	// Load bitmap font

	Font_p pFont = sdl_wglib::LoadBitmapFont( "../resources/anuvverbubbla_8x8.png", "../resources/anuvverbubbla_8x8.fnt", SoftSurfaceFactory::create() );

	// Load and setup cursor

	Surface_p pCursorImg = sdl_wglib::loadSurface("../resources/cursors.png", SoftSurfaceFactory::create() );

	GfxAnim_p pCursorEOL = GfxAnim::create();
	pCursorEOL->setSize( SizeI(8,8) );
	pCursorEOL->addFrames(pCursorImg, CoordI(0,0), SizeI(2,1), 200 );
	pCursorEOL->setPlayMode( WG_FORWARD_LOOPING );

	GfxAnim_p pCursorINS = GfxAnim::create();
	pCursorINS->setSize( SizeI(8,8) );
	pCursorINS->addFrames( pCursorImg, CoordI(0,8), SizeI(2,1), 200 );
	pCursorINS->setPlayMode( WG_FORWARD_LOOPING );

	WgCursorPtr pCursor = WgCursor::create();
	pCursor->setAnim(WgCursor::EOL, pCursorEOL);
	pCursor->setAnim(WgCursor::INS, pCursorINS);
	pCursor->setAnim(WgCursor::OVR, pCursorEOL);
	pCursor->setBearing(WgCursor::EOL, CoordI(0,-8));
	pCursor->setBearing(WgCursor::INS, CoordI(0,-8));
	pCursor->setBearing(WgCursor::OVR, CoordI(0,-8));

	Base::setDefaultCursor( pCursor );

	// Set default textprop

	Textprop prop;

	prop.setFont(pFont);
	prop.setColor( Color::white );
	prop.setSize(8);

	Base::setDefaultTextprop( prop.register() );


	RootPanel_p pRoot = setupGUI( pGfxDevice );

	pRoot->findWidget( CoordI(10,10), WG_SEARCH_ACTION_TARGET );

   // program main loop

    while (eventLoop( pRoot->EventHandler() ))
    {

		// GET DIRTY RECTS

		int nDirtyRects;
		SDL_Rect	dirtyRects[100];

		if( pRoot->nbDirtyRects() <= 100 )
		{
			nDirtyRects = pRoot->nbDirtyRects();
			for( int i = 0 ; i < nDirtyRects ; i++ )
			{
				const RectI * pR = pRoot->firstDirtyRect() + i;

				dirtyRects[i].x = pR->x;
				dirtyRects[i].y = pR->y;
				dirtyRects[i].w = pR->w;
				dirtyRects[i].h = pR->h;
			}
		}
		else
		{
			nDirtyRects = 1;

			const RectI r = pRoot->geo();

			dirtyRects[0].x = r.x;
			dirtyRects[0].y = r.y;
			dirtyRects[0].w = r.w;
			dirtyRects[0].h = r.h;
		}


        // DRAWING STARTS HERE

		SDL_LockSurface( pScreen );
		pRoot->render();
		SDL_UnlockSurface( pScreen );

        // DRAWING ENDS HERE

        // finally, update the screen :)

		SDL_UpdateRects( pScreen, nDirtyRects, dirtyRects);


        // Pause for a while

        SDL_Delay(10);

    } // end main loop


	// Exit WonderGUI

	g_pModal = 0;
	pRoot = 0;
	pGfxDevice = 0;

	Base::exit();

	IMG_Quit();

    // all is well ;)
    printf("Exited cleanly\n");
    return 0;
}

//____ printWidgetSizes() _______________________________________________________

void printWidgetSizes()
{
	printf( "AnimPlayer: %d bytes\n", (int) sizeof(AnimPlayer) );
	printf( "Button: %d bytes\n", (int) sizeof(Button) );
	printf( "CheckBox: %d bytes\n", (int) sizeof(CheckBox) );
	printf( "LineEditor: %d bytes\n", (int) sizeof(LineEditor) );
	printf( "ValueEditor: %d bytes\n", (int) sizeof(ValueEditor) );
	printf( "Filler: %d bytes\n", (int) sizeof(Filler) );
	printf( "FlexPanel: %d bytes\n", (int) sizeof(FlexPanel) );
	printf( "FpsDisplay: %d bytes\n", (int) sizeof(FpsDisplay) );
	printf( "Menu: %d bytes\n", (int) sizeof(Menu) );
	printf( "PopupLayer: %d bytes\n", (int) sizeof(PopupLayer) );
	printf( "ModalLayer: %d bytes\n", (int) sizeof(ModalLayer) );
	printf( "Image: %d bytes\n", (int) sizeof(Image) );
	printf( "RadioButton: %d bytes\n", (int) sizeof(RadioButton) );
	printf( "RefreshButton: %d bytes\n", (int) sizeof(RefreshButton) );
	printf( "ShaderCapsule: %d bytes\n", (int) sizeof(ShaderCapsule) );
	printf( "Scrollbar: %d bytes\n", (int) sizeof(Scrollbar) );
	printf( "PackPanel: %d bytes\n", (int) sizeof(PackPanel) );
	printf( "StackPanel: %d bytes\n", (int) sizeof(StackPanel) );
//	printf( "WgTablePanel: %d bytes\n", (int) sizeof(WgTablePanel) );
	printf( "SizeCapsule: %d bytes\n", (int) sizeof(SizeCapsule) );
	printf( "TextDisplay: %d bytes\n", (int) sizeof(TextDisplay) );
	printf( "ValueDisplay: %d bytes\n", (int) sizeof(ValueDisplay) );
//	printf( "ListPanel: %d bytes\n", (int) sizeof(ListPanel) );
	printf( "ScrollPanel: %d bytes\n", (int) sizeof(ScrollPanel) );
	printf( "\n" );
	printf( "FlexHook: %d bytes\n", (int) sizeof(FlexHook) );
	printf( "PopupHook: %d bytes\n", (int) sizeof(PopupHook) );
	printf( "ModalHook: %d bytes\n", (int) sizeof(ModalHook) );
	printf( "PackHook: %d bytes\n", (int) sizeof(PackHook) );
	printf( "StackHook: %d bytes\n", (int) sizeof(StackHook) );
//	printf( "WgTableHook: %d bytes\n", (int) sizeof(WgTableHook) );
//	printf( "ListHook: %d bytes\n", (int) sizeof(ListHook) );
	printf( "ScrollHook: %d bytes\n", (int) sizeof(ScrollHook) );
}

int sortWidgetId( const Widget * p1, const Widget * p2 )
{
	return p1->id() - p2->id();
}


//____ setupGUI() ______________________________________________________________

RootPanel_p setupGUI( const GfxDevice_p& pDevice )
{
	ResDB_p pDB = sdl_wglib::LoadStdWidgets( "../resources/blocks.png", SoftSurfaceFactory::create() );
	if( !pDB )
		return 0;

	RootPanel_p pRoot = RootPanel::create( pDevice );

	WgEventHandlerPtr pEventHandler = pRoot->EventHandler();

	WgEventLogger * pEventLogger = new WgEventLogger( std::cout );
	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_POSITION );
	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_REPEAT );
	pEventLogger->IgnoreEvent( WG_EVENT_MOUSE_PRESS );
	pEventLogger->IgnoreAllEvents();
	pEventLogger->LogMouseButtonEvents();
	pEventHandler->AddCallback( pEventLogger );


	// Load images

	SoftSurfaceFactory_p pFactory = SoftSurfaceFactory::create();

	Surface_p pBackImg = sdl_wglib::loadSurface("../resources/What-Goes-Up-3.bmp", pFactory );

	Surface_p pFlagImg = sdl_wglib::loadSurface("cb2.bmp", pFactory );

	Surface_p pSplashImg = sdl_wglib::loadSurface("../resources/splash.png", pFactory );

	Surface_p pBigImg = sdl_wglib::loadSurface("../resources/frog.jpg", pFactory );

	// MenuPanel

	PopupLayer_p pPopupLayer = PopupLayer::create();
	pRoot->setWidget( pPopupLayer );

	// Bottom Flex

	FlexPanel_p pBottom = FlexPanel::create();
	pPopupLayer->setBaseWidget( pBottom );
	pBottom->setSkin( ColorSkin::create( Color::black ) );

	// Main Flex

	FlexPanel_p pFlex = FlexPanel::create();
	pBottom->addWidget( pFlex, WAX_NORTHWEST, CoordI(), WAX_SOUTHEAST, CoordI(), Borders(10) );

	// Background

//	Image * pBackground = new Image();
//	pBackground->SetSource( pBackBlock );
//	FlexHook * pHook = pFlex->addWidget( pBackground );
//	pHook->SetAnchored( WAX_NORTHWEST, WAX_SOUTHEAST );

	BlockSkin_p pBgSkin = BlockSkin::create();
	pBgSkin->setSurface(pBackImg);
	pBgSkin->setBlockGeo( pBackImg->size() );
	pBgSkin->setStateBlock( WG_STATE_NORMAL, CoordI(0,0) );
	pBgSkin->setTiledCenter(true);
	pFlex->setSkin( pBgSkin );


	//
/*
	{
		ShaderCapsule * pShader = new ShaderCapsule();
		pShader->setBlendMode(WAX_BLENDMODE_MULTIPLY);
		pShader->setColor( Color(0xFF0000FF) );

		StackPanel * pStack = new StackPanel();
		pShader->setWidget( pStack );

		Image * pBg = (Image*) pDB->cloneWidget( "plate" );
		pStack->addWidget( pBg );

		ShaderCapsule * pShader2 = new ShaderCapsule();
		pShader2->setBlendMode(WAX_BLENDMODE_ADD);
		pShader2->setColor( Color(0xFFFFFFFF) );
		StackHook * pHook = pStack->addWidget( pShader2 );
		pHook->setSizePolicy( StackHook::SCALE );
		pHook->setOrigo( WAX_CENTER );
		pHook->SetBorders( Borders(2) );


		Image * pSplash= new Image();
		pSplash->SetSource( pSplashBlock );
		pShader2->setWidget( pSplash );

		addResizablePanel( pFlex, pShader, pEventHandler );
	}
*/

/*
	{
		StackPanel * pStack = new StackPanel();

		Image * pBg = (Image*) pDB->cloneWidget( "plate" );
		pStack->addWidget( pBg );

		ListPanel * pVBox = new ListPanel();
		StackHook * pHook = pStack->addWidget( pVBox );
		pHook->setPadding( Borders(4) );

		for( int i = 0 ; i < 10 ; i++ )
		{
			ValueDisplay * pV = new ValueDisplay();
			pV->setValue( 1000 + i );
			pVBox->addWidget( pV );
		}
		

//		ValueEditor * pValue = new ValueEditor();
//		pVBox->addWidget( pValue );

//		Scrollbar * pScrollbar = (Scrollbar*) pDB->cloneWidget( "hscrollbar" );
//		pScrollbar->setScrollbarTarget(pValue);
//		pVBox->addWidget( pScrollbar );

		addResizablePanel( pFlex, pStack, pEventHandler );

//		pValue->setValue( 100 );
//		pValue->grabFocus();
	}
*/
/*
	{
		Knob * pKnob = new Knob();
		addResizablePanel( pFlex, pKnob, pEventHandler );
	}
*/

	// Test PackList

	{
		PackList_p pList = PackList::create();
		Skin_p pPlate = pDB->getSkin("plate");

		pList->setSkin( pPlate );
		pList->setEntrySkin( pDB->getSkin("listentry"));
		pList->setLassoSkin( BoxSkin::create( Color(0,0,255,64), Borders(1), Color(0,0,255,128)) );
		pList->setSelectMode( WG_SELECT_MULTI );
//		pList->setOrientation( WG_HORIZONTAL );

		pList->header.setSkin( pDB->getSkin("plate") );
		pList->header.label.set( "LABEL" );
		pList->header.arrow.set( pDB->getSkin("sortarrow"), WAX_EAST, Borders(0,0,0,2) );
		pList->header.icon.set( pDB->getSkin("smiley"), WAX_WEST, Borders(0,2,0,0) );

		pList->setSortFunction( sortWidgetId );

		for( int i = 0 ; i < 20 ; i++ )
		{
			int id = rand() % 100;
/*			PackPanel_p pEntry = PackPanel::create();
			pEntry->setId(id);
*/


			TextDisplay_p pLabel = TextDisplay::create();
			char str[40];
			sprintf( str, "ENTRY %d", id );
			pLabel->text()->set( str );
			pLabel->text()->setAlignment(WAX_CENTER);
			pLabel->text()->setWrap(true);
			pLabel->setMarkOpacity(0);
			pLabel->setId(id);
/*			pEntry->addWidget(pLabel);
			
			Button_p pButton = Button::cast(pDB->cloneWidget("button"));
			pButton->Label()->set( "PUSH" );
			pEntry->addWidget(pButton);
*/
//			pList->addWidget( pEntry );
			pList->insertWidgetSorted( pLabel );
		}


		ScrollPanel_p pWindow = ScrollPanel::cast(pDB->getWidget("view"));
		pWindow->setContent( pList );
//		pWindow->setContentSizePolicy( SizePolicy::WG_DEFAULT, SizePolicy::WG_BOUND );
		pWindow->setContentSizePolicy( SizePolicy::WG_BOUND, SizePolicy::WG_DEFAULT );

//		pFlex->addWidget(pWindow, RectI( 50,50,200,200) );
		pFlex->addWidget(pWindow, CoordI(50,50) );

		PackPanel_p pButtonPanel = PackPanel::create();
		pButtonPanel->setOrientation( WG_VERTICAL );
		pFlex->addWidget(pButtonPanel, CoordI(300,50) );


		Button_p pButton = Button::cast(pDB->cloneWidget("button"));
		pButton->Label()->set("ADD ENTRY PADDING");
		pButtonPanel->addWidget(pButton );
		pRoot->EventHandler()->AddCallback( WgEventFilter::select(), pButton, cbIncreaseEntryPadding, pList );

		Button_p pButton2 = Button::cast(pDB->cloneWidget("button"));
		pButton2->Label()->set("ADD ENTRY");
		pButtonPanel->addWidget(pButton2 );
		pRoot->EventHandler()->AddCallback( WgEventFilter::select(), pButton2, cbAddEntryText, pList->firstWidget() );

	}
	
	// Test menus and MenuPanel.
/*
	{
		Menu_p pSubMenu1_1 = Menu::cast(pDB->cloneWidget( "menu" ));
		pSubMenu1_1->addItem( new MenuEntry( 1, String("ENTRY 11"), String("Help text for entry 11"), Skin_p(), 0 ));
		pSubMenu1_1->addItem( new MenuEntry( 2, String("ENTRY 12"), String("Help text for entry 12"), Skin_p(), 0 ));
		
		Menu_p pSubMenu1 = Menu::cast(pDB->cloneWidget( "menu" ));
		pSubMenu1->addItem( new MenuEntry( 3, String("ENTRY 7"), String("Help text for entry 7"), Skin_p(), 0 ));
		pSubMenu1->addItem( new MenuEntry( 4, String("ENTRY 8"), String("Help text for entry 8"), Skin_p(), 0 ));
		pSubMenu1->addItem( new MenuSeparator() );
		pSubMenu1->addItem( new MenuSubMenu( 5, String("SUBMENU 1_1"), String("Help text for submenu 1_1"), Skin_p(), 0, pSubMenu1_1.GetRealPtr() ) );

		Menu_p pSubMenu2 = Menu::cast(pDB->cloneWidget( "menu" ));
		pSubMenu2->addItem( new MenuEntry( 6, String("ENTRY 9"), String("Help text for entry 9"), Skin_p(), 0 ));
		pSubMenu2->addItem( new MenuSeparator() );
		pSubMenu2->addItem( new MenuEntry( 7, String("ENTRY 10"), String("Help text for entry 10"), Skin_p(), 0 ));

		Menu_p pMenu = Menu::cast(pDB->cloneWidget( "menu" ));
		pMenu->addItem( new MenuEntry( 8, String("ENTRY 1"), String("Help text for entry 1"), Skin_p(), 0 ));
		pMenu->addItem( new MenuSeparator() );
		pMenu->addItem( new MenuEntry( 9, String("ENTRY 2"), String("Help text for entry 2"), Skin_p(), 0 ));
		pMenu->addItem( new MenuEntry( 10, String("ENTRY 3"), String("Help text for entry 3"), Skin_p(), 0 ));
		pMenu->addItem( new MenuEntry( 11, String("ENTRY 4"), String("Help text for entry 4"), Skin_p(), 0 ));
		pMenu->addItem( new MenuEntry( 12, String("ENTRY 5"), String("Help text for entry 5"), Skin_p(), 0 ));
		pMenu->addItem( new MenuEntry( 13, String("ENTRY 6"), String("Help text for entry 6"), Skin_p(), 0 ));

		pMenu->addItem( new MenuSubMenu( 14, String("SUBMENU 1"), String("Help text for submenu 1"), Skin_p(), 0, pSubMenu1.GetRealPtr() ) );
		pMenu->addItem( new MenuSubMenu( 15, String("SUBMENU 2"), String("Help text for submenu 2"), Skin_p(), 0, pSubMenu2.GetRealPtr() ) );

		pFlex->addWidget(pMenu);

//		addResizablePanel( pFlex, pMenu, pEventHandler );
		pMenu->grabFocus();

//		pMenuPanel->OpenMenu( pMenu, RectI(10,10,100,10), WAX_SOUTHWEST );

	}
*/
	// Test menubar and menus
/*
	{
		Menubar * pMenubar = WgCast<Menubar>(pDB->cloneWidget("menubar"));

		Menu * pFileMenu = (Menu*) pDB->cloneWidget( "menu" );
		pFileMenu->addItem( new MenuEntry( String("Open..."), String("Open a file"), WgBlocksetPtr(), 0 ));
		pFileMenu->addItem( new MenuEntry( String("Save"), String("Save the file"), WgBlocksetPtr(), 0 ));
		pFileMenu->addItem( new MenuSeparator() );
		pFileMenu->addItem( new MenuEntry( String("Quit"), String("Quit this program"), WgBlocksetPtr(), 0 ));
		pMenubar->addMenu( "FILE", pFileMenu );
	
		Menu * pHelpMenu = (Menu*) pDB->cloneWidget( "menu" );
		pHelpMenu->addItem( new MenuEntry( String("Help"), String("Get some help"), WgBlocksetPtr(), 0 ));
		pHelpMenu->addItem( new MenuSeparator() );
		pHelpMenu->addItem( new MenuEntry( String("About"), String("About this program"), WgBlocksetPtr(), 0 ));
		pMenubar->addMenu( "HELP", pHelpMenu );

		addResizablePanel( pFlex, pMenubar, pEventHandler );
	}
*/
/*
	// Test oscilloscope

	{
		Surface * pImg = sdl_wglib::loadSurface("../resources/blocks.png", SoftSurfaceFactory() );

		WgBlocksetPtr pMarkerBlock = WgBlockset::CreateFromRect( pImg, RectI(1,120,8,8) );

		Oscilloscope * pOsc = new Oscilloscope();

		pOsc->SetBackground( ColorSkin::create( Color(0,0,96) ) );

		float grid[] = {-1.f,-0.5f,0.f,0.5f,1.f};
		pOsc->setHGridLines( 5, grid );
		pOsc->setVGridLines( 5, grid );
		pOsc->setGridColor( Color(64,64,64) );

		pOsc->SetMarkerGfx( pMarkerBlock );
		pOsc->addMarker( 30, 0.f );


		float points[256];

		for( int i = 0 ; i < 256 ; i++ )
		{
			points[i] = (float) sin(i/25.f)*0.90f;
		}

		pOsc->setLinePoints(256,points);


		addResizablePanel( pFlex, pOsc, pEventHandler );
	}
*/
	// Test combobox
/*
	{
		Combobox * pCombobox = WgCast<Combobox>(pDB->cloneWidget("combobox"));

		Menu * pComboMenu = (Menu*) pDB->cloneWidget( "menu" );
		pComboMenu->addItem( new MenuEntry( 1, String("Alfa"), String("First"), WgBlocksetPtr(), 0 ) );
		pComboMenu->addItem( new MenuEntry( 2, String("Beta"), String("Second"), WgBlocksetPtr(), 0 ) );
		pCombobox->setMenu( pComboMenu );

		addResizablePanel( pFlex, pCombobox, pEventHandler );

	}
*/

	// Test view
/*
	{
		ScrollPanel * pView = (ScrollPanel*) pDB->cloneWidget( "view" );

		Image * pImage = new Image();
		pImage->SetSource( pBigBlock );
		pView->setContent( pImage );
		pView->setOverlayScrollbars(true);
		((PanelHook*)pView->HScrollbar()->hook())->setPadding( 10 );
		((PanelHook*)pView->VScrollbar()->hook())->setPadding( 10 );

		((PanelHook*)pView->content()->hook())->setPadding( 50 );

		addResizablePanel( pFlex, pView, pEventHandler );
	}
*/

	// Test overlapping non-visible
/*
	{
		Filler * pCovered = new Filler();
		pCovered->setColors( Colorset::create( Color::aliceblue ) );
		pFlex->addWidget( pCovered, RectI( 10,10,20,20 ) );

		g_pCovered = pCovered;

		Filler * pCovering = new Filler();
		pCovering->setColors( Colorset::create( Color::wheat ) );
		FlexHook * pHook = pFlex->addWidget( pCovering, RectI( 0,0,50,50 ) );
		pHook->setVisible(false);
	}
*/

	// Modal panel
/*
	g_pModal = new ModalLayer();
	g_pModal->SetBase( pFlex );

	pRoot->setWidget(g_pModal);


	// Modal button

	Button * pModalButton = (Button*) pDB->cloneWidget( "button" );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonClick(pModalButton, 1), cbCloseModal, pModalButton );

	//

	Button * pButton = (Button*) pDB->cloneWidget( "button" );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pButton, 1), cbOpenModal, pModalButton );

	FlexHook * pHook = pFlex->addWidget( pButton, RectI(0,0,100,100), WAX_NORTHWEST );
*/
	//
/*
	Image * pFlag1= new Image();
	pFlag1->SetSource( pSplashBlock );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonPress(pFlag1, 1), cbInitDrag, pFlag1 );

	FlexHook * pHook = pFlex->addWidget( pFlag1, CoordI(0,0), WAX_CENTER );



	Image * pFlag2= new Image();
	pFlag2->SetSource( pFlagBlock );
	pEventHandler->AddCallback( WgEventFilter::MouseButtonDrag(pFlag1, 1), cbDragWidget, pFlag1 );

	pHook = pFlex->addWidget( pFlag2, CoordI(100,100), WAX_CENTER );

	//
*/

//	PackPanel_p pVBox = PackPanel::create();
//	pVBox->setOrientation( WG_VERTICAL );
//	pFlex->addWidget( pVBox, CoordI(50,50), WAX_NORTHWEST );


//	Image * pFlag3 = new Image();
//	pFlag3->SetSource( pFlagBlock );
//	Image * pFlag4= new Image();
//	pFlag4->SetSource( pFlagBlock );

//	Button_p pButton2 = Button::cast(pDB->cloneWidget( "button" ));
//	pButton2->Label()->set( "BUTTON TEXT" );
//	pVBox->addWidget(pButton2);

//	pVBox->addWidget(pFlag3);
//	pVBox->addWidget(pFlag4);


//	SizeCapsule * pSizeCapsule = new SizeCapsule();
//	pSizeCapsule->setMaxSize( SizeI(150,600) );
//	pSizeCapsule->setWidget( pVBox );
//	FlexHook * p = pFlex->addWidget( pVBox, CoordI(50,50), WAX_NORTHWEST );
//	p->setSizePolicy( WG_BOUND, WG_DEFAULT );
//	p->setSize( SizeI(150,10 ) );

//	TextDisplay * pText1 = new TextDisplay();
//	pText1->setText("TEXTA1");
//	pText1->setEditMode(WG_TEXT_EDITABLE);
//	pVBox->addWidget(pText1);

//	TextDisplay_p pText2 = TextDisplay::create();
//	pText2->text()->set("TEXTB234ABC sajfas kjfaljsras kjasdfkasd kajfd fkajfa fkdjfa dfasfda asdkfj Hej");
//	pText2->setEditMode(WG_TEXT_EDITABLE);
//	pVBox->addWidget(pText2);

//	pText1->grabFocus();

	// Radiobuttons test

//	pVBox->addWidget( pDB->cloneWidget( "radiobutton" ) );
//	pVBox->addWidget( pDB->cloneWidget( "radiobutton" ) );

//	pFlex->addWidget( pDB->cloneWidget( "radiobutton" ) );
//	pFlex->addWidget( pDB->cloneWidget( "radiobutton" ) );

//	pVBox->SetRadioGroup(true);

	return pRoot;
}

//____ initSDL() ______________________________________________________________

SDL_Surface * initSDL( int w, int h )
{
    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 0;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* pScreen = SDL_SetVideoMode(w, h, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( !pScreen )
    {
        printf("Unable to set %dx%d video: %s\n", w, h, SDL_GetError());
        return 0;
    }

	SDL_EnableUNICODE(true);

	return pScreen;
}


//____ eventLoop() ____________________________________________________________

bool eventLoop( const WgEventHandlerPtr& pHandler )
{
	sdl_wglib::BeginEvents( pHandler );

   // message processing loop
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// check for messages
		switch (event.type)
		{
			// exit if the window is closed
			case SDL_QUIT:
				return false;

			// check for keypresses
			case SDL_KEYDOWN:
			{
				// exit if ESCAPE is pressed
//				if (event.key.keysym.sym == SDLK_ESCAPE)
//					return false;
			}
		}
		sdl_wglib::TranslateEvent( pHandler, event );
	}

	sdl_wglib::EndEvents( pHandler );

	return true;
}



Coord dragStartPos;

//____ cbInitDrag() ___________________________________________________________

void cbInitDrag( const WgEventPtr& _pEvent, const Object_p& pWidget )
{
	FlexHook * pHook = static_cast<FlexHook*>(Widget::cast(pWidget)->hook().GetRealPtr());


	dragStartPos = pHook->floatOfs();
	printf( "DRAG START!\n" );
}

//____ cbDragWidget() __________________________________________________________

void cbDragWidget( const WgEventPtr& _pEvent, const Object_p& pObject )
{
	Widget_p pWidget = Widget::cast(pObject);

	if( _pEvent->type() != WG_EVENT_MOUSE_DRAG || !pWidget->parent() )
		return;

	const WgMouseDragEventPtr pEvent = WgMouseDragEvent::cast(_pEvent);

	CoordI	dragDistance = pEvent->draggedTotal();

	CoordI	ofs = dragStartPos + dragDistance;

//	printf( "AccDistance: %d, %d\n", dragDistance.x, dragDistance.y );
	printf( "ofs: %d, %d   start: %d %d   distance: %d, %d\n", ofs.x, ofs.y, dragStartPos.x, dragStartPos.y, dragDistance.x, dragDistance.y );

	FlexHook * pHook = static_cast<FlexHook*>(pWidget->hook().GetRealPtr());
	pHook->setOfs(dragStartPos+dragDistance);
}

//____ cbOpenModal() __________________________________________________________

void cbOpenModal( const WgEventPtr& _pEvent, const Object_p& pWidget )
{
	g_pModal->addModalWidget( Widget::cast(pWidget), CoordI(), WAX_SOUTHEAST );
}

//____ cbCloseModal() __________________________________________________________

void cbCloseModal( const WgEventPtr& _pEvent, const Object_p& pWidget )
{
	g_pModal->removeWidget( Widget::cast(pWidget));
}

//____ cbResizeWidget() _________________________________________________________

void cbResize( const WgEventPtr& _pEvent, const Object_p& pWidget )
{
	FlexHook * pHook =   static_cast<FlexHook*>( Widget::cast(pWidget)->hook().GetRealPtr() );
	const WgMouseDragEventPtr pEvent = WgMouseDragEvent::cast(_pEvent);

	CoordI dragged = pEvent->draggedNow();

	pHook->setSize( pHook->size() + SizeI(dragged.x,dragged.y) );
}

//____ cbIncreaseEntryPadding() _________________________________________________________

void cbIncreaseEntryPadding( const WgEventPtr& _pEvent, const Object_p& _pWidget )
{
	Widget_p pWidget = Widget::cast(_pWidget);

	ExtendedSkin_p pSkin = ExtendedSkin::cast(pWidget->skin());
		
	RectI contentRect = pSkin->contentRect(RectI(0,0,100,100), WG_STATE_NORMAL );
	Borders padding( contentRect.top(), 100 - contentRect.right(), 100 - contentRect.bottom(), contentRect.left() );

	padding.top++;
	padding.left++;
	pSkin->setContentPadding(padding);
	pWidget->refresh();

//	List_p pList = List::cast(pWidget);
//	pList->setEntrySkin(pSkin);

}

//____ cbAddEntryText() _________________________________________________________

void cbAddEntryText( const WgEventPtr& _pEvent, const Object_p& _pWidget )
{
	TextDisplay_p pWidget = TextDisplay::cast(_pWidget);
	pWidget->text()->append("**** **** **** **** **** **** ****");
}


//____ addResizablePanel() _________________________________________________

void addResizablePanel( const FlexPanel_p& pParent, const Widget_p& pChild, const WgEventHandlerPtr& pEventHandler )
{
	pEventHandler->AddCallback( WgEventFilter::mouseDrag(WG_BUTTON_MIDDLE), pChild, cbResize, pChild );

	pEventHandler->AddCallback( WgEventFilter::mousePress(WG_BUTTON_RIGHT), pChild, cbInitDrag, pChild );
	pEventHandler->AddCallback( WgEventFilter::mouseDrag(WG_BUTTON_RIGHT), pChild, cbDragWidget, pChild );
}





