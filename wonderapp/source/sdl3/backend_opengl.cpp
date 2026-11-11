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


//____ SDLWindowGL ____________________________________________________________
//
// Wax renders into a surface of our own, which keeps what was rendered between
// frames, as only the dirty rects are drawn. Each frame that rendered anything is
// copied to the back buffer of the window and swapped in. Drawing directly into
// the front buffer of the window shows nothing on Wayland, where a window isn't
// even mapped until it has swapped in a buffer.

class SDLWindowGL : public SDLWindow
{
public:
	SDLWindowGL(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowGL() {};

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	void	_createCanvas();
	void	_present();

	GlSurface_p	m_pCanvas;
	bool		m_bPresentPending = false;		// Rendered while hidden, so not yet swapped in.
};


static SDL_Window*		g_pDummyWindow = nullptr;
static SDL_GLContext	g_glContext = nullptr;
static GLuint			g_readFramebuffer = 0;		// Reads the canvas of a window when presenting it.

static GlBackend_p		g_pBackend;

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
	// A GLEW built for GLX fails with GLEW_ERROR_NO_GLX_DISPLAY under Wayland, but only
	// after the OpenGL entry points are loaded, so that error is fine.

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err && GLEW_ERROR_NO_GLX_DISPLAY != err)
	{
		printf("Unable to initialize GLEW: %s\n", (const char*) glewGetErrorString(err));
		return false;
	}
#endif

	glGenFramebuffers(1, &g_readFramebuffer);

	g_pBackend = GlBackend::create();

	auto pGlDevice = GfxDeviceGen2::create(g_pBackend);

	wg::Base::setDefaultDevice(pGlDevice);

	auto pSurfaceFactory = GlSurfaceFactory::create();
	wg::Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = GlEdgemapFactory::create();
	wg::Base::setDefaultEdgemapFactory(pEdgemapFactory);

	wg::Base::setDefaultWarpmapFactory(g_pBackend->warpmapFactory());

	return true;
}

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{
	g_pBackend = nullptr;

	glDeleteFramebuffers(1, &g_readFramebuffer);

	SDL_GL_DestroyContext(g_glContext);
	SDL_DestroyWindow(g_pDummyWindow);
}

//____ backend_specific_update() ______________________________________________

void backend_specific_update()
{
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

	// Swapping doesn't wait for the display, the main loop sets the pace.

	SDL_GL_SetSwapInterval(0);

	_createCanvas();
	m_pRootPanel = RootPanel::create(m_pCanvas);

	//TODO: This is ugly. It should be handled when windows gets focused.

	wg::Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ render() _______________________________________________________________

void SDLWindowGL::render()
{
	if (!m_pRootPanel)
		return;

	if (m_pRootPanel->nbDirtyRects() > 0)
	{
		SDL_GL_MakeCurrent(m_pSDLWindow, g_glContext);
		m_pRootPanel->render();
		m_bPresentPending = true;
	}

	// A hidden window can't be swapped in, so what was rendered waits until it is shown.

	if (m_bPresentPending && !(SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_HIDDEN))
	{
		_present();
		m_bPresentPending = false;
	}
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowGL::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	_createCanvas();

	m_pRootPanel->setCanvas(m_pCanvas);
	_notifyUserWindowOfResize();
    render();
}

//____ _createCanvas() ________________________________________________________

void SDLWindowGL::_createCanvas()
{
	m_pCanvas = GlSurface::create( WGBP(Surface, _.canvas = true, _.format = PixelFormat::ARGB_8, _.scale = _scale(), _.size = _sizeInPixels() ) );
}

//____ _present() _____________________________________________________________
//
// Surfaces are upside down to GL, so the canvas is flipped when copied. The bytes
// are copied as they are, so the window gets sRGB whether it is sRGB-capable or not,
// and is made opaque.

void SDLWindowGL::_present()
{
	SDL_GL_MakeCurrent(m_pSDLWindow, g_glContext);

	SizeI size = m_pCanvas->pixelSize();

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_FRAMEBUFFER_SRGB);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, g_readFramebuffer);
	glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_pCanvas->getTexture(), 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, size.w, size.h, 0, size.h, size.w, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	SDL_GL_SwapWindow(m_pSDLWindow);
}
