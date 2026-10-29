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

#include <wax_metalsurface.h>
#include <wax_metalsurfacefactory.h>
#include <wax_metaledgemapfactory.h>
#include <wax_metalbackend.h>

#import <QuartzCore/CAMetalLayer.h>

using namespace wg;
using namespace wax;

//____ CocoaWindowMetal ________________________________________________________
//
// Renders into a Metal canvas of our own, which is blitted to a drawable of the
// view's CAMetalLayer and presented after each render.

class CocoaWindowMetal : public CocoaWindow
{
public:
	CocoaWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	~CocoaWindowMetal();

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	void				_present();

	CAMetalLayer *		m_layer = nil;
	MetalBackend_p		m_pBackend;
	MetalSurface_p		m_pCanvas;
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
	Base::setDefaultDevice(pDevice);

	auto pSurfaceFactory = MetalSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = MetalEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);

	Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());

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

	SizeI pixels = _sizeInPixels();

	// A layer of our own makes the view layer-hosting, so AppKit leaves its content to us.

	m_layer = [[CAMetalLayer alloc] init];
	m_layer.device = g_metalDevice;
	m_layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
	m_layer.contentsScale = m_window.backingScaleFactor;
	m_layer.drawableSize = CGSizeMake(pixels.w, pixels.h);

	[m_view setLayer:m_layer];
	[m_view setWantsLayer:YES];

	m_pBackend = wax_static_cast<MetalBackend_p>(static_cast<GfxDeviceGen2*>(Base::defaultDevice().rawPtr())->backend());
	m_pBackend->setDefaultCanvas(nullptr, pixels, PixelFormat::ARGB_8);

	m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::ARGB_8, .scale = _scale(), .size = pixels });

	m_pRootPanel = RootPanel::create(m_pCanvas);
	m_pRootPanel->setWindowRef(reinterpret_cast<uintptr_t>(this));
}

//____ ~CocoaWindowMetal() ____________________________________________________

CocoaWindowMetal::~CocoaWindowMetal()
{
	[m_layer release];
}

//____ render() _______________________________________________________________

void CocoaWindowMetal::render()
{
	//TODO: We are re-rendering and presenting the whole window every time.

	if( !m_pRootPanel || m_bDestroyed || m_pRootPanel->nbDirtyRects() == 0 )
		return;

	m_pRootPanel->render();
	_present();
}

//____ onWindowSizeUpdated() __________________________________________________

void CocoaWindowMetal::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	SizeI pixels = _sizeInPixels();

	// Update drawable size immediately to prevent stretching

	m_layer.contentsScale = m_window.backingScaleFactor;
	m_layer.drawableSize = CGSizeMake(pixels.w, pixels.h);

	m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::ARGB_8, .scale = _scale(), .size = pixels });
	m_pRootPanel->setCanvas(m_pCanvas);

	_notifyUserWindowOfResize();

	m_pRootPanel->render();
	_present();
}

//____ _present() _____________________________________________________________

void CocoaWindowMetal::_present()
{
	@autoreleasepool {

		// Presenting with a transaction keeps the content in sync with the window
		// frame during live resize, but outside of that macOS can hold the frame
		// back for a long time, leaving the window black if nothing else is rendered.

		m_layer.presentsWithTransaction = m_window.inLiveResize;

		id<CAMetalDrawable> drawable = [m_layer nextDrawable];
		if( !drawable )
			return;

		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].loadAction  = MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].texture = drawable.texture;

		wg::SizeSPX size = m_pRootPanel->canvasSize();
		m_pBackend->setDefaultCanvas(pass, size/64, wg::PixelFormat::ARGB_8);

		m_pBackend->autopresent(drawable);

		auto pDevice = Base::defaultDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(CanvasRef::Default);
		pDevice->setBlitSource(m_pCanvas);
		pDevice->blit({0,0});
		pDevice->endCanvasUpdate();
		pDevice->endRender();
	}
}
