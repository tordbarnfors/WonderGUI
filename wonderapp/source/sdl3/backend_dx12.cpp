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

#include <dx12_wrapper.h>

#include <wax_dx12surfacefactory.h>
#include <wax_dx12edgemapfactory.h>
#include <wax_dx12backend.h>

#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>


using namespace wg;
using namespace wax;


//____ SDLWindowDX12 __________________________________________________________
//
// Renders through a flip model swap chain on the window's HWND. The swap chain
// handling is the same as in the Win32 DX12 WonderApp, except that we present
// right after rendering, since SDL handles WM_PAINT for us.

class SDLWindowDX12 : public SDLWindow
{
public:
	SDLWindowDX12(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	virtual ~SDLWindowDX12();

	void	render() override;
	void 	onWindowSizeUpdated() override;

	void	releaseDevice();		// Lets go of everything on the device, so that it can be replaced.
	void	restoreDevice();		// Recreates the swap chain on the new device.

private:

	void	_createSwapChain(HWND hwnd, UINT width, UINT height);
	void	_createSwapChainBuffers();
	void	_dropSwapChainBuffers();

	void	_present();

	HWND				m_hwnd = nullptr;
	bool				m_bHidden = false;

	// Areas rendered since the last present, in pixels, presented as the swap
	// chain's dirty rects. DXGI copies everything outside them from the
	// previously presented buffer, which is what makes partial redraws work.
	// A full frame is presented instead whenever they can't be trusted.

	std::vector<RECT>	m_dirtyRects;
	bool				m_bPresentFullFrame = true;

	static const UINT								c_nbBuffers = 2;

	Microsoft::WRL::ComPtr<ID3D12Device>			m_pDX12Device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>			m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>			m_renderBuffers[c_nbBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE						m_rtvHandles[c_nbBuffers];

	// The format of the views, which is not the format of the swap chain buffers
	// as we ask for sRGB. See _createSwapChainBuffers().

	DXGI_FORMAT										m_rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	UINT m_currentBuffer = 0;		// Always read from GetCurrentBackBufferIndex(), never advanced by hand.

	UINT m_width = 0, m_height = 0;
	UINT m_heapIncrement = 0;
};


static DX12Wrapper*		g_pDX12Wrapper = nullptr;

static std::vector<SDLWindowDX12*>	s_windows;		// All our windows, to move them to a new device.

// A new device can't always be had right after the old one was lost - a reset
// GPU or an updated driver takes a moment to come back - so we try again at
// this interval until it works.

static const Uint64		c_recoveryRetryInterval = 1000;		// Milliseconds.
static Uint64			s_nextRecoveryAttempt = 0;

//____ _backend() _____________________________________________________________

static DX12Backend_p _backend()
{
	return wax_static_cast<DX12Backend_p>(wax_static_cast<GfxDeviceGen2_p>(Base::defaultDevice())->backend());
}

//____ backend_specific_init() ________________________________________________

bool backend_specific_init()
{
#ifndef NDEBUG
	DX12Wrapper::initDebugger();		// Validation layer is slow, keep it out of release builds.
#endif

	g_pDX12Wrapper = new DX12Wrapper();

	auto pBackend = DX12Backend::create(g_pDX12Wrapper->dx12Device(), g_pDX12Wrapper->renderCommandQueue());

	auto pGfxDevice = GfxDeviceGen2::create(pBackend);
	Base::setDefaultDevice(pGfxDevice);

	auto pSurfaceFactory = DX12SurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	Base::setDefaultEdgemapFactory(pBackend->edgemapFactory());
	Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());

	return true;
}

//____ backend_specific_exit() ________________________________________________

void backend_specific_exit()
{
	delete g_pDX12Wrapper;
	g_pDX12Wrapper = nullptr;

	DX12Wrapper::reportLiveObjects();
	DX12Wrapper::exitDebugger();
}

//____ _recoverDevice() _______________________________________________________
//
// Everything has to let go of the old device before D3D12 gives us a new one:
// the windows their swap chains, DX12Backend everything it and the surfaces
// and edgemaps have on it, and the wrapper the device itself. Then it all
// comes back on the new device.

static bool _recoverDevice()
{
	if (DX12Backend::device())
	{
		for (auto pWin : s_windows)
			pWin->releaseDevice();

		DX12Backend::setDevice(nullptr, nullptr);
		g_pDX12Wrapper->releaseDevice();
	}

	if (!g_pDX12Wrapper->createDevice())
		return false;

	if (!DX12Backend::setDevice(g_pDX12Wrapper->dx12Device(), g_pDX12Wrapper->renderCommandQueue()))
	{
		g_pDX12Wrapper->releaseDevice();
		return false;
	}

	for (auto pWin : s_windows)
		pWin->restoreDevice();

	return true;
}

//____ _simulateDeviceLoss() __________________________________________________
//
// Debug aid: set the environment variable WAPP_DX12_SIMULATE_DEVICE_LOSS to a
// number of seconds, and the device is removed that often, to test recovery.

#ifndef NDEBUG
static void _simulateDeviceLoss()
{
	static Uint64 interval = 0;
	static Uint64 nextLoss = 0;
	static bool bInitialized = false;

	if (!bInitialized)
	{
		bInitialized = true;

		const char * pValue = SDL_getenv("WAPP_DX12_SIMULATE_DEVICE_LOSS");
		if (pValue)
			interval = Uint64(atoi(pValue)) * 1000;

		nextLoss = SDL_GetTicks() + interval;
	}

	if (interval == 0 || DX12Backend::isDeviceLost() || SDL_GetTicks() < nextLoss)
		return;

	if (DX12Backend::simulateDeviceLoss())
		OutputDebugStringA("wapp: Simulating device loss\n");

	nextLoss = SDL_GetTicks() + interval;
}
#endif

//____ backend_specific_update() ______________________________________________

void backend_specific_update()
{
#ifndef NDEBUG
	_simulateDeviceLoss();
#endif

	// DX12Backend notices a lost device on its own, but only once it tries to
	// use it. Asking every frame catches it before we render into it.

	if (!DX12Backend::checkDeviceLost())
		return;

	Uint64 now = SDL_GetTicks();
	if (now < s_nextRecoveryAttempt)
		return;

	if (_recoverDevice())
	{
		OutputDebugStringA("wapp: Recovered from device loss\n");
		s_nextRecoveryAttempt = 0;
	}
	else
	{
		OutputDebugStringA("wapp: Failed to recover from device loss, trying again shortly\n");
		s_nextRecoveryAttempt = now + c_recoveryRetryInterval;
	}
}

//____ create_backend_specific_window() ________________________________________

SDLWindow * create_backend_specific_window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new SDLWindowDX12(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ SDLWindowDX12() ________________________________________________________

SDLWindowDX12::SDLWindowDX12(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	s_windows.push_back(this);

	if (!_createSDLWindow(title, origin, pos, size, 0, resizable, open))
		return;

	HWND hwnd = (HWND) SDL_GetPointerProperty(SDL_GetWindowProperties(m_pSDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	if (!hwnd)
	{
		printf("Unable to get HWND of SDL window: %s\n", SDL_GetError());
		return;
	}

	m_hwnd = hwnd;

	SizeI pixels = _sizeInPixels();
	m_width = pixels.w;
	m_height = pixels.h;

	_createSwapChain(hwnd, m_width, m_height);
	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), m_rtvFormat, { spx(m_width * 64), spx(m_height * 64) }, _scale());
	m_pRootPanel = RootPanel::create(CanvasRef::Default, Base::defaultDevice());

	//TODO: This is ugly. It should be handled when windows gets focused.

	Base::inputHandler()->setFocusedWindow(m_pRootPanel);
}

//____ ~SDLWindowDX12() _______________________________________________________

SDLWindowDX12::~SDLWindowDX12()
{
	s_windows.erase(std::remove(s_windows.begin(), s_windows.end(), this), s_windows.end());

	// The GPU might still be working on (or presenting) our swap chain buffers.

	if (g_pDX12Wrapper)
		g_pDX12Wrapper->flushRenderQueue();
}

//____ render() _______________________________________________________________

void SDLWindowDX12::render()
{
	if (m_bHidden || !m_pRootPanel || !m_pSwapChain)
		return;						// No swap chain while the device is being replaced.

	// A flip-model swap chain decides which buffer is the back buffer. Ask it
	// every frame instead of keeping our own count.

	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), m_rtvFormat, {spx(m_width * 64), spx(m_height * 64)}, m_pRootPanel->scale());

