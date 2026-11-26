/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <cocoagfxbackend.h>

#include <wax_metalbitmap.h>
#include <wax_metalbitmapfactory.h>
#include <wax_metaledgemapfactory.h>
#include <wax_metalbackend.h>

#import <QuartzCore/QuartzCore.h>
#import <IOSurface/IOSurface.h>

#include <algorithm>

using namespace wg;
using namespace wax;

//____ CocoaWindowMetal ________________________________________________________
//
// Renders with Metal into buffers of our own, IOSurfaces with a texture on each,
// which the layer of the view shows one at a time. There is no CAMetalLayer and
// no canvas besides the buffers.
//
// A drawable of a CAMetalLayer comes back with nothing said about what is in it,
// so all of a window has to be sent to it for every change. Buffers of our own
// keep what they had, and we know what that is: each has a list of what has
// changed since it was last drawn into. A frame takes a buffer that nobody
// reads, copies what that buffer has missed from the one being shown, has Wax
// draw the dirty rects on top as the default canvas, and gives it to the layer.
// So a change is drawn once and no more than the change is copied.
//
// What this stands on, in the headers of the macOS SDK:
//
//   - CALayer.h names an IOSurfaceRef as something the contents of a layer
//     typically is. Nothing says how to tell a layer that the surface it shows
//     has new pixels, so a frame is always another surface than the one shown.
//   - MTLDevice has newTextureWithDescriptor:iosurface:plane:.
//   - IOSurfaceRef.h has the use count of a surface for those who keep a pool of
//     them, and IOSurfaceIsInUse() as the call that says whether a surface can
//     be taken again. It says CoreVideo "and other APIs" count that way, not
//     that Core Animation does while a layer shows a surface: that it does is
//     our reading. Were it wrong, a part being redrawn could show half done.
//   - Nothing says when what Metal has rendered into a surface can be seen by
//     whoever shows it. A command buffer that has completed is the one point
//     where the GPU is known to be done, so we wait for that before the layer
//     gets the surface, which holds the main thread for as long as the GPU
//     takes with the frame.
//
// Nothing holds us back to the rate of the display, as nextDrawable of a
// CAMetalLayer does: frames made faster than they are shown are made in vain.

class CocoaWindowMetal : public CocoaWindow
{
public:
	CocoaWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	~CocoaWindowMetal();

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	struct Buffer
	{
		IOSurfaceRef	surface = nullptr;
		id<MTLTexture>	texture = nil;			// On the surface. Ours to release.
		PatchesI		changed;				// What has changed since it was last drawn into, in pixels.
		bool			bAllChanged = true;		// All of it has, or the list got too long.
	};

	static const int	c_maxBuffers = 3;
	static const int	c_maxChangedRects = 64;		// More than this and all of it is copied.

	bool	_createBuffer( Buffer& buffer );
	void	_releaseBuffers();
	int		_pickBuffer();
	void	_bringUpToDate( Buffer& buffer );
	void	_addChanged( const RectSPX * pRects, int nRects, int drawnInto );
	void	_setDefaultCanvas( id<MTLTexture> texture );

	CALayer *			m_layer = nil;
	MetalBackend_p		m_pBackend;

	Buffer				m_buffers[c_maxBuffers];
	int					m_nBuffers = 0;
	int					m_front = -1;				// The buffer the layer shows, -1 for none.
	int					m_previousFront = -1;		// The one it showed before that.
	SizeI				m_bufferSize;				// In pixels, that of the window.
};


static id<MTLDevice>	g_metalDevice = nil;

//____ initGfxBackend() _______________________________________________________

bool initGfxBackend()
{
	g_metalDevice = MTLCreateSystemDefaultDevice();
	if (!g_metalDevice)
	{
		printf("No Metal device available\n");
		return false;
	}

	MetalBackend::setMetalDevice(g_metalDevice);

	auto pBackend = MetalBackend::create();
	auto pDevice = GfxDeviceGen2::create(pBackend);
	wg::Base::setDefaultDevice(pDevice);

	auto pBitmapFactory = MetalBitmapFactory::create();
	wg::Base::setDefaultBitmapFactory(pBitmapFactory);

	auto pEdgemapFactory = MetalEdgemapFactory::create();
	wg::Base::setDefaultEdgemapFactory(pEdgemapFactory);

	wg::Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());

	return true;
}

//____ exitGfxBackend() _______________________________________________________

void exitGfxBackend()
{
}

//____ createBackendWindow() __________________________________________________

CocoaWindow * createBackendWindow(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new CocoaWindowMetal(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ CocoaWindowMetal() _____________________________________________________

CocoaWindowMetal::CocoaWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	if (!_createNSWindow(title, origin, pos, size, resizable, open))
		return;

	// A layer of our own makes the view layer-hosting, so AppKit leaves its content to us.

	m_layer = [[CALayer alloc] init];
	m_layer.contentsGravity = kCAGravityTopLeft;		// No stretching while waiting for a new buffer.
	m_layer.contentsScale = m_window.backingScaleFactor;
	m_layer.backgroundColor = CGColorGetConstantColor(kCGColorBlack);
	m_layer.opaque = YES;

	[m_view setLayer:m_layer];
	[m_view setWantsLayer:YES];

	m_pBackend = wax_static_cast<MetalBackend_p>(static_cast<GfxDeviceGen2*>(wg::Base::defaultDevice().rawPtr())->backend());

	// The default canvas has the size of the window for the root panel to read. It gets a
	// buffer to draw into for each frame.

	SizeI pixels = _sizeInPixels();
	m_bufferSize = { std::max(pixels.w, 1), std::max(pixels.h, 1) };

	_setDefaultCanvas(nil);

	m_pRootPanel = RootPanel::create(CanvasRef::Default, wg::Base::defaultDevice());
	m_pRootPanel->setWindowRef(reinterpret_cast<uintptr_t>(this));
}

//____ ~CocoaWindowMetal() ____________________________________________________

CocoaWindowMetal::~CocoaWindowMetal()
{
	m_layer.contents = nil;
	[m_layer release];

	_releaseBuffers();
}

//____ render() _______________________________________________________________

void CocoaWindowMetal::render()
{
	if( !m_pRootPanel || m_bDestroyed || m_pRootPanel->nbDirtyRects() == 0 )
		return;

	@autoreleasepool {

		// With all buffers read by someone, what is dirty stays so until the next turn.

		int index = _pickBuffer();
		if( index < 0 )
			return;

		Buffer& buffer = m_buffers[index];

		_bringUpToDate(buffer);

		_setDefaultCanvas(buffer.texture);
		m_pRootPanel->render();
		_setDefaultCanvas(nil);

		_addChanged(m_pRootPanel->firstUpdatedRect(), m_pRootPanel->nbUpdatedRects(), index);

		// The GPU must be done with the buffer before anyone is shown it, see the top of the file.

		m_pBackend->waitForCompletion();

		// An explicit transaction is committed right away. An implicit one would wait
		// for AppKit's run loop, which doesn't run while we are in our own loop.

		[CATransaction begin];
		[CATransaction setDisableActions:YES];
		m_layer.contents = (__bridge id) buffer.surface;
		[CATransaction commit];

		m_previousFront = m_front;
		m_front = index;
	}
}

//____ onWindowSizeUpdated() __________________________________________________

void CocoaWindowMetal::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	m_layer.contentsScale = m_window.backingScaleFactor;

	// The buffers have the size of the window, so we start over with them. The layer keeps
	// the one it shows until it gets another.

	_releaseBuffers();

	SizeI pixels = _sizeInPixels();
	m_bufferSize = { std::max(pixels.w, 1), std::max(pixels.h, 1) };

	_setDefaultCanvas(nil);
	m_pRootPanel->setCanvas(CanvasRef::Default);

	_notifyUserWindowOfResize();

	render();
}

//____ _setDefaultCanvas() ____________________________________________________
//
// Makes a texture what Wax draws into as the default canvas, keeping what is in it. Without
// a texture the default canvas only has its size, and nothing can be drawn.

void CocoaWindowMetal::_setDefaultCanvas( id<MTLTexture> texture )
{
	MTLRenderPassDescriptor * pass = nil;

	if( texture != nil )
	{
		pass = [MTLRenderPassDescriptor renderPassDescriptor];			// Autoreleased, the backend retains it.
		pass.colorAttachments[0].texture = texture;
		pass.colorAttachments[0].loadAction = MTLLoadActionLoad;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
	}

	m_pBackend->setDefaultCanvas(pass, m_bufferSize, PixelFormat::ARGB_8, _scale());
}

//____ _createBuffer() ________________________________________________________
//
// A surface of the size of the window in BGRA, said to be sRGB, and a texture on it that
// Metal renders into as sRGB.

bool CocoaWindowMetal::_createBuffer( Buffer& buffer )
{
	NSDictionary * properties = @{
		(id) kIOSurfaceWidth :				@(m_bufferSize.w),
		(id) kIOSurfaceHeight :				@(m_bufferSize.h),
		(id) kIOSurfaceBytesPerElement :	@4,
		(id) kIOSurfacePixelFormat :		@((uint32_t) 'BGRA')
	};

	IOSurfaceRef surface = IOSurfaceCreate((__bridge CFDictionaryRef) properties);
	if( !surface )
		return false;

	CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
	CFPropertyListRef colorSpaceProperties = CGColorSpaceCopyPropertyList(colorSpace);

	if( colorSpaceProperties )
	{
		IOSurfaceSetValue(surface, kIOSurfaceColorSpace, colorSpaceProperties);
		CFRelease(colorSpaceProperties);
	}

	CGColorSpaceRelease(colorSpace);

	MTLTextureDescriptor * descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm_sRGB
																							   width:NSUInteger(m_bufferSize.w)
																							  height:NSUInteger(m_bufferSize.h)
																						   mipmapped:NO];
	descriptor.usage = MTLTextureUsageRenderTarget;

	id<MTLTexture> texture = [g_metalDevice newTextureWithDescriptor:descriptor iosurface:surface plane:0];

	if( texture == nil )
	{
		CFRelease(surface);
		return false;
	}

	buffer.surface = surface;
	buffer.texture = texture;
	buffer.changed.clear();
	buffer.bAllChanged = true;				// It has nothing yet.
	return true;
}

