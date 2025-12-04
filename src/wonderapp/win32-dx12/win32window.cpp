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
#include <wg_dx12backend.h>
#include <dx12_wrapper.h>

#include <wg_gfxdevice.h>
#include <wg_dx12backend.h>


using namespace wg;

extern DX12Wrapper* g_pDX12Wrapper;


//____ constructor ___________________________________________________

Win32Window::Win32Window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	RectI pixelGeo = { int(pos.x), int(pos.y), int(size.w), int(size.h) };

	m_width = pixelGeo.w;
	m_height = pixelGeo.h;


	m_windowHandle = CreateWindow("WappWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW, pixelGeo.x, pixelGeo.y, pixelGeo.w, pixelGeo.h, 0, 0, 0, this);

	if (!m_windowHandle)
	{
		int x = 0;
		//TODO: Error handling!
	}
	else
	{
		_createSwapChain(g_pDX12Wrapper, m_windowHandle, m_width, m_height);

		UINT dpi = GetDpiForWindow(m_windowHandle);
		int scale = dpi * 64 / 96; // 96 DPI is 100% scaling
	
		auto pBackend = wg_static_cast<DX12Backend_p>(wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice())->backend());
		pBackend->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), { spx(pixelGeo.w * 64), spx(pixelGeo.h * 64) },scale);
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
}


//____ render() _______________________________________________________________

void Win32Window::render()
{
	if (m_bHidden)
		return;

	GfxDeviceGen2_p pGfxDevice = wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice());

	auto pBackend = wg_static_cast<DX12Backend_p>(pGfxDevice->backend());


	pBackend->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), {spx(m_width * 64), spx(m_height * 64)}, m_pRootPanel->scale());

//	m_pRootPanel->addDirtyPatch({ 0,0, spx(m_width * 64), spx(m_height * 64) });

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
		InvalidateRect(m_windowHandle, &rc, FALSE);
	}
}

//____ paint() ________________________________________________________________

void Win32Window::paint()
{
	HRGN updateRegion = CreateRectRgn(0, 0, 0, 0);

	if (GetUpdateRgn(m_windowHandle, updateRegion, FALSE) != NULLREGION)
	{
		// First call to get required buffer size
		DWORD size = GetRegionData(updateRegion, 0, nullptr);

		// Allocate buffer
		std::vector<BYTE> buffer(size);
		RGNDATA* regionData = reinterpret_cast<RGNDATA*>(buffer.data());

		// Get the actual data
		GetRegionData(updateRegion, size, regionData);

		// Extract rectangles
		RECT* rects = reinterpret_cast<RECT*>(regionData->Buffer);
		DWORD rectCount = regionData->rdh.nCount;

		std::vector<RECT> dirtyRects(rects, rects + rectCount);

		DXGI_PRESENT_PARAMETERS presentParams = {};
		presentParams.DirtyRectsCount = dirtyRects.size();
		presentParams.pDirtyRects = dirtyRects.data();

//		m_pSwapChain->Present(0, 0);
		m_pSwapChain->Present1(0, 0, &presentParams);
		m_currentBuffer = (m_currentBuffer + 1) % c_nbBuffers;
	}

	DeleteObject(updateRegion);
	ValidateRect(m_windowHandle, nullptr);
}


//____ onResize() ____________________________________________________________

void Win32Window::onResize(int widthInPixels, int heightInPixels)
{
	m_width = widthInPixels;
	m_height = heightInPixels;

	auto pBackend = wg_static_cast<DX12Backend_p>(wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice())->backend());
	pBackend->waitForCompletion();

	_dropSwapChainBuffers();
	auto retVal = m_pSwapChain->ResizeBuffers(0, widthInPixels, heightInPixels, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING  /* | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH*/);
	assert(retVal == S_OK);
	_createSwapChainBuffers();

	m_currentBuffer = 0;

	if (widthInPixels == 0 || heightInPixels == 0)
	{
		m_bHidden = true;
		return;
	}

	m_bHidden = false;

	UINT dpi = GetDpiForWindow(m_windowHandle);
	int scale = dpi * 64 / 96; // 96 DPI is 100% scaling

	// Update root panel

	pBackend->setDefaultCanvas(m_rtvHandles[m_currentBuffer], m_renderBuffers[m_currentBuffer].Get(), { widthInPixels * 64, heightInPixels * 64 }, scale);
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
	return false;
}

//____ title() ________________________________________________________________

std::string Win32Window::title()
{
	return "";
}

//____ _createSwapChain() ______________________________________________________

void Win32Window::_createSwapChain(DX12Wrapper* pDX12Wrapper, const HWND hwnd, UINT width, UINT height)
{
	ID3D12Device* pDevice = pDX12Wrapper->dx12Device();

	IDXGIFactory2* pFactory = static_cast<IDXGIFactory3*>(pDX12Wrapper->dxgiFactory());

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
	description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; /* | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; */

	if (S_OK != pFactory->CreateSwapChainForHwnd(pCommandQueue, hwnd, &description, nullptr, nullptr, &m_pSwapChain))
		assert(false);

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
 