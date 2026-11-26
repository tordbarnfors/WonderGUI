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

#include <wax_sdlgpubackend.h>
#include <wax_sdlgpusurface.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>


using namespace wg;
using namespace wax;


//____ SDLWindowGPU ___________________________________________________________
//
// Renders with the GPU API of SDL, which picks Vulkan, Direct3D 12 or Metal.
//
// Wax renders into a surface of our own, which keeps what was rendered between
// frames, as only the dirty rects are drawn. Each frame that rendered anything is
// copied to a texture of the window's swap chain and presented, since those don't
// keep their content.
//
// The swap chain is sRGB where the device has it, so that copying our sRGB canvas
// to it gives the same bytes. Without it the window gets the canvas linear, which
// is too dark.
//
// Environment variables:
//
//   SDL_GPU_DRIVER=vulkan|direct3d12|metal	Picks the driver, which SDL otherwise does.
//   WAPP_GPU_DEBUG=1						Debug mode of SDL_GPU, with the validation
//											layers of the driver where installed.
//   WAPP_GPU_DIRECT=1						Draws straight into the swap chain, see below.
//
// With WAPP_GPU_DIRECT=1 there is no canvas of our own. Each frame that has something to draw
// acquires a texture of the swap chain, clears it, where an application with graphics of its own
// would render them, and has Wax draw all widgets on top of that in the same command buffer, see
// SdlGpuBackend::setCommandBuffer(). That is how an application that owns the frame uses the
// backend, and this is here to have it run and tested.
//
// A window that can't be shown with the device, as on a display the GPU can't
// present to, is still rendered into, but shows nothing. That is enough for tests
// that read back what was rendered, like gfxdevicetester --accuracy.

class SDLWindowGPU : public SDLWindow
{
public:
	SDLWindowGPU(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowGPU() {};

	void	render() override;
	void 	onWindowSizeUpdated() override;
	void	onExposed() override { m_bPresentPending = true; }		// The canvas still has it all.

protected:

	void	destroy() override;

private:

	void	_createCanvas();
	bool	_present();
	bool	_renderDirect();

	SdlGpuSurface_p	m_pCanvas;						// Null when drawing straight into the swap chain.
	bool			m_bDirect = false;				// Draws straight into the swap chain.
	bool			m_bClaimed = false;				// Has a swap chain of the device.
	bool			m_bPresentPending = false;		// Rendered but not yet presented.
};


static SDL_GPUDevice *	g_pDevice = nullptr;
static SdlGpuBackend_p	g_pBackend;
static bool				g_bDirect = false;				// WAPP_GPU_DIRECT=1

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
	const char * pDebug = getenv("WAPP_GPU_DEBUG");
	bool bDebug = pDebug && strcmp(pDebug, "1") == 0;

	const char * pDirect = getenv("WAPP_GPU_DIRECT");
	g_bDirect = pDirect && strcmp(pDirect, "1") == 0;

	g_pDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, bDebug, nullptr);
	if (!g_pDevice)
	{
		printf("Unable to create SDL GPU device: %s\n", SDL_GetError());
		return false;
	}

	if (!SdlGpuBackend::setDevice(g_pDevice))
		return false;

	g_pBackend = SdlGpuBackend::create();
	if (!g_pBackend)
		return false;

	auto pDevice = GfxDeviceGen2::create(g_pBackend);
	wg::Base::setDefaultDevice(pDevice);

	wg::Base::setDefaultSurfaceFactory(g_pBackend->surfaceFactory());
	wg::Base::setDefaultEdgemapFactory(g_pBackend->edgemapFactory());
	wg::Base::setDefaultWarpmapFactory(g_pBackend->warpmapFactory());

	return true;
}

//____ backend_specific_exit() ________________________________________________
//
// Surfaces and such of the app may outlive us, in which case the device is left
// for the process to clean up.

void backend_specific_exit()
{
	g_pBackend = nullptr;

	if (!g_pDevice)
		return;

	SDL_WaitForGPUIdle(g_pDevice);

	if (!SdlGpuBackend::setDevice(nullptr))
		return;

	SDL_DestroyGPUDevice(g_pDevice);
	g_pDevice = nullptr;
}

//____ backend_specific_update() ______________________________________________

void backend_specific_update()
{
}

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowGPU(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ SDLWindowGPU() _________________________________________________________

SDLWindowGPU::SDLWindowGPU(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	if (!_createSDLWindow(title, origin, pos, size, 0, resizable, open))
		return;

	m_bClaimed = SDL_ClaimWindowForGPUDevice(g_pDevice, m_pSDLWindow);

	if (m_bClaimed)
	{
		// Presenting doesn't wait for the display, the main loop sets the pace. A swap chain
		// texture that isn't free yet is waited for by presenting later.

		if (SDL_WindowSupportsGPUSwapchainComposition(g_pDevice, m_pSDLWindow, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR))
			SDL_SetGPUSwapchainParameters(g_pDevice, m_pSDLWindow, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_VSYNC);
		else
			printf("The window has no sRGB swap chain, it will look too dark.\n");
	}
	else
		printf("Unable to show a window with the SDL GPU device, it is rendered into but shows nothing: %s\n", SDL_GetError());

	// Without a swap chain there is nothing to draw straight into.

	m_bDirect = g_bDirect && m_bClaimed;

	_createCanvas();

	if (m_bDirect)
		m_pRootPanel = RootPanel::create(CanvasRef::Default, wg::Base::defaultDevice());
	else
		m_pRootPanel = RootPanel::create(m_pCanvas);

	//TODO: This is ugly. It should be handled when windows gets focused.

	wg::Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ destroy() ______________________________________________________________

void SDLWindowGPU::destroy()
{
	if (m_bClaimed && g_pDevice)
		SDL_ReleaseWindowFromGPUDevice(g_pDevice, m_pSDLWindow);

	m_bClaimed = false;
	SDLWindow::destroy();
}

//____ render() _______________________________________________________________

void SDLWindowGPU::render()
{
	if (!m_pRootPanel)
		return;

	bool bHidden = (SDL_GetWindowFlags(m_pSDLWindow) & SDL_WINDOW_HIDDEN) != 0;

	if (m_bDirect)
	{
		// Nothing is kept between frames, so a frame is drawn in full when anything has changed,
		// once there is a texture of the swap chain to draw it into.

		if (m_pRootPanel->nbDirtyRects() > 0)
			m_bPresentPending = true;

		if (m_bPresentPending && !bHidden)
			m_bPresentPending = !_renderDirect();

		return;
	}

	if (m_pRootPanel->nbDirtyRects() > 0)
	{
		m_pRootPanel->render();
		m_bPresentPending = true;
	}

	// A hidden window has nothing to present to, so what was rendered waits until it is
	// shown, and so does what finds no swap chain texture free.

	if (m_bPresentPending && !bHidden)
		m_bPresentPending = !_present();
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowGPU::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	_createCanvas();

	if (m_bDirect)
		m_pRootPanel->setCanvas(CanvasRef::Default);
	else
		m_pRootPanel->setCanvas(m_pCanvas);

	_notifyUserWindowOfResize();
	render();
}

//____ _createCanvas() ________________________________________________________

//
// When drawing straight into the swap chain, the default canvas of the backend gets the size and
// format of the window, for the root panel to read. Its texture is set for each frame.

void SDLWindowGPU::_createCanvas()
{
	if (m_bDirect)
	{
		SizeI size = _sizeInPixels();

		g_pBackend->setDefaultCanvas(nullptr, SDL_GetGPUSwapchainTextureFormat(g_pDevice, m_pSDLWindow),
									 { spx(size.w * 64), spx(size.h * 64) }, _scale());
		return;
	}

	m_pCanvas = SdlGpuSurface::create( WGBP(Surface, _.canvas = true, _.format = PixelFormat::ARGB_8, _.scale = _scale(), _.size = _sizeInPixels() ) );
}

//____ _present() _____________________________________________________________
//
// Copies the canvas to the next texture of the swap chain, after Wax's rendering,
// which has been submitted before. Returns false if it has to be done again, as
// when there was no texture free.

bool SDLWindowGPU::_present()
{
	if (!m_bClaimed || !m_pCanvas || !m_pCanvas->texture())
		return true;

	SDL_GPUCommandBuffer * pCmd = SDL_AcquireGPUCommandBuffer(g_pDevice);
	if (!pCmd)
		return true;

	SDL_GPUTexture * pTarget = nullptr;
	Uint32 width = 0, height = 0;

	if (!SDL_AcquireGPUSwapchainTexture(pCmd, m_pSDLWindow, &pTarget, &width, &height))
	{
		SDL_CancelGPUCommandBuffer(pCmd);
		return true;
	}

	if (pTarget)
	{
		SizeI size = m_pCanvas->pixelSize();

		SDL_GPUBlitInfo blit = {};
		blit.source.texture = m_pCanvas->texture();
		blit.source.w = Uint32(size.w);
		blit.source.h = Uint32(size.h);
		blit.destination.texture = pTarget;
		blit.destination.w = width;
		blit.destination.h = height;
		blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
		blit.filter = SDL_GPU_FILTER_NEAREST;

		SDL_BlitGPUTexture(pCmd, &blit);
	}

	SDL_SubmitGPUCommandBuffer(pCmd);
	return pTarget != nullptr;
}

//____ _renderDirect() ________________________________________________________
//
// Draws a frame straight into the next texture of the swap chain: what the application has of its
// own, here only a clear, and then all widgets on top of it, in the command buffer the texture was
// acquired with. Returns false if it has to be done again, as when there was no texture free.

bool SDLWindowGPU::_renderDirect()
{
	SDL_GPUCommandBuffer * pCmd = SDL_AcquireGPUCommandBuffer(g_pDevice);
	if (!pCmd)
		return true;

	SDL_GPUTexture * pTarget = nullptr;
	Uint32 width = 0, height = 0;

	if (!SDL_AcquireGPUSwapchainTexture(pCmd, m_pSDLWindow, &pTarget, &width, &height))
	{
		SDL_CancelGPUCommandBuffer(pCmd);
		return true;
	}

	if (pTarget)
	{
		SDL_GPUColorTargetInfo target = {};
		target.texture = pTarget;
		target.clear_color = { 0.f, 0.f, 0.f, 1.f };
		target.load_op = SDL_GPU_LOADOP_CLEAR;
		target.store_op = SDL_GPU_STOREOP_STORE;

		SDL_EndGPURenderPass(SDL_BeginGPURenderPass(pCmd, &target, 1, nullptr));

		// The swap chain decides the size of the frame, which the window follows.

		SizeSPX size = { spx(width * 64), spx(height * 64) };
		SDL_GPUTextureFormat format = SDL_GetGPUSwapchainTextureFormat(g_pDevice, m_pSDLWindow);

		g_pBackend->setDefaultCanvas(pTarget, format, size, _scale());
		g_pBackend->setCommandBuffer(pCmd);

		if (m_pRootPanel->canvasSize() != size)
			m_pRootPanel->setCanvas(CanvasRef::Default);

		m_pRootPanel->addDirtyPatch(size);
		m_pRootPanel->render();

		// The texture is gone once the command buffer is submitted.

		g_pBackend->setCommandBuffer(nullptr);
		g_pBackend->setDefaultCanvas(nullptr, format, size, _scale());
	}

	SDL_SubmitGPUCommandBuffer(pCmd);
	return pTarget != nullptr;
}
