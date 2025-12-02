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

	Rect geo = { pos, size };

	m_windowHandle = CreateWindow("WappWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW, geo.x, geo.y, geo.w, geo.h, 0, 0, 0, this);

	if (!m_windowHandle)
	{
		int x = 0;
		//TODO: Error handling!
	}
	else
	{
		UINT dpi = GetDpiForWindow(m_windowHandle);
		int scale = dpi * 64 / 96; // 96 DPI is 100% scaling
	
		auto pBackend = wg_static_cast<DX12Backend_p>(wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice())->backend());
		pBackend->setDefaultCanvas({ spx(geo.w * 64), spx(geo.h * 64) },scale);
		m_pRootPanel = RootPanel::create(CanvasRef::Default, Base::defaultGfxDevice());
		assert(m_pRootPanel);
		
		m_pDXGISwapChain = new DXGI_SwapChain(g_pDX12Wrapper, m_windowHandle, m_pRootPanel, (UINT)size.w, (UINT)size.h);

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
	delete m_pDXGISwapChain;
}


//____ render() _______________________________________________________________

void Win32Window::render()
{
	if (m_bHidden)
		return;

//	m_pRootPanel->render();

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

		m_pDXGISwapChain->swapChain()->Present1(0, 0, &presentParams);
	}

	DeleteObject(updateRegion);
	ValidateRect(m_windowHandle, nullptr);
}


//____ onResize() ____________________________________________________________

void Win32Window::onResize(int widthInPixels, int heightInPixels)
{
	m_pDXGISwapChain->resizeBuffers(widthInPixels, heightInPixels);

	if (widthInPixels == 0 || heightInPixels == 0)
	{
		m_bHidden = true;
		return;
	}

	m_bHidden = false;

	UINT dpi = GetDpiForWindow(m_windowHandle);
	int scale = dpi * 64 / 96; // 96 DPI is 100% scaling

	// Update root panel

	auto pBackend = wg_static_cast<DX12Backend_p>(wg_static_cast<GfxDeviceGen2_p>(Base::defaultGfxDevice())->backend());
	pBackend->setDefaultCanvas({ widthInPixels * 64, heightInPixels * 64 }, scale);
	m_pRootPanel = RootPanel::create(CanvasRef::Default, Base::defaultGfxDevice());

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