	m_pRootPanel->render();

	int nRects = m_pRootPanel->nbUpdatedRects();
	auto pRects = m_pRootPanel->firstUpdatedRect();

	for (int i = 0; i < nRects; i++)
	{
		RectI rect = * pRects++ / 64;
		RECT rc;
		rc.left = rect.x;
		rc.top = rect.y;
		rc.right = rect.x + rect.w;
		rc.bottom = rect.y + rect.h;

		m_dirtyRects.push_back(rc);
	}

	if (nRects > 0)
		_present();
}

//____ _present() _____________________________________________________________

void SDLWindowDX12::_present()
{
	// Dirty rects must be the areas we have actually rendered, and every pixel
	// in them must be up to date - DXGI keeps the rest of the previously
	// presented frame. The first frame on a new set of buffers (after creation
	// or a resize) has to be presented in full.

	bool bFullFrame = m_bPresentFullFrame;

	// Dirty rects have to be inside the back buffer. Not using std::min/max
	// here, windows.h has them as macros.

	for (auto& rect : m_dirtyRects)
	{
		if (rect.left < 0)
			rect.left = 0;
		if (rect.top < 0)
			rect.top = 0;
		if (rect.right > (LONG)m_width)
			rect.right = (LONG)m_width;
		if (rect.bottom > (LONG)m_height)
			rect.bottom = (LONG)m_height;
	}

	m_dirtyRects.erase(std::remove_if(m_dirtyRects.begin(), m_dirtyRects.end(),
		[](const RECT& rect) { return rect.right <= rect.left || rect.bottom <= rect.top; }),
		m_dirtyRects.end());

	if (m_dirtyRects.empty())
		bFullFrame = true;

	DXGI_PRESENT_PARAMETERS presentParams = {};

	if (!bFullFrame)
	{
		presentParams.DirtyRectsCount = (UINT) m_dirtyRects.size();
		presentParams.pDirtyRects = m_dirtyRects.data();
	}

	HRESULT hr = m_pSwapChain->Present1(0, 0, &presentParams);

	m_dirtyRects.clear();
	m_bPresentFullFrame = false;

	// A lost device is reported by DX12Backend and recovered from by
	// backend_specific_update() before the next frame.

	if (FAILED(hr) && !DX12Backend::handleDeviceLoss(hr))
	{
		char buffer[128];
		sprintf_s(buffer, "Present1() failed, HRESULT = 0x%08lX", (unsigned long)hr);

		Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, nullptr, nullptr, __func__, __FILE__, __LINE__);
	}
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindowDX12::onWindowSizeUpdated()
{
	// Without a swap chain the device is being replaced, and restoreDevice()
	// picks up the size.

	if (!m_pSwapChain || !m_pRootPanel)
		return;

	SizeI pixels = _sizeInPixels();

	// A minimized window has a zero-sized client area, which ResizeBuffers()
	// doesn't accept. Keep the old buffers until we get a real size again.

	if (pixels.w == 0 || pixels.h == 0)
	{
		m_bHidden = true;
		return;
	}

	m_width = pixels.w;
	m_height = pixels.h;

	// The new buffers hold nothing, and the old rects belong to a canvas that no
	// longer exists, so the next present has to be a full frame.

	m_dirtyRects.clear();
	m_bPresentFullFrame = true;

	// All references to the buffers must be gone and the GPU done with them
	// before ResizeBuffers(). Waiting for DX12Backend alone is not enough,
	// DXGI queues its own present work on the same command queue.

	g_pDX12Wrapper->flushRenderQueue();

	_dropSwapChainBuffers();
	HRESULT hr = m_pSwapChain->ResizeBuffers(0, m_width, m_height, DXGI_FORMAT_UNKNOWN, 0);	// Flags must match those used at creation.

	// On a lost device we go on without buffers. Nothing renders until
	// backend_specific_update() has replaced the device and restoreDevice() the swap chain.

	if (SUCCEEDED(hr))
		_createSwapChainBuffers();
	else if (!DX12Backend::handleDeviceLoss(hr))
		assert(false);

	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

	m_bHidden = false;

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), m_rtvFormat, { spx(m_width * 64), spx(m_height * 64) }, _scale());
	m_pRootPanel->setCanvas(CanvasRef::Default);

	_notifyUserWindowOfResize();
	render();
}

