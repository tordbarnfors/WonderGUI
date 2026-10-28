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

#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>


using namespace wg;
using namespace wax;


class SDLWindowSoftware : public SDLWindow
{
public:
	SDLWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowSoftware() {};

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	wg::Surface_p	_generateWindowSurface();
};



//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels( pBackend );

	auto pSoftDevice = GfxDeviceGen2::create(pBackend);

	Base::setDefaultDevice(pSoftDevice);

	auto pSurfaceFactory = SoftSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = SoftEdgemapFactory::create();
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
	return new SDLWindowSoftware(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ SDLWindowSoftware() ____________________________________________________

SDLWindowSoftware::SDLWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	if (!_createSDLWindow(title, origin, pos, size, 0, resizable, open))
		return;

	auto pWindowSurface = _generateWindowSurface();
	if (pWindowSurface == nullptr)
	{
		SDL_DestroyWindow(m_pSDLWindow);
		m_pSDLWindow = nullptr;
		return;
	}

	m_pRootPanel = RootPanel::create( { .canvasSurface = pWindowSurface } );

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ render() _______________________________________________________________

void SDLWindowSoftware::render()
{
	if (!m_pRootPanel)
		return;

	m_pRootPanel->render();

    int nRects = m_pRootPanel->nbUpdatedRects();

    if( nRects > 0 )
    {
        const RectSPX* pUpdatedRects = m_pRootPanel->firstUpdatedRect();
        SDL_Rect* pSDLRects = (SDL_Rect*)Base::memStackAlloc(sizeof(SDL_Rect) * nRects);

        for (int i = 0; i < nRects; i++)
        {
            pSDLRects[i].x = pUpdatedRects[i].x / 64;
            pSDLRects[i].y = pUpdatedRects[i].y / 64;
            pSDLRects[i].w = pUpdatedRects[i].w / 64;
            pSDLRects[i].h = pUpdatedRects[i].h / 64;
        }

        SDL_UpdateWindowSurfaceRects(m_pSDLWindow, pSDLRects, nRects);

        Base::memStackFree(sizeof(SDL_Rect) * nRects);
    }
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowSoftware::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	// The window surface is invalid after a resize and SDL_GetWindowSurface()
	// gives us a new one.

    auto pWindowSurface = _generateWindowSurface();
	if (!pWindowSurface)
		return;

	m_pRootPanel->setCanvas(pWindowSurface);
	_notifyUserWindowOfResize();
    render();
}

//____ _generateWindowSurface() _______________________________________________

Surface_p SDLWindowSoftware::_generateWindowSurface()
{
	SDL_Surface* pWinSurf = SDL_GetWindowSurface(m_pSDLWindow);
	if (pWinSurf == nullptr)
	{
		printf("Unable to get window SDL Surface: %s\n", SDL_GetError());
		return nullptr;
	}

	PixelFormat format = PixelFormat::Undefined;

	switch (SDL_BYTESPERPIXEL(pWinSurf->format))
	{
		case 4:
			format = PixelFormat::XRGB_8;
			break;
		default:
		{
			printf("Unsupported pixelformat of SDL Surface!\n");
			return nullptr;
		}
	}

	Blob_p pCanvasBlob = Blob::create(pWinSurf->pixels, 0);
	auto pWindowSurface = SoftSurface::create( WGBP(Surface, _.size = {pWinSurf->w, pWinSurf->h}, _.format = format, _.canvas = true, _.scale = _scale() ), pCanvasBlob, pWinSurf->pitch);

	return pWindowSurface;
}
