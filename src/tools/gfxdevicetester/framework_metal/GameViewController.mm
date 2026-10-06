//
//  GameViewController.m
//  gfxdevice_testapp_metal
//
//  Created by Tord Jansson on 2020-08-07.
//  Copyright © 2020 Nordic UI Technologies AB. All rights reserved.
//

#import "GameViewController.h"
#import "Renderer.h"

#include <wondergui.h>
#include <wg_metalgfxdevice.h>
#include <wg_metalsurfacefactory.h>

#include "../device.h"
#include "../gfxdevicetester.h"
#include "../app.h"

using namespace wg;

//____ MetalDevice _______________________________________________________________________

class MetalDevice : public Device
{
public:
    MetalDevice()
    {
    }

    const char * name() const
    {
        return m_pName;
    }

    bool init(SizeI canvasSize, PixelFormat canvasFormat)
    {
        Surface::Blueprint bp;
        bp.size = canvasSize;
        bp.format = canvasFormat;
        bp.canvas = true;
        
        m_pCanvas = MetalSurface::create(bp);
        m_pDevice = MetalGfxDevice::create();              // Needs a UBO binding point separate from other GfxDevice.
        return true;
    }

    void exit()
    {
        m_pDevice = nullptr;
        m_pCanvas = nullptr;
    }

    GfxDevice_p beginRender() const
    {
        m_pDevice->beginRender();
        m_pDevice->beginCanvasUpdate(m_pCanvas);
        return m_pDevice;
    }

    void endRender() const
    {
        m_pDevice->endCanvasUpdate();
        m_pDevice->endRender();
    }

    GfxDevice_p    gfxDevice() const
    {
        return m_pDevice;
    }

    Surface_p canvas() const
    {
        return m_pCanvas;
    }

private:

    const char * m_pName = { "Metal" };

    MetalGfxDevice_p      m_pDevice;
    MetalSurface_p        m_pCanvas;
};



//____ MyAppVisitor() ________________________________________________________

class MyAppVisitor : public AppVisitor
{
public:
    MyAppVisitor( RootPanel_p pRoot ) : m_pRoot(pRoot) {}
    
    wg::RootPanel_p    rootPanel() override
    {
        return          m_pRoot;
    }

    int64_t            time() override
    {
		
		dispatch_time_t now = dispatch_time(DISPATCH_TIME_NOW, 0);
		
        return now;
    }

    wg::Blob_p       loadBlob(const char* pPath) override;
    Surface_p        loadSurface(const char* pPath, SurfaceFactory* pFactory = nullptr,
                                 const Surface::Blueprint& blueprint = wg::Surface::Blueprint() ) override;
protected:
    RootPanel_p          m_pRoot;

};

//____ GameViewController ________________________________________________________________________________

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
    
    RootPanel_p             m_pRoot;

    GfxDeviceTester*        m_pApp;
    MyAppVisitor*           m_pVisitor;

    wg::MetalGfxDevice_p    m_pDevice;


    wg::Size                m_windowSize;

	float					m_OSXScale;
	float					m_WGScale;
}

//____ viewDidLoad _______________________________________________________

- (void)viewDidLoad
{
	
	m_OSXScale = 2.f;
	m_WGScale = 2.f;
	
    [super viewDidLoad];

    _view = (MTKView *)self.view;
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

    _view.device = MTLCreateSystemDefaultDevice();

    if(!_view.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    Base::init(nullptr);

    MetalGfxDevice::setMetalDevice( _view.device );
    m_pDevice = MetalGfxDevice::create();

    
    auto pContext = Context::create();
    pContext->setGammaCorrection(true);
    pContext->setGfxDevice(m_pDevice);
    pContext->setSurfaceFactory(MetalSurfaceFactory::create());
    
    Base::setActiveContext(pContext);

    // Get apps window size before we continue

    m_pApp = new GfxDeviceTester();
    m_windowSize = m_pApp->startWindowSize();

	MTLRenderPassDescriptor * pDesc = _view.currentRenderPassDescriptor;
	pDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    m_pDevice->setDefaultCanvas( pDesc, wg::SizeI(m_windowSize.w,m_windowSize.h), PixelFormat::ARGB_8, int(m_WGScale*64) );
	
    m_pRoot = RootPanel::create( CanvasRef::Default, m_pDevice);
    
    // Create app and visitor, make any app-specif initialization

    m_pVisitor = new MyAppVisitor(m_pRoot);


    m_pApp->addTestDevice(new SoftwareDevice());
    m_pApp->addTestDevice(new MetalDevice());

    // Initialize the app

    bool bContinue = m_pApp->init( m_pVisitor );

    //
    
    _renderer = [[Renderer alloc] initWithMetalKitView:_view rootPanel:m_pRoot];

    [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];

    _view.delegate = _renderer;



    //
    
    auto trackingArea = [[NSTrackingArea alloc] initWithRect:NSMakeRect(0,0, 20000, 20000)

        options: (NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow )

        owner:self userInfo:nil];

    [_view addTrackingArea:trackingArea];

    //
    
    [NSTimer scheduledTimerWithTimeInterval:0.016
    target:self
    selector:@selector(updateApp)
    userInfo:nil
     repeats:YES];
    
}

//____ updateApp _______________________________________________________________

- (void)updateApp
{
    // Dispatch messages

    Base::msgRouter()->dispatch();

    // Uppdate the app

    m_pApp->update();

    // Periodic update

    Base::update(16000);
}

//____ scrollWheel __________________________________________________________

- (void)scrollWheel:(NSEvent *)theEvent {
    
    bool bInverted = [theEvent isDirectionInvertedFromDevice];
    
    float fDeltaX = [theEvent deltaX];

    // Convert to an integer representing the number of scrolls events
    int iDeltaX;
    if(fDeltaX > 0.0f)
        iDeltaX = 1;
    else if(fDeltaX < 0.0f)
        iDeltaX = -1;
    else
        iDeltaX = 0;

    iDeltaX = 1*((int)fDeltaX + iDeltaX);

        
    float fDeltaY = [theEvent deltaY];

    // Convert to an integer representing the number of scrolls events
    int iDeltaY;
    if(fDeltaY > 0.0f)
        iDeltaY = 1;
    else if(fDeltaY < 0.0f)
        iDeltaY = -1;
    else
        iDeltaY = 0;

    iDeltaY = 1*((int)fDeltaY + iDeltaY);

    if( bInverted )
    {
        // Don't know if we should invert iDeltaX here, but I guess so.
        
        iDeltaX *= -1;
        iDeltaY *= -1;
    }
    
	if( iDeltaX != 0 )
		Base::inputHandler()->setWheelRoll(1, iDeltaX, bInverted );

	if( iDeltaY != 0 )
		Base::inputHandler()->setWheelRoll(1, iDeltaY, bInverted );

}

//____ mouseDown ___________________________________________________________

- (void)mouseDown:(NSEvent *)theEvent {

    Base::inputHandler()->setButton(MouseButton::Left, true);
}

//____ mouseUp _____________________________________________________________

- (void)mouseUp:(NSEvent *)theEvent {

    Base::inputHandler()->setButton(MouseButton::Left, false);
}

//____ rightMouseDown ______________________________________________________

- (void)rightMouseDown:(NSEvent *)theEvent {

    Base::inputHandler()->setButton(MouseButton::Right, true);
}

//____ rightMouseUp ________________________________________________________

- (void)rightMouseUp:(NSEvent *)theEvent {

    Base::inputHandler()->setButton(MouseButton::Right, false);
}

//____ mouseEntered ________________________________________________________

- (void)mouseEntered:(NSEvent *)theEvent {

    NSPoint mouseLoc = [_view convertPoint:[theEvent locationInWindow] fromView:nil];
         
         // mouseLoc has bottom, left orientation. GUI needs top, left
         NSRect r = [_view bounds];
         mouseLoc.y = r.size.height - mouseLoc.y;

    //     mouseLoc = pkCocoaAPI->ConvertMousePos(mouseLoc);

        Base::inputHandler()->setPointer(m_pRoot, { wg::pts(mouseLoc.x)*m_OSXScale/m_WGScale, wg::pts(mouseLoc.y)*m_OSXScale/m_WGScale } );

}

//____ mouseExited ________________________________________________________

- (void)mouseExited:(NSEvent *)theEvent {

    Base::inputHandler()->setPointer(m_pRoot, { -1, -1 } );

}

//____ mouseMoved ________________________________________________________

- (void)mouseMoved:(NSEvent *)theEvent {
   
    NSPoint mouseLoc = [_view convertPoint:[theEvent locationInWindow] fromView:nil];
     
     // mouseLoc has bottom, left orientation. GUI needs top, left
     NSRect r = [_view bounds];
     mouseLoc.y = r.size.height - mouseLoc.y;

//     mouseLoc = pkCocoaAPI->ConvertMousePos(mouseLoc);

    Base::inputHandler()->setPointer(m_pRoot, { wg::pts(mouseLoc.x)*m_OSXScale/m_WGScale, wg::pts(mouseLoc.y)*m_OSXScale/m_WGScale } );
}

//____ mouseDragged ________________________________________________________

- (void)mouseDragged:(NSEvent *)theEvent {

     NSPoint mouseLoc = [_view convertPoint:[theEvent locationInWindow] fromView:nil];
             
             // mouseLoc has bottom, left orientation. GUI needs top, left
             NSRect r = [_view bounds];
             mouseLoc.y = r.size.height - mouseLoc.y;

        //     mouseLoc = pkCocoaAPI->ConvertMousePos(mouseLoc);

            Base::inputHandler()->setPointer(m_pRoot, { wg::pts(mouseLoc.x)*m_OSXScale/m_WGScale, wg::pts(mouseLoc.y)*m_OSXScale/m_WGScale} );

}

//____ loadBlob() _________________________________________________________

Blob_p MyAppVisitor::loadBlob(const char* pPath)
{
    FILE* fp;

#ifdef WIN32
    errno_t err = fopen_s(&fp, pPath, "rb");
#else
    fp = fopen(pPath, "rb");
#endif
    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_END);
    int size = (int)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    Blob_p pBlob = Blob::create(size);

    int nRead = (int)fread(pBlob->data(), 1, size, fp);
    fclose(fp);

    if (nRead < size)
        return 0;

    return pBlob;
}

