/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <sdlwindow.h>

#include <wg_metalsurface.h>
#include <wg_metalsurfacefactory.h>
#include <wg_metaledgemapfactory.h>
#include <wg_metalbackend.h>


using namespace wg;


class SDLWindowMetal : public SDLWindow
{
public:
	SDLWindowMetal(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowMetal() {};

	void	render() override;
	void 	onWindowSizeUpdated( int w, int h ) override;

private:

	SDL_Renderer *      m_pSDLRenderer = 0;
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
	Base::setDefaultGfxDevice(pDevice);

	auto pSurfaceFactory = MetalSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = MetalEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);


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
	wg::Rect geo;

	geo = { pos + Coord(4,20), size};

	uint32_t flags = SDL_WINDOW_METAL | SDL_WINDOW_ALLOW_HIGHDPI;

	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;


	SDL_Window* pSDLWindow = SDL_CreateWindow(title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
	if (pSDLWindow == NULL)
		return nullptr;
	/*
	 if( !blueprint.minSize.isEmpty() )
	 SDL_SetWindowMinimumSize(pSDLWindow,blueprint.minSize.w, blueprint.minSize.h);

	 if( !blueprint.maxSize.isEmpty() )
	 SDL_SetWindowMaximumSize(pSDLWindow,blueprint.maxSize.w, blueprint.maxSize.h);
	 */

	@autoreleasepool {

		m_pSDLRenderer = SDL_CreateRenderer(pSDLWindow, -1, SDL_RENDERER_PRESENTVSYNC);
		const CAMetalLayer* layer = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(m_pSDLRenderer);
		layer.device = g_metalDevice;

		CGFloat scaleFactor = layer.contentsScale;

		auto pDevice = Base::defaultGfxDevice();

		m_pBackend = wg_static_cast<MetalBackend_p>(static_cast<GfxDeviceGen2*>(Base::defaultGfxDevice().rawPtr())->backend());
		m_pBackend->setDefaultCanvas(nullptr, {int(geo.w),int(geo.h)}, PixelFormat::BGRA_8_sRGB);

		m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::BGRA_8_sRGB, .scale = int(scaleFactor)*64, .size = {int(geo.w*scaleFactor),int(geo.h*scaleFactor)} });

	}

	auto pRootPanel = RootPanel::create(m_pCanvas);

//	m_pRootPanel->setWindowRef((uintptr_t) pWindow.rawPtr());

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);

	m_pRootPanel = pRootPanel;
	m_pSDLWindow = pSDLWindow;
	m_pUserWindow = pUserWindow;
}

//____ render() _______________________________________________________________

void SDLWindowMetal::render()
{
	//TODO: This is awful! We are re-rendering and presenting whole screen every time!

	if( m_pRootPanel->nbDirtyRects() == 0 )
		return;

	m_pRootPanel->render();


	@autoreleasepool {

		const CAMetalLayer *swapchain = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(m_pSDLRenderer);

		swapchain.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
		swapchain.presentsWithTransaction = NO;

		id<CAMetalDrawable> surface = [swapchain nextDrawable];

		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].loadAction  = MTLLoadActionLoad;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].texture = surface.texture;

		wg::SizeI size = m_pRootPanel->canvasSize();
		m_pBackend->setDefaultCanvas(pass, size, wg::PixelFormat::BGRA_8_sRGB);

		m_pBackend->autopresent(surface);

		auto pDevice = Base::defaultGfxDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(CanvasRef::Default);
		pDevice->setBlitSource(m_pCanvas);
		pDevice->blit({0,0});
		pDevice->endCanvasUpdate();

		pDevice->endRender();

	}
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowMetal::onWindowSizeUpdated( int w, int h )
{
	@autoreleasepool {
		CAMetalLayer* swapchain = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(m_pSDLRenderer);

		CGFloat scaleFactor = swapchain.contentsScale;

		w *= scaleFactor;
		h *= scaleFactor;

		// Update drawable size immediately to prevent stretching
		swapchain.drawableSize = CGSizeMake(w, h);
		swapchain.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
		swapchain.presentsWithTransaction = YES;


		id<CAMetalDrawable> surface = [swapchain nextDrawable];

		MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
		pass.colorAttachments[0].loadAction  = MTLLoadActionLoad;
		pass.colorAttachments[0].storeAction = MTLStoreActionStore;
		pass.colorAttachments[0].texture = surface.texture;

		m_pCanvas = MetalSurface::create({.canvas = true, .format = PixelFormat::BGRA_8_sRGB, .scale = int(scaleFactor*64), .size = {w,h} });
		m_pRootPanel->setCanvas(m_pCanvas);
		m_pRootPanel->render();

		wg::SizeI size = m_pRootPanel->canvasSize();
		m_pBackend->setDefaultCanvas(pass, size, wg::PixelFormat::BGRA_8_sRGB);
		m_pBackend->autopresent(surface);

		auto pDevice = Base::defaultGfxDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(CanvasRef::Default);
		pDevice->setBlitSource(m_pCanvas);
		pDevice->blit({0,0});
		pDevice->endCanvasUpdate();

		pDevice->endRender();
	}

}


