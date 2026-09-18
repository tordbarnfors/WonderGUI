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

#include <win32window.h>
#include <windows.h>
#include <dx12_wrapper.h>

#include <wg_gfxdevice.h>
#include <wg_dx12backend.h>

#include <cstdio>
#include <vector>
#include <algorithm>


using namespace wg;

extern DX12Wrapper* g_pDX12Wrapper;
extern std::wstring _stringToWString(const std::string& str);

// Set to true together with DX12Backend's c_bDebugClearUpdateRects: that clear
// paints the whole update rect, which a partial present isn't allowed to do.

static constexpr bool c_bPresentFullFrames = false;

//____ _backend() _____________________________________________________

static DX12Backend_p _backend()
{
	return wg_static_cast<DX12Backend_p>(wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice())->backend());
}


//____ constructor ___________________________________________________

Win32Window::Win32Window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	RectI pixelGeo = { int(pos.x), int(pos.y), int(size.w), int(size.h) };

	m_windowHandle = CreateWindow("WappWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW, pixelGeo.x, pixelGeo.y, pixelGeo.w, pixelGeo.h, 0, 0, 0, this);

	if (!m_windowHandle)
	{
		int x = 0;
		//TODO: Error handling!
	}
	else
	{
		// The size passed to CreateWindow() includes borders and title bar, the
		// swap chain should match the client area.

		RECT clientRect;
		GetClientRect(m_windowHandle, &clientRect);
		m_width = clientRect.right - clientRect.left;
		m_height = clientRect.bottom - clientRect.top;

		_createSwapChain(g_pDX12Wrapper, m_windowHandle, m_width, m_height);
		m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

		UINT dpi = GetDpiForWindow(m_windowHandle);
		int scale = dpi * 64 / 96; // 96 DPI is 100% scaling

		_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), { spx(m_width * 64), spx(m_height * 64) },scale);
		m_pRootPanel = RootPanel::create(CanvasRef::Default, Base::defaultGfxDevice());
		assert(m_pRootPanel);


		// Show window if open is true

		if (open)
		{
			ShowWindow(m_windowHandle, SW_SHOW);
			UpdateWindow(m_windowHandle);
		}
	}
}

//____ destructor _____________________________________________________

Win32Window::~Win32Window()
{
	// The GPU might still be working on (or presenting) our swap chain buffers.

	if (g_pDX12Wrapper)
		g_pDX12Wrapper->flushRenderQueue();
}


//____ render() _______________________________________________________________

void Win32Window::render()
{
	if (m_bHidden || !m_pRootPanel)
		return;

	// A flip-model swap chain decides which buffer is the back buffer. Ask it
	// every frame instead of keeping our own count, which drifts out of sync as
	// soon as a Present() isn't paired with exactly one render.

	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();
	m_bRendered = true;

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), {spx(m_width * 64), spx(m_height * 64)}, m_pRootPanel->scale());

//	m_pRootPanel->addDirtyPatch({ 0,0, spx(m_width * 64), spx(m_height * 64) });

	m_pRootPanel->render();

	// Collect what was rendered, both to get a WM_PAINT and to present the areas
	// as the swap chain's dirty rects. They accumulate until we present, since
	// several render() calls can share a back buffer.

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
		InvalidateRect(m_windowHandle, &rc, FALSE);
	}
}

//____ _dirtyRectsCoverRegion() _______________________________________________
//
// True if the rects we have rendered cover everything Windows wants repainted.
// If they don't, we would be telling DXGI that areas are up to date when they
// are not, and it would keep whatever the previously presented buffer had there.

bool Win32Window::_dirtyRectsCoverRegion(HRGN updateRegion) const
{
	if (m_dirtyRects.empty())
		return false;

	HRGN rendered = CreateRectRgn(0, 0, 0, 0);

	for (auto& rect : m_dirtyRects)
	{
		HRGN rectRegion = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
		CombineRgn(rendered, rendered, rectRegion, RGN_OR);
		DeleteObject(rectRegion);
	}

	HRGN notRendered = CreateRectRgn(0, 0, 0, 0);
	int result = CombineRgn(notRendered, updateRegion, rendered, RGN_DIFF);

	DeleteObject(rendered);
	DeleteObject(notRendered);

	return result == NULLREGION;
}

//____ paint() ________________________________________________________________

void Win32Window::paint()
{
	// UpdateWindow() in the constructor sends WM_PAINT before anything has been
	// rendered. Don't present an undefined buffer.

	if (!m_bRendered)
	{
		ValidateRect(m_windowHandle, nullptr);
		return;
	}

	HRGN updateRegion = CreateRectRgn(0, 0, 0, 0);

	if (GetUpdateRgn(m_windowHandle, updateRegion, FALSE) != NULLREGION)
	{
		// Dirty rects must be the areas we have actually rendered, and every pixel
		// in them must be up to date - DXGI keeps the rest of the previously
		// presented frame. Present the whole frame if we can't promise that, which
		// is the case for the first frame on a new set of buffers (after creation
		// or a resize), or if Windows wants an area repainted that WonderGUI
		// didn't render.

		bool bFullFrame = m_bPresentFullFrame || c_bPresentFullFrames || !_dirtyRectsCoverRegion(updateRegion);

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

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			char buffer[128];
			sprintf_s(buffer, "Device lost on Present1(), GetDeviceRemovedReason() = 0x%08lX",
				(unsigned long)m_pDX12Device->GetDeviceRemovedReason());

			Base::throwError(ErrorLevel::Critical, ErrorCode::RenderFailure, buffer, nullptr, nullptr, __func__, __FILE__, __LINE__);
		}
	}

	DeleteObject(updateRegion);
	ValidateRect(m_windowHandle, nullptr);
}


