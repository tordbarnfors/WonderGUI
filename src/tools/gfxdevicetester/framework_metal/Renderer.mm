/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Implementation of a platform independent renderer class, which performs Metal setup and per frame rendering
*/

#import "MetalKit/MetalKit.h"

#import "Renderer.h"

#include <wondergui.h>
#include <wg_metalgfxdevice.h>
#include <wg_metalsurfacefactory.h>

using namespace wg;



// Main class performing the rendering
@implementation Renderer
{
    id<MTLDevice> _device;

    // The render pipeline generated from the vertex and fragment shaders in the .metal shader file.
    id<MTLRenderPipelineState> _pipelineState;

    // The command queue used to pass commands to the device.
    id<MTLCommandQueue> _commandQueue;

    // The current size of the view, used as an input to the vertex shader.
    simd_uint2 _viewportSize;

    RootPanel_p             m_pRootPanel;
 
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView rootPanel:(RootPanel_p)pRootPanel
{
    self = [super init];
    if(self)
    {
//        mtkView.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        
        m_pRootPanel = pRootPanel;

    }

    return self;
}

/// Called whenever view changes orientation or is resized
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    // Save the size of the drawable to pass to the vertex shader.
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
}

/// Called whenever the view needs to render a frame.
- (void)drawInMTKView:(nonnull MTKView *)view
{
    MTLRenderPassDescriptor * pDesc = view.currentRenderPassDescriptor;
    pDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;

    auto pDevice = wg_static_cast<MetalGfxDevice_p>(m_pRootPanel->gfxDevice());
    
    pDevice->autopresent(view.currentDrawable);

	pDevice->setDefaultCanvas( pDesc, {(int)_viewportSize.x,(int)_viewportSize.y}, PixelFormat::ARGB_8, m_pRootPanel->scale() );
    
    m_pRootPanel->setCanvas( CanvasRef::Default );
    
    // Render. We do this outside the app since we might want to
    // handle updated rectangles in a system specific way.
    
    m_pRootPanel->addDirtyPatch({0,0,(int)_viewportSize.x,(int)_viewportSize.y});
    
    m_pRootPanel->render();
}



@end
