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
#include <sdlwindow.h>

#include <wax_metalsurface.h>
#include <wax_metalsurfacefactory.h>
#include <wax_metaledgemapfactory.h>
#include <wax_metalbackend.h>

#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/NSWindow.h>


using namespace wg;
using namespace wax;


class SDLWindowMetal : public SDLWindow
{
public:
	SDLWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowMetal();

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	CAMetalLayer *		_layer();
	void				_present(bool bWithTransaction);

	SDL_MetalView		m_metalView = nullptr;
	MetalBackend_p		m_pBackend;
	MetalSurface_p		m_pCanvas;
};


static id<MTLDevice>	g_metalDevice = nil;

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	g_metalDevice = MTLCreateSystemDefaultDevice();
	if (!g_metalDevice) {
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

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{
}

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowMetal(pUserWindow,origin,pos,size,title,resizable,open);
}

//____ SDLWindowMetal() _______________________________________________________

SDLWindowMetal::SDLWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	if (!_createSDLWindow(title, origin, pos, size, SDL_WINDOW_METAL, resizable, open))
		return;

	m_metalView = SDL_Metal_CreateView(m_pSDLWindow);
	if (!m_metalView)
	{
		printf("Unable to create Metal view: %s\n", SDL_GetError());
		return;
	}

	SizeI pixels = _sizeInPixels();
	int scale = _scale();

	@autoreleasepool {

		CAMetalLayer * layer = _layer();
		layer.device = g_metalDevice;
		layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
		layer.drawableSize = CGSizeMake(pixels.w, pixels.h);

		m_pBackend = wax_static_cast<MetalBackend_p>(static_cast<GfxDeviceGen2*>(Base::defaultDevice().rawPtr())->backend());
		m_pBackend->setDefaultCanvas(nullptr, pixels, PixelFormat::ARGB_8);

		m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::ARGB_8, .scale = scale, .size = pixels });
	}

	m_pRootPanel = RootPanel::create(m_pCanvas);

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ ~SDLWindowMetal() ______________________________________________________

SDLWindowMetal::~SDLWindowMetal()
{
	if (m_metalView)
		SDL_Metal_DestroyView(m_metalView);
}

//____ _layer() _______________________________________________________________

CAMetalLayer * SDLWindowMetal::_layer()
{
	return (__bridge CAMetalLayer *)SDL_Metal_GetLayer(m_metalView);
}

//____ render() _______________________________________________________________

void SDLWindowMetal::render()
{
	//TODO: This is awful! We are re-rendering and presenting whole screen every time!

	if( !m_pRootPanel || m_pRootPanel->nbDirtyRects() == 0 )
		return;

	m_pRootPanel->render();
	_present(false);
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowMetal::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	SizeI pixels = _sizeInPixels();

	@autoreleasepool {

		// Update drawable size immediately to prevent stretching

		_layer().drawableSize = CGSizeMake(pixels.w, pixels.h);

		m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::ARGB_8, .scale = _scale(), .size = pixels });
		m_pRootPanel->setCanvas(m_pCanvas);
		m_pRootPanel->render();
	}

	_notifyUserWindowOfResize();

	// Presenting with a transaction keeps the content in sync with the window
	// frame during live resize, but outside of that macOS can hold the frame back
	// for a long time, leaving the window black if nothing else is rendered.

	NSWindow * pNSWindow = (__bridge NSWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(m_pSDLWindow), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
	_present( pNSWindow && [pNSWindow inLiveResize] );
}

//____ _present() _____________________________________________________________

void SDLWindowMetal::_present(bool bWithTransaction)
{
	@autoreleasepool {

		CAMetalLayer * swapchain = _layer();

		swapchain.presentsWithTransaction = bWithTransaction ? YES : NO;

		id<CAMetalDrawable> surface = [swapchain nextDrawable];

		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].loadAction  = MTLLoadActionDontCare;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].texture = surface.texture;

		wg::SizeSPX size = m_pRootPanel->canvasSize();
		m_pBackend->setDefaultCanvas(pass, size/64, wg::PixelFormat::ARGB_8);

		m_pBackend->autopresent(surface);

		auto pDevice = Base::defaultDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(CanvasRef::Default);
		pDevice->setBlitSource(m_pCanvas);
		pDevice->blit({0,0});
		pDevice->endCanvasUpdate();

		pDevice->endRender();
	}
}
