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
#include <sdlwindow_sw.h>

#ifdef WIN32
#    include <SDL.h>
#    include <SDL_image.h>
#    include <Windows.h>
#    include <libloaderapi.h>
#elif __APPLE__
#    include <SDL2/SDL.h>
#    include <SDL2_image/SDL_image.h>
#    include <dlfcn.h>
#else
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_image.h>
#    include <dlfcn.h>
#endif

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>

#include <wg_dragndropoverlay.h>
#include <wg_popupoverlay.h>




using namespace wg;


const TypeInfo SDLWindowSW::TYPEINFO = { "SDLSoftwareWindow", &SDLWindow::TYPEINFO };


//____ create() _______________________________________________________________

SDLWindow_p SDLWindow::create(const Blueprint& blueprint)
{
    Rect geo;

    geo = { blueprint.pos + Coord(4,20), blueprint.size};

    uint32_t flags = 0;

    if (blueprint.resizable)
        flags |= SDL_WINDOW_RESIZABLE;


    SDL_Window* pSDLWindow = SDL_CreateWindow(blueprint.title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
    if (pSDLWindow == NULL)
        return nullptr;

    if( !blueprint.minSize.isEmpty() )
        SDL_SetWindowMinimumSize(pSDLWindow,blueprint.minSize.w, blueprint.minSize.h);

    if( !blueprint.maxSize.isEmpty() )
        SDL_SetWindowMaximumSize(pSDLWindow,blueprint.maxSize.w, blueprint.maxSize.h);

    auto pDevice = Base::defaultGfxDevice();
    if( !pDevice )
    {
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels( pBackend );

		auto pSoftDevice = GfxDeviceGen2::create(pBackend);

        Base::setDefaultGfxDevice(pSoftDevice);
        pDevice = pSoftDevice;
        
        auto pSurfaceFactory = SoftSurfaceFactory::create();
        Base::setDefaultSurfaceFactory(pSurfaceFactory);

		auto pEdgemapFactory = SoftEdgemapFactory::create();
		Base::setDefaultEdgemapFactory(pEdgemapFactory);

	}
    
    auto pWindowSurface = SDLWindowSW::_generateWindowSurface(pSDLWindow, geo.w, geo.h);
    if (pWindowSurface == nullptr)
        return nullptr;

	auto pRootPanel = RootPanel::create( { .canvasSurface = pWindowSurface } );

    SDLWindowSW_p pWindow = new SDLWindowSW(blueprint.title, pRootPanel, geo, pSDLWindow);

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

    return pWindow;
}


//____ SDLWindowSW constructor ___________________________________________________

SDLWindowSW::SDLWindowSW(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow)
    : SDLWindow(title, pRootPanel, geo, pSDLWindow)
{
}

//____ typeInfo() _________________________________________________________

const wg::TypeInfo& SDLWindowSW::typeInfo(void) const
{
    return TYPEINFO;
}

//____ onWindowSizeUpdated() _________________________________________________

void SDLWindowSW::onWindowSizeUpdated( int w, int h )
{
    auto pWindowSurface = _generateWindowSurface(m_pSDLWindow, w, h );
    m_pRootPanel->setCanvas(pWindowSurface);
}


//____ _generateWindowSurface() _______________________________________________

Surface_p SDLWindowSW::_generateWindowSurface(SDL_Window* pWindow, int width, int height )
{
    SDL_Surface* pWinSurf = SDL_GetWindowSurface(pWindow);
    if (pWinSurf == nullptr)
    {
        //        printf("Unable to get window SDL Surface: %s\n", SDL_GetError());
        return nullptr;
    }

    PixelFormat format = PixelFormat::Undefined;

    switch (pWinSurf->format->BitsPerPixel)
    {
        case 32:
            format = PixelFormat::BGRX_8;
            break;
        case 24:
            format = PixelFormat::BGR_8;
            break;
        default:
        {
            printf("Unsupported pixelformat of SDL Surface!\n");
            return nullptr;
        }
    }

    Blob_p pCanvasBlob = Blob::create(pWinSurf->pixels, 0);
    auto pWindowSurface = SoftSurface::create( WGBP(Surface, _.size = {width, height}, _.format = format, _.canvas = true ), pCanvasBlob, pWinSurf->pitch);

    return pWindowSurface;
}
