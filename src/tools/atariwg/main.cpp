#include <mint/sysbind.h>

#include <wondergui.h>

#include <wg_softbackend.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softkernels_rgb565be_base.h>
#include <wg_softkernels_rgb565be_extras.h>

using namespace wg;

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

    FILE * fp = fopen("SKN565BE.SRF", "rb" );
    
    if( fp == nullptr )
    {
        printf( "ERROR: Couldn't load SKN565BE.SRF\n");
        fclose(fp);
        Bconin(2);
        return -1;
    }

    fseek(fp,0,SEEK_END);
    
    int fileSize = (int) ftell(fp);
        
    char * pBuffer = (char*)malloc(fileSize);
    
    fseek(fp,0,SEEK_SET);
    
    int bytesRead = fread(pBuffer,1,fileSize,fp);
    fclose(fp);

    SurfaceReader_p pSurfaceReader = SurfaceReader::create({ .factory = pSurfaceFactory });

    auto pWidgetSurface = pSurfaceReader->readSurfaceFromMemory(pBuffer);
    
    free(pBuffer);
    
    
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

	pFlex->slots.pushBack( pFiller, { .pos = {10,10} } );

	pRoot->slot = pFlex;

    SurfaceDisplay_p pDisplay = SurfaceDisplay::create( { .surface = pWidgetSurface });
    
    pFlex->slots.pushFront( pDisplay, { .pos = { 20,20} } );
    
    
    
    printf("Render.\n");

    
	pRoot->render();

    short oldMode = VsetMode(0x114);

    void * pOldScreen = Physbase();

    VsetScreen(-1,pScreen,-1,-1);


    Bconin(2);

    
	VsetScreen(-1,pOldScreen,-1,-1);

	VsetMode(oldMode);


	Mfree(pScreen);

	Base::exit();
	
	Bconin(2);

	return 0;
}
