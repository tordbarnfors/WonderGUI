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

#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_gledgemapfactory.h>
#include <wg_glbackend.h>

#ifndef __APPLE__
#   include <GL/glew.h>
#endif


using namespace wg;


class SDLWindowGL : public SDLWindow
{
public:
	SDLWindowGL(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowGL() {};

	void	render() override;
	void 	onWindowSizeUpdated( int w, int h ) override;

private:

	void	_refreshScale();

	int		m_scale = 64;
};


static SDL_Window*		g_pDummyWindow = nullptr;
static SDL_GLContext	g_glContext;

GlBackend_p				g_pBackend;

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	g_pDummyWindow = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	g_glContext = SDL_GL_CreateContext(g_pDummyWindow);

#if defined(_WIN32) || defined(__linux__)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		auto* pError = glewGetErrorString(err);
		return false;
	}

#endif

	g_pBackend = GlBackend::create();

	auto pGlDevice = GfxDeviceGen2::create(g_pBackend);

	Base::setDefaultGfxDevice(pGlDevice);

	auto pSurfaceFactory = GlSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = GlEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);




	return true;
}

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{
	g_pBackend = nullptr;

	SDL_GL_DeleteContext(g_glContext);
	SDL_DestroyWindow(g_pDummyWindow);
}

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowGL(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ SDLWindowGL() ___________________________________________________________

SDLWindowGL::SDLWindowGL(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;

	Rect geo = { pos, size };

	m_pSDLWindow = SDL_CreateWindow(title.c_str(), geo.x, geo.y, geo.w, geo.h, flags);
	if (m_pSDLWindow == NULL)
		return;

	_refreshScale();

#ifdef _WIN32
	g_pBackend->setDefaultCanvas({ spx(size.w * 64), spx(size.h * 64) }, m_scale);
#else
	g_pBackend->setDefaultCanvas({ spx(size.w * m_scale), spx(size.h * m_scale) }, m_scale);
#endif

	m_pRootPanel = RootPanel::create(CanvasRef::Default);

//	pRootPanel->setWindowRef((uintptr_t) pWindow.rawPtr());

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);

}

//____ render() _______________________________________________________________

void SDLWindowGL::render()
{
	int nRects = m_pRootPanel->nbDirtyRects();
	if (nRects == 0)
		return;

	SDL_GL_MakeCurrent(m_pSDLWindow, g_glContext);

	g_pBackend->setDefaultCanvas( m_pRootPanel->canvasSize(), m_scale);
	m_pRootPanel->render();


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

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowGL::onWindowSizeUpdated( int w, int h )
{
	_refreshScale();

#ifdef _WIN32
	g_pBackend->setDefaultCanvas({ spx(w * 64), spx(h * 64) }, m_scale);
#else
	g_pBackend->setDefaultCanvas({ spx(w * m_scale), spx(h * m_scale) }, m_scale);
#endif

	m_pRootPanel->setCanvas(CanvasRef::Default);
    render();
}

//____ _refershScale() ____________________________________________________________

void SDLWindowGL::_refreshScale()
{
#ifdef _WIN32

	int displayIndex = SDL_GetWindowDisplayIndex(m_pSDLWindow);
	float ddpi;
	if (SDL_GetDisplayDPI(displayIndex, &ddpi, nullptr, nullptr) == 0) 
		m_scale = 64 * ddpi / 96.0f;
	else
		m_scale = 64;

#else

	int windowWidth, windowHeight;
	int drawableWidth, drawableHeight;

	SDL_GetWindowSize(m_pSDLWindow, &windowWidth, &windowHeight);
	SDL_GL_GetDrawableSize(m_pSDLWindow, &drawableWidth, &drawableHeight);
	m_scale = 64 * drawableWidth / (float)windowWidth;

#endif
}