//____ releaseDevice() _______________________________________________________

void SDLWindowDX12::releaseDevice()
{
	// The swap chain holds the device through the render queue it was created
	// on, and D3D12 won't give us a new device until the old one is gone.

	_dropSwapChainBuffers();
	m_pSwapChain.Reset();
	m_RTVHeap.Reset();
	m_pDX12Device.Reset();

	m_dirtyRects.clear();
}

//____ restoreDevice() _______________________________________________________

void SDLWindowDX12::restoreDevice()
{
	if (!m_hwnd || m_pSwapChain)
		return;

	// The window may have changed size while it had no swap chain.

	SizeI pixels = _sizeInPixels();

	bool bResized = pixels.w > 0 && pixels.h > 0 && (UINT(pixels.w) != m_width || UINT(pixels.h) != m_height);

	if (bResized)
	{
		m_width = pixels.w;
		m_height = pixels.h;
		m_bHidden = false;
	}

	_createSwapChain(m_hwnd, m_width, m_height);
	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), m_rtvFormat, { spx(m_width * 64), spx(m_height * 64) }, _scale());

	// Nothing of what was on screen is in the new buffers.

	m_bPresentFullFrame = true;

	if (m_pRootPanel)
	{
		m_pRootPanel->setCanvas(CanvasRef::Default);
		m_pRootPanel->addDirtyPatch({ 0, 0, spx(m_width * 64), spx(m_height * 64) });
	}

	if (bResized)
		_notifyUserWindowOfResize();
}

