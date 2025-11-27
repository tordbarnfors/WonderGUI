#include <mint/sysbind.h>

#include <wondergui.h>

#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb565be_extras.h>

#include <themes/simplistic/wg_simplistic.h>

using namespace wg;

Surface_p   loadSurface( const char * pPath, const Surface::Blueprint& bp = {} );
Blob_p      loadBlob( const char * pPath );

int main( int argc, char * argv[] )
{    
    printf("Loaded.\n");

    Base::init(nullptr);

    auto pSurfaceFactory = SoftSurfaceFactory::create();
    Base::setDefaultSurfaceFactory(pSurfaceFactory);
    
    printf("Base initialized.\n");

	long pScreen = Malloc(2*320*240);

    printf("Screen initialized.\n");

	short * p = (short *) pScreen;

//	for( int x = 0 ; x < 320*240 ; x++ )
//		* p++ = 0xFF;

//    printf("Screen cleared.\n");


	Blob_p pCanvasBlob = Blob::create( (char *) pScreen, 0);
	SoftSurface_p pCanvas = SoftSurface::create( WGBP(Surface,
													  _.size = SizeI(320,240),
													  _.format = PixelFormat::RGB_565_bigendian,
                                                      _.canvas = true ),
												 pCanvasBlob,
												 320*2 );

    printf("Blob created.\n");


	// Wg create the GfxDevice that will be used for all rendering, providing
	// it our canvas to draw up.

    printf("Creating SoftBackend.\n");

	SoftBackend_p pBackend = SoftBackend::create();

    printf("Creating GfxDevice.\n");

    GfxDevice_p pGfxDevice = GfxDeviceGen2::create(pBackend);

    printf("Adding softkernels.\n");

    addBaseSoftKernelsForRGB565BECanvas( pBackend );
	addExtraSoftKernelsForRGB565BECanvas( pBackend );

    printf("Loading widget surface.\n");

    auto pWidgetSurface = loadSurface("SKN565BE.SRF" );
    
    if( !pWidgetSurface )
    {
        Bconin(2);
        return -1;
    }

    if( pWidgetSurface->isOpaque() )
        printf( "IS OPAQUE\n");
    
    printf("Loading font.\n");
        
    auto pFontSurface = loadSurface("FNT_5X7.SRF");
    auto pFontSpec = loadBlob("FNT_5X7.FNT");

    printf("Passed 1.\n");

    
    if( !pFontSurface || !pFontSpec )
    {
        Bconin(2);
        return -1;
    }

    printf("Passed 2.\n");

    auto pFont = BitmapFont::create( pFontSurface, (char *) pFontSpec->data() );
    
    auto pStyle = TextStyle::create( { .color = HiColor::White, .font = pFont, .size = 10 });
    Base::setDefaultStyle(pStyle);
    
    // Initialize theme
    
    auto pTheme = Simplistic::create(pFont,pFont,pFont,pFont,pWidgetSurface);
    
    
    
	printf("Creating GUI.\n");

	// We create a RootPanel. This is responsible for rendering the
	// tree of child widgets connected to it and handle their events.
	// We provide it the GfxDevice to use for rendering.

	RootPanel_p pRoot = RootPanel::create( pCanvas, pGfxDevice );

	pRoot->setSkin( ColorSkin::create( Color::Yellow ));

    PackPanel_p pMainPanel = PackPanel::create( { .axis = Axis::Y });
    
    PackPanel_p pTopBar = PackPanel::create( { .axis = Axis::X } );

    TextDisplay_p pTopLabel = TextDisplay::create( WGOVR(pTheme->windowTitleBar(), _.display.text = "WONDERGUI TEST APP" ));
    
    pTopBar->slots.pushBack(pTopLabel);
    
    SplitPanel_p pMidSection = SplitPanel::create( WGOVR(pTheme->splitPanelX()));

    PackPanel_p pTreeSection = PackPanel::create( { .skin = pTheme->canvasSkin() });
    pMidSection->slots[0] = pTreeSection;

    ScrollPanel_p pTextSection = ScrollPanel::create( pTheme->scrollPanelXY() );
    pMidSection->slots[1] = pTextSection;

    TextEditor_p pTextEditor = TextEditor::create( WGOVR(pTheme->textEditor(), _.editor.text = "This is where you will edit the text of the entry." ));
    
    pTextSection->slot = pTextEditor;
    
    PackPanel_p pBottomBar = PackPanel::create( { .axis = Axis::X, .skin = pTheme->plateSkin() });

    
    Button_p pButtonSave = Button::create( WGOVR(pTheme->pushButton(), _.label.text = "Save" ));
    Button_p pButtonQuit = Button::create( WGOVR(pTheme->pushButton(), _.label.text = "Quit" ));

    pBottomBar->slots.pushBack( { Filler::create(), pButtonSave, pButtonQuit } );
    pBottomBar->setSlotWeight( 0, 3, { 1.f, 0.f, 0.f });
    

    pMainPanel->slots.pushBack( { pTopBar, pMidSection, pBottomBar } );
    pMainPanel->setSlotWeight( 0, 3, { 0.f, 1.f, 0.f });

    

	pRoot->slot = pMainPanel;

    
    printf("Render.\n");

    

    short oldMode = VsetMode(0x114);

    void * pOldScreen = Physbase();

    VsetScreen(-1,pScreen,-1,-1);

    pRoot->render();

    Bconin(2);

    
	VsetScreen(-1,pOldScreen,-1,-1);

	VsetMode(oldMode);


	Mfree(pScreen);

    pGfxDevice = nullptr;
    pBackend = nullptr;
    pFontSurface = nullptr;
    pFontSpec = nullptr;
    pFont = nullptr;
    pRoot = nullptr;
//    pFlex = nullptr;
//    pFiller = nullptr;
    pWidgetSurface = nullptr;
//    pDisplay = nullptr;
//    pLabel = nullptr;
    pCanvasBlob = nullptr;
    pSurfaceFactory = nullptr;
    
	Base::exit();
	
	Bconin(2);

	return 0;
}

