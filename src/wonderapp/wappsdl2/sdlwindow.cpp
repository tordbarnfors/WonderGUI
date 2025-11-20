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
#include <sdlwindow.h>

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
#include <wg_softbackend.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softkernels_default.h>

using namespace wg;


//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels( pBackend );

	auto pSoftDevice = GfxDeviceGen2::create(pBackend);

	Base::setDefaultGfxDevice(pSoftDevice);

	auto pSurfaceFactory = SoftSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = SoftEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);

	return true;
}

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{

}



//____ constructor ___________________________________________________

SDLWindow::SDLWindow(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	uint32_t flags = 0;
	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;

	Rect geo = { pos, size };

	SDL_Window* pSDLWindow = SDL_CreateWindow(title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
	if (pSDLWindow == NULL)
		return nullptr;

	auto pWindowSurface = _generateWindowSurface(pSDLWindow, geo.w, geo.h);
	 if (pWindowSurface == nullptr)
		 return nullptr;

	m_pRootPanel = RootPanel::create( { .canvasSurface = pWindowSurface } );

//	pRootPanel->setWindowRef((uintptr_t) pWindow.rawPtr());

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);

    m_pSDLWindow = pSDLWindow;
}

//____ destructor _____________________________________________________________

SDLWindow::~SDLWindow()
{
}

//____ render() _______________________________________________________________

void SDLWindow::render()
{
	m_pRootPanel->render();

	//TODO: Just update the dirty rectangles!

	SDL_UpdateWindowSurface(m_pSDLWindow);
}

//____ destroy() ______________________________________________________________

void SDLWindow::destroy()
{
	SDL_DestroyWindow(m_pSDLWindow);
	m_pSDLWindow = nullptr;					// Signal that window has been destroyed.
}

//____ setGeo() _______________________________________________________________

wg::Rect SDLWindow::setGeo(const wg::Rect& geo)
{
	//TODO: Calculate and update position as well.

	SDL_SetWindowSize(m_pSDLWindow, geo.w, geo.h);
	return geo;
}

//____ requestFocus() _________________________________________________________

bool SDLWindow::requestFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ releaseFocus() _________________________________________________________

bool SDLWindow::releaseFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ minimize() _____________________________________________________________

bool SDLWindow::minimize()
{
	//TODO: Implement!!!
	return false;
}

//____ restore() ______________________________________________________________

bool SDLWindow::restore()
{
	//TODO: Implement!!!
	return false;
}

//___ setTitle() ______________________________________________________________

bool SDLWindow::setTitle(std::string& title)
{
    SDL_SetWindowTitle(m_pSDLWindow, title.c_str());
    return true;
}

//____ title() ________________________________________________________________

std::string SDLWindow::title()
{
    return std::string(SDL_GetWindowTitle(m_pSDLWindow));
}

//____ SDLWindowId() __________________________________________________________

uint32_t SDLWindow::SDLWindowId()
{
    return SDL_GetWindowID(m_pSDLWindow);
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindow::onWindowSizeUpdated( int w, int h )
{
	auto pWindowSurface = _generateWindowSurface(m_pSDLWindow, w, h );
	m_pRootPanel->setCanvas(pWindowSurface);
}

//____ _generateWindowSurface() _______________________________________________

Surface_p SDLWindow::_generateWindowSurface(SDL_Window* pWindow, int width, int height )
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
