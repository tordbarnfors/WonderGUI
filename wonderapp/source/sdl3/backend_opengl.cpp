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

#include <wax_glsurface.h>
#include <wax_glsurfacefactory.h>
#include <wax_gledgemapfactory.h>
#include <wax_glbackend.h>

#ifndef __APPLE__
#   include <GL/glew.h>
#endif


using namespace wg;
using namespace wax;


class SDLWindowGL : public SDLWindow
{
public:
	SDLWindowGL(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowGL() {};

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	void	_setDefaultCanvas();
};


static SDL_Window*		g_pDummyWindow = nullptr;
static SDL_GLContext	g_glContext = nullptr;

static GlBackend_p		g_pBackend;

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);

	// The context is created on a hidden window, so it outlives the app's windows.

	g_pDummyWindow = SDL_CreateWindow("", 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (!g_pDummyWindow)
	{
		printf("Unable to create window for OpenGL context: %s\n", SDL_GetError());
		return false;
	}

	g_glContext = SDL_GL_CreateContext(g_pDummyWindow);
	if (!g_glContext)
	{
		printf("Unable to create OpenGL context: %s\n", SDL_GetError());
		return false;
	}

#if defined(_WIN32) || defined(__linux__)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Unable to initialize GLEW: %s\n", (const char*) glewGetErrorString(err));
		return false;
	}
#endif

	g_pBackend = GlBackend::create();

	auto pGlDevice = GfxDeviceGen2::create(g_pBackend);

	Base::setDefaultDevice(pGlDevice);

	auto pSurfaceFactory = GlSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = GlEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);

	Base::setDefaultWarpmapFactory(g_pBackend->warpmapFactory());

	return true;
}

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{
	g_pBackend = nullptr;

	SDL_GL_DestroyContext(g_glContext);
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

	if (!_createSDLWindow(title, origin, pos, size, SDL_WINDOW_OPENGL, resizable, open))
		return;

	SDL_GL_MakeCurrent(m_pSDLWindow, g_glContext);

	_setDefaultCanvas();
	m_pRootPanel = RootPanel::create(CanvasRef::Default);

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ render() _______________________________________________________________

void SDLWindowGL::render()
{
	if (!m_pRootPanel)
		return;

	int nRects = m_pRootPanel->nbDirtyRects();
	if (nRects == 0)
		return;

	SDL_GL_MakeCurrent(m_pSDLWindow, g_glContext);

	g_pBackend->setDefaultCanvas( m_pRootPanel->canvasSize(), m_pRootPanel->scale() );
	m_pRootPanel->render();
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowGL::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	_setDefaultCanvas();

	m_pRootPanel->setCanvas(CanvasRef::Default);
	_notifyUserWindowOfResize();
    render();
}

//____ _setDefaultCanvas() ____________________________________________________

void SDLWindowGL::_setDefaultCanvas()
{
	SizeI pixels = _sizeInPixels();
	g_pBackend->setDefaultCanvas({ spx(pixels.w * 64), spx(pixels.h * 64) }, _scale());
}