//____ loadSurface() __________________________________________________________

Surface_p loadSurface( const char * pPath, const Surface::Blueprint& bp )
{
    FILE * fp = fopen( pPath, "rb" );
    
    if( fp == nullptr )
    {
        printf( "ERROR: Couldn't load %s\n", pPath);
        fclose(fp);
        return nullptr;
    }

    fseek(fp,0,SEEK_END);
    
    int fileSize = (int) ftell(fp);

    char * pBuffer = (char*)malloc(fileSize);
    
    fseek(fp,0,SEEK_SET);
    
    int bytesRead = fread(pBuffer,1,fileSize,fp);
    fclose(fp);
    
    auto pSurfaceFactory = SoftSurfaceFactory::create();
    SurfaceReader_p pSurfaceReader = SurfaceReader::create({ .factory = pSurfaceFactory });

    auto pSurface = pSurfaceReader->readSurfaceFromMemory(pBuffer, bp );
    
    free(pBuffer);

    return pSurface;
}

//____ loadBlob() __________________________________________________________

Blob_p loadBlob( const char * pPath )
{
    FILE * fp = fopen( pPath, "rb" );
    
    if( fp == nullptr )
    {
        printf( "ERROR: Couldn't load %s\n", pPath);
        fclose(fp);
        return nullptr;
    }

    fseek(fp,0,SEEK_END);
    
    int fileSize = (int) ftell(fp);

    auto pBlob = Blob::create(fileSize+1);
        
    fseek(fp,0,SEEK_SET);
    
    int bytesRead = fread(pBlob->data(),1,fileSize,fp);
    fclose(fp);

    ((char*)pBlob->data())[fileSize] = 0;           // Zero-terminating content.
    
    return pBlob;
}