//____ loadSurface() ______________________________________________________

Surface_p MyAppVisitor::loadSurface(const char* pPath, SurfaceFactory* pFactory, const Surface::Blueprint& _bp)
{
    NSString * nsPath = [[NSString alloc] initWithUTF8String:pPath];
    
    
    NSImage * nsImage = [[NSImage alloc] initWithContentsOfFile:nsPath];
    
    if (!nsImage)
        return nullptr;

    // Dimensions - source image determines context size

    NSImageRep *rep = [[nsImage representations] objectAtIndex:0];
    NSSize imageSize = NSMakeSize(rep.pixelsWide, rep.pixelsHigh);
    
//    NSSize imageSize = nsImage.size;
    NSRect imageRect = NSMakeRect(0, 0, imageSize.width, imageSize.height);

    // Create a context to hold the image data

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

    CGContextRef ctx = CGBitmapContextCreate(NULL,
                                             imageSize.width,
                                             imageSize.height,
                                             8,
                                             0,
                                             colorSpace,
                                             kCGImageAlphaPremultipliedLast);

    // Wrap graphics context

    NSGraphicsContext* gctx = [NSGraphicsContext graphicsContextWithCGContext:ctx flipped:NO];

    // Make our bitmap context current and render the NSImage into it

    [NSGraphicsContext setCurrentContext:gctx];
    [nsImage drawInRect:imageRect];

    
    //

    size_t width = CGBitmapContextGetWidth(ctx);
    size_t height = CGBitmapContextGetHeight(ctx);
    uint32_t* pixel = (uint32_t*)CGBitmapContextGetData(ctx);
    
    int pitch = (int) CGBitmapContextGetBytesPerRow(ctx);
    
    PixelDescription output;
    
    PixelFormat format = Base::activeContext()->gammaCorrection() ? PixelFormat::ARGB_8 : PixelFormat::ARGB_8;

    output.format = format;
    output.bits = 32;
    output.bIndexed = false;
    output.bLinear = format == PixelFormat::ARGB_8 ? true : false;

    output.R_bits = 8;
    output.G_bits = 8;
    output.B_bits = 8;
    output.A_bits = 8;

    output.R_loss = 0;
    output.G_loss = 0;
    output.B_loss = 0;
    output.A_loss = 0;

    output.A_mask = 0xFF000000;
    output.R_mask = 0xFF;
    output.G_mask = 0xFF00;
    output.B_mask = 0xFF0000;

    output.A_shift = 24;
    output.R_shift = 0;
    output.G_shift = 8;
    output.B_shift = 16;

    
    PixelFormat px;

    if (output.A_bits > 0)
        px = PixelFormat::ARGB_8;
    else
        px = PixelFormat::XRGB_8;

    if (!pFactory)
        pFactory = Base::activeContext()->surfaceFactory();

//    auto pSurface = pFactory->createSurface(SizeI(width, height), px, (uint8_t*)pixel, pitch, &output, flags);

    Surface::Blueprint bp = _bp;
    bp.size.w = width;
    bp.size.h = height;
    bp.format = px;

    
    auto pSurface = pFactory->createSurface( bp, (uint8_t*)pixel, pitch, &output);

    // Clean up

    [NSGraphicsContext setCurrentContext:nil];
    CGContextRelease(ctx);
    CGColorSpaceRelease(colorSpace);



    return pSurface;
 
}


@end