//____ _createSwapChain() ______________________________________________________

void SDLWindowDX12::_createSwapChain(HWND hwnd, UINT width, UINT height)
{
	ID3D12Device* pDevice = g_pDX12Wrapper->dx12Device();
	IDXGIFactory2* pFactory = g_pDX12Wrapper->dxgiFactory();
	ID3D12CommandQueue* pCommandQueue = g_pDX12Wrapper->renderCommandQueue();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NumDescriptors = c_nbBuffers;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	if (S_OK != pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf())))
		assert(false);

	m_heapIncrement = pDevice->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);

	DXGI_SWAP_CHAIN_DESC1 description = {};
	description.Width = width;
	description.Height = height;
	description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.Stereo = false;
	description.SampleDesc = { 1,0 };
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = c_nbBuffers;
	description.Scaling = DXGI_SCALING_NONE;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	description.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> pSwapChain1;
	if (S_OK != pFactory->CreateSwapChainForHwnd(pCommandQueue, hwnd, &description, nullptr, nullptr, &pSwapChain1))
		assert(false);

	if (S_OK != pSwapChain1.As(&m_pSwapChain))		// IDXGISwapChain3 needed for GetCurrentBackBufferIndex().
		assert(false);

	// We handle resizing ourselves and have no fullscreen support, so keep DXGI
	// from acting on Alt+Enter or changing the window.

	pFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

	m_pDX12Device = pDevice;

	_createSwapChainBuffers();
}

//____ _createSwapChainBuffers() _________________________________________________

void SDLWindowDX12::_createSwapChainBuffers()
{
	// The shaders work in linear, so the hardware has to encode to sRGB on the way
	// into the buffer. A flip model swap chain can't have an sRGB format of its own,
	// but a view with one over a plain buffer is allowed.

	m_rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = m_rtvFormat;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < c_nbBuffers; i++)
	{
		m_rtvHandles[i] = rtvHandle;

		m_renderBuffers[i].Reset();

		if (S_OK != m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderBuffers[i])))
			assert(false);
		m_pDX12Device->CreateRenderTargetView(m_renderBuffers[i].Get(), &rtvDesc, rtvHandle);
		rtvHandle.ptr += m_heapIncrement;
	}
}

//____ _dropSwapChainBuffers() ____________________________________________________

void SDLWindowDX12::_dropSwapChainBuffers()
{
	for (UINT i = 0; i < c_nbBuffers; i++)
		m_renderBuffers[i].Reset();
}