//____ onResize() ____________________________________________________________

void Win32Window::onResize(int widthInPixels, int heightInPixels)
{
	if (!m_pSwapChain)
		return;					// WM_SIZE can arrive before the constructor is done.

	// A minimized window has a zero-sized client area, which ResizeBuffers()
	// doesn't accept. Keep the old buffers until we get a real size again.

	if (widthInPixels == 0 || heightInPixels == 0)
	{
		m_bHidden = true;
		return;
	}

	m_width = widthInPixels;
	m_height = heightInPixels;

	// The new buffers hold nothing, and the old rects belong to a canvas that no
	// longer exists, so the next present has to be a full frame.

	m_dirtyRects.clear();
	m_bPresentFullFrame = true;

	// All references to the buffers must be gone and the GPU done with them
	// before ResizeBuffers(). Waiting for DX12Backend alone is not enough,
	// DXGI queues its own present work on the same command queue.

	g_pDX12Wrapper->flushRenderQueue();

	_dropSwapChainBuffers();
	auto retVal = m_pSwapChain->ResizeBuffers(0, widthInPixels, heightInPixels, DXGI_FORMAT_UNKNOWN, 0);	// Flags must match those used at creation.
	assert(retVal == S_OK);
	_createSwapChainBuffers();

	m_currentBuffer = m_pSwapChain->GetCurrentBackBufferIndex();

	m_bHidden = false;

	UINT dpi = GetDpiForWindow(m_windowHandle);
	int scale = dpi * 64 / 96; // 96 DPI is 100% scaling

	// Update root panel

	_backend()->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), { widthInPixels * 64, heightInPixels * 64 }, scale);
	m_pRootPanel->setCanvas(CanvasRef::Default);

	//

	m_pUserWindow->onResize({ pts(widthInPixels * scale / 64), pts(heightInPixels * scale / 64) });
}

//____ destroy() _____________________________________________________________

void Win32Window::destroy()
{
	DestroyWindow(m_windowHandle);
}

//____ setGeo() ______________________________________________________________

Rect Win32Window::setGeo(const wg::Rect& geo)
{
	//TODO: Implement!!!
	return geo;
}

//____ requestFocus() ________________________________________________________

bool Win32Window::requestFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ releaseFocus() ________________________________________________________

bool Win32Window::releaseFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ minimize() ____________________________________________________________

bool Win32Window::minimize()
{
	//TODO: Implement!!!
	return false;
}

//____ restore() _____________________________________________________________

bool Win32Window::restore()
{
	//TODO: Implement!!!
	return false;
}

//____ setTitle() _____________________________________________________________

bool Win32Window::setTitle(std::string& title)
{
	auto wTitle = _stringToWString(title);
	SetWindowTextW(m_windowHandle, wTitle.c_str());
	return true;
}

//____ title() ________________________________________________________________

std::string Win32Window::title()
{
	int length = GetWindowTextLengthW(m_windowHandle);

	if (length == 0)
		return std::string();

	// Get the wide string
	std::vector<wchar_t> wideString(length + 1);
	GetWindowTextW(m_windowHandle, wideString.data(), length + 1);

	// Convert to UTF-8
	int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wideString.data(), -1, nullptr, 0, nullptr, nullptr);

	if (utf8Size > 0)
	{
		std::string utf8String(utf8Size - 1, 0); // -1 to exclude null terminator
		WideCharToMultiByte(CP_UTF8, 0, wideString.data(), -1, &utf8String[0], utf8Size, nullptr, nullptr);
		return utf8String;
	}

	return std::string();
}

//____ _createSwapChain() ______________________________________________________

void Win32Window::_createSwapChain(DX12Wrapper* pDX12Wrapper, const HWND hwnd, UINT width, UINT height)
{
	ID3D12Device* pDevice = pDX12Wrapper->dx12Device();
	IDXGIFactory2* pFactory = pDX12Wrapper->dxgiFactory();
	ID3D12CommandQueue* pCommandQueue = pDX12Wrapper->renderCommandQueue();

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

	// DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING would need a check with
	// IDXGIFactory5::CheckFeatureSupport() first, and Present1() doesn't ask for
	// tearing anyway. If it's added back, ResizeBuffers() must use the same flags.

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

void Win32Window::_createSwapChainBuffers()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < c_nbBuffers; i++)
	{
		m_rtvHandles[i] = rtvHandle;

		m_renderBuffers[i].Reset();

		if (S_OK != m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderBuffers[i])))
			assert(false);
		m_pDX12Device->CreateRenderTargetView(m_renderBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_heapIncrement;
	}
}

//____ _dropSwapChainBuffers() ____________________________________________________

void Win32Window::_dropSwapChainBuffers()
{
	for (UINT i = 0; i < c_nbBuffers; i++)
		m_renderBuffers[i].Reset();
}
