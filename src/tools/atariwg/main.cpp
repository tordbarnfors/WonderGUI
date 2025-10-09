#include <mint/sysbind.h>

#include <wondergui.h>

#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb565be_extras.h>

using namespace wg;

Surface_p   loadSurface( const char * pPath );
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

    auto pWidgetSurface = loadSurface("SKN565BE.SRF");

    if( !pWidgetSurface )
    {
        Bconin(2);
        return -1;
    }

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
    
    
	printf("Creating GUI.\n");

	// We create a RootPanel. This is responsible for rendering the
	// tree of child widgets connected to it and handle their events.
	// We provide it the GfxDevice to use for rendering.

	RootPanel_p pRoot = RootPanel::create( pCanvas, pGfxDevice );

	pRoot->setSkin( ColorSkin::create( Color::Yellow ));

	FlexPanel_p pFlex = FlexPanel::create();

    Filler_p pFiller = Filler::create( {
        .defaultSize = {100,100},
        .skin = ColorSkin::create( HiColor(4096,0,0,4096) )
    });

//	pFlex->slots.pushBack( pFiller, { .pos = {10,10} } );

	pRoot->slot = pFlex;

    SurfaceDisplay_p pDisplay = SurfaceDisplay::create( { .surface = pWidgetSurface });
    
    pFlex->slots.pushFront( pDisplay, { .pos = { 20,20} } );
    
    TextDisplay_p pLabel = TextDisplay::create( { .display = { .text = "TESTING" }, .skin = ColorSkin::create( Color::Green ) } );
 
    pFlex->slots.pushFront( pLabel, { .pos = { 10,10}, .size = {60,20} } );

    
    printf("Render.\n");

    
	pRoot->render();

    short oldMode = VsetMode(0x114);

    void * pOldScreen = Physbase();

    VsetScreen(-1,pScreen,-1,-1);

    
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
    pFlex = nullptr;
    pFiller = nullptr;
    pWidgetSurface = nullptr;
    pDisplay = nullptr;
    pLabel = nullptr;
    pCanvasBlob = nullptr;
    pSurfaceFactory = nullptr;
    
	Base::exit();
	
	Bconin(2);

	return 0;
}

//____ loadSurface() __________________________________________________________

Surface_p loadSurface( const char * pPath )
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

    auto pSurface = pSurfaceReader->readSurfaceFromMemory(pBuffer);
    
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
