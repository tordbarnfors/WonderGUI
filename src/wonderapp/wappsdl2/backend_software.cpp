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

#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_softedgemapfactory.h>
#include <wg_softbackend.h>
#include <wg_softkernels_default.h>


using namespace wg;


class SDLWindowSoftware : public SDLWindow
{
public:
	SDLWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowSoftware() {};

	void	render() override;
	void 	onWindowSizeUpdated( int w, int h ) override;

private:

	static wg::Surface_p   _generateWindowSurface(SDL_Window* pWindow, int width, int height );


};



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

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowSoftware(pUserWindow, origin, pos, size, title, resizable, open);
}


SDLWindowSoftware::SDLWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	uint32_t flags = 0;
	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;

	Rect geo = { pos, size };

	SDL_Window* pSDLWindow = SDL_CreateWindow(title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
	if (pSDLWindow == NULL)
		return;

	auto pWindowSurface = _generateWindowSurface(pSDLWindow, geo.w, geo.h);
	if (pWindowSurface == nullptr)
	{
		SDL_DestroyWindow(m_pSDLWindow);
		pSDLWindow = nullptr;
		return;
	 }

	m_pRootPanel = RootPanel::create( { .canvasSurface = pWindowSurface } );

//	pRootPanel->setWindowRef((uintptr_t) pWindow.rawPtr());

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);

	m_pSDLWindow = pSDLWindow;
}

//____ render() _______________________________________________________________

void SDLWindowSoftware::render()
{
	m_pRootPanel->render();

	//TODO: Just update the dirty rectangles!

	SDL_UpdateWindowSurface(m_pSDLWindow);
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowSoftware::onWindowSizeUpdated( int w, int h )
{
	auto pWindowSurface = _generateWindowSurface(m_pSDLWindow, w, h );
	m_pRootPanel->setCanvas(pWindowSurface);
}

//____ _generateWindowSurface() _______________________________________________

Surface_p SDLWindowSoftware::_generateWindowSurface(SDL_Window* pWindow, int width, int height )
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

