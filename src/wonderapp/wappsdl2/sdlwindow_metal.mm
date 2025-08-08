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

#include <wonderapp.h>
#include <sdlwindow_metal.h>

#include <SDL2/SDL.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <wg_base.h>

#include <wg_gfxdevice_gen2.h>
#include <wg_metalsurface.h>
#include <wg_metalsurfacefactory.h>
#include <wg_metaledgemapfactory.h>
#include <wg_metalbackend.h>
#include <wg_metalgfxdevicefactory.h>
#include <wg_metalbackend.h>

#include <wg_dragndropoverlay.h>
#include <wg_popupoverlay.h>


using namespace wg;


const TypeInfo SDLWindowMetal::TYPEINFO = { "SDLMetalWindow", &SDLWindow::TYPEINFO };

//int SDLWindowGL::s_windowCounter = 0;
//bool SDLWindowGL::s_bInitialized = false;

//____ create() _______________________________________________________________

SDLWindow_p SDLWindow::create(const Blueprint& blueprint)
{
    wg::Rect geo;

    geo = { blueprint.pos + Coord(4,20), blueprint.size};

    uint32_t flags = SDL_WINDOW_METAL;

    if (blueprint.resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    
    SDL_Window* pSDLWindow = SDL_CreateWindow(blueprint.title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
    if (pSDLWindow == NULL)
        return nullptr;

    if( !blueprint.minSize.isEmpty() )
        SDL_SetWindowMinimumSize(pSDLWindow,blueprint.minSize.w, blueprint.minSize.h);

    if( !blueprint.maxSize.isEmpty() )
        SDL_SetWindowMaximumSize(pSDLWindow,blueprint.maxSize.w, blueprint.maxSize.h);


    SDL_Renderer *renderer = SDL_CreateRenderer(pSDLWindow, -1, SDL_RENDERER_PRESENTVSYNC);
        const CAMetalLayer *swapchain = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(renderer);
        const id<MTLDevice> gpu = swapchain.device;

	auto pDevice = Base::defaultGfxDevice();
	if( !pDevice )
	{
		MetalBackend::setMetalDevice(gpu);

		auto pBackend = MetalBackend::create();
		pDevice = GfxDeviceGen2::create(pBackend);
		Base::setDefaultGfxDevice(pDevice);

		auto pSurfaceFactory = MetalSurfaceFactory::create();
		Base::setDefaultSurfaceFactory(pSurfaceFactory);

		auto pEdgemapFactory = MetalEdgemapFactory::create();
		Base::setDefaultEdgemapFactory(pEdgemapFactory);


		auto pGfxDeviceFactory = MetalGfxDeviceFactory::create( WGBP(MetalGfxDeviceFactory,
																	 _.device = gpu,
																	 _.defaultCanvasSize = {int(geo.w),int(geo.h)},
																	 _.defaultCanvasPixelFormat = PixelFormat::BGRA_8_sRGB ));
		Base::setDefaultGfxDeviceFactory(pGfxDeviceFactory);
	}

	auto pBackend = static_cast<GfxDeviceGen2*>(Base::defaultGfxDevice().rawPtr())->backend();
	wg_static_cast<MetalBackend_p>(pBackend)->setDefaultCanvas(nullptr, {int(geo.w),int(geo.h)}, PixelFormat::BGRA_8_sRGB);
    auto pRootPanel = RootPanel::create(CanvasRef::Default, nullptr);

    SDLWindowMetal_p pWindow = new SDLWindowMetal(blueprint.title, pRootPanel, geo, pSDLWindow, renderer);

	pRootPanel->setWindowRef((uintptr_t) pWindow.rawPtr());

    if (blueprint.finalizer)
        pWindow->setFinalizer(blueprint.finalizer);
    
	// Setup basic widget hierarchy.

	auto pDragNDropOverlay = DragNDropOverlay::create();
	pRootPanel->slot = pDragNDropOverlay;

	auto pPopupOverlay = PopupOverlay::create();
	pDragNDropOverlay->mainSlot = pPopupOverlay;

	pWindow->m_pLastOverlay = pPopupOverlay;

    //TODO: This is ugly. It should be handled when windows gets focused.

    Base::inputHandler()->setFocusedWindow(pRootPanel);

//    SDLWindowGL::s_bInitialized = true;
    return pWindow;
}


//____ SDLWindowMetal constructor ___________________________________________________

SDLWindowMetal::SDLWindowMetal(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow, SDL_Renderer * pRenderer)
    : SDLWindow(title, pRootPanel, geo, pSDLWindow),
    m_pSDLRenderer(pRenderer)
{
	m_pBackend = static_cast<GfxDeviceGen2*>(Base::defaultGfxDevice().rawPtr())->backend();

//    s_windowCounter++;
}

//____ typeInfo() _________________________________________________________

const wg::TypeInfo& SDLWindowMetal::typeInfo(void) const
{
    return TYPEINFO;
}

//____ onWindowSizeUpdated() _________________________________________________

void SDLWindowMetal::onWindowSizeUpdated( int width, int height )
{
    m_geo.w = width;
    m_geo.h = height;
    
    wg_static_cast<MetalBackend_p>(m_pBackend)->setDefaultCanvas(nullptr, SizeSPX(width, height), wg::PixelFormat::BGRA_8_sRGB, 64);

    m_pRootPanel->setCanvas(CanvasRef::Default);
}

//____ render() _______________________________________________________________

void SDLWindowMetal::render()
{
	if( m_pRootPanel->nbDirtyRects() == 0 )
		return;

    @autoreleasepool {
		
		const CAMetalLayer *swapchain = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(m_pSDLRenderer);
	
		swapchain.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

		id<CAMetalDrawable> surface = [swapchain nextDrawable];

        MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
        pass.colorAttachments[0].loadAction  = MTLLoadActionLoad;
        pass.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass.colorAttachments[0].texture = surface.texture;

		wg_static_cast<MetalBackend_p>(m_pBackend)->setDefaultCanvas(pass, {int(m_geo.w),int(m_geo.h)}, wg::PixelFormat::BGRA_8_sRGB);

        wg_static_cast<MetalBackend_p>(m_pBackend)->autopresent(surface);

		m_pRootPanel->addDirtyPatch({0,0,int(m_geo.w)*64,int(m_geo.h)*64});
		
        m_pRootPanel->render();
    }
}
