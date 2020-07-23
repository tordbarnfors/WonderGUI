/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Header for a platform independent renderer class, which performs Metal setup and per frame rendering.
*/

#import "MetalKit/MetalKit.h"

@interface Renderer : NSObject<MTKViewDelegate>

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;

@end