//____ _releaseBuffers() ______________________________________________________
//
// The layer holds on to the surface it shows, and Metal to textures it still draws into.

void CocoaWindowMetal::_releaseBuffers()
{
	for( int i = 0 ; i < m_nBuffers ; i++ )
	{
		[m_buffers[i].texture release];
		m_buffers[i].texture = nil;

		CFRelease(m_buffers[i].surface);
		m_buffers[i].surface = nullptr;

		m_buffers[i].changed.clear();
		m_buffers[i].bAllChanged = true;
	}

	m_nBuffers = 0;
	m_front = -1;
	m_previousFront = -1;
}

//____ _pickBuffer() __________________________________________________________
//
// A buffer to draw the next frame into: one that the layer doesn't show and that nobody
// reads, by IOSurfaceIsInUse(), and of those rather not the one that was shown last. A new
// one is made if there is none and we may have more. Returns -1 if we have to wait.

int CocoaWindowMetal::_pickBuffer()
{
	int pick = -1;

	for( int i = 0 ; i < m_nBuffers ; i++ )
	{
		if( i == m_front || IOSurfaceIsInUse(m_buffers[i].surface) )
			continue;

		if( pick < 0 || pick == m_previousFront )
			pick = i;
	}

	if( pick < 0 && m_nBuffers < c_maxBuffers )
	{
		if( _createBuffer(m_buffers[m_nBuffers]) )
			pick = m_nBuffers++;
	}

	return pick;
}

//____ _bringUpToDate() _______________________________________________________
//
// Copies what the buffer has missed from the one being shown, which has it all, in a command
// buffer that comes before the session that draws the frame. With nothing shown yet there
// is nothing to copy, and the root panel draws all of it.

void CocoaWindowMetal::_bringUpToDate( Buffer& buffer )
{
	if( m_front >= 0 && (buffer.bAllChanged || !buffer.changed.isEmpty()) )
	{
		id<MTLTexture> source = m_buffers[m_front].texture;

		id<MTLCommandBuffer> commandBuffer = [MetalBackend::metalCommandQueue() commandBuffer];
		commandBuffer.label = @"CocoaWindowMetal bring buffer up to date";

		id<MTLBlitCommandEncoder> blitEncoder = [commandBuffer blitCommandEncoder];

		if( buffer.bAllChanged )
		{
			[blitEncoder copyFromTexture:source sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(0, 0, 0)
							  sourceSize:MTLSizeMake(NSUInteger(m_bufferSize.w), NSUInteger(m_bufferSize.h), 1)
							   toTexture:buffer.texture destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(0, 0, 0)];
		}
		else
		{
			for( const RectI& rect : buffer.changed )
			{
				[blitEncoder copyFromTexture:source sourceSlice:0 sourceLevel:0 sourceOrigin:MTLOriginMake(NSUInteger(rect.x), NSUInteger(rect.y), 0)
								  sourceSize:MTLSizeMake(NSUInteger(rect.w), NSUInteger(rect.h), 1)
								   toTexture:buffer.texture destinationSlice:0 destinationLevel:0 destinationOrigin:MTLOriginMake(NSUInteger(rect.x), NSUInteger(rect.y), 0)];
			}
		}

		[blitEncoder endEncoding];
		[commandBuffer commit];
	}

	buffer.changed.clear();
	buffer.bAllChanged = false;
}

//____ _addChanged() __________________________________________________________
//
// Adds what a frame drew to the lists of the buffers it wasn't drawn into, which don't have
// it yet.

void CocoaWindowMetal::_addChanged( const RectSPX * pRects, int nRects, int drawnInto )
{
	for( int b = 0 ; b < m_nBuffers ; b++ )
	{
		Buffer& buffer = m_buffers[b];

		if( b == drawnInto || buffer.bAllChanged )
			continue;

		for( int i = 0 ; i < nRects ; i++ )
		{
			// Whole pixels, within the buffer.

			int x1 = std::max(int(pRects[i].x) >> 6, 0);
			int y1 = std::max(int(pRects[i].y) >> 6, 0);
			int x2 = std::min((int(pRects[i].x + pRects[i].w) + 63) >> 6, m_bufferSize.w);
			int y2 = std::min((int(pRects[i].y + pRects[i].h) + 63) >> 6, m_bufferSize.h);

			buffer.changed.add( RectI(x1, y1, x2 - x1, y2 - y1) );
		}

		if( buffer.changed.size() > c_maxChangedRects )
		{
			buffer.changed.clear();
			buffer.bAllChanged = true;
		}
	}
}
