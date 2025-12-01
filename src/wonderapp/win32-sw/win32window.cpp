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
#include <wg_softsurface.h>

using namespace wg;

//____ constructor ___________________________________________________

Win32Window::Win32Window(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	Rect geo = { pos, size };



	m_windowHandle = CreateWindow("WappWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW, geo.x, geo.h, geo.w, geo.h, 0, 0, 0, this);

	if (!m_windowHandle)
	{
		int x = 0;
		// Error handling!
	}
	else
	{
		BITMAPINFO bmi = { 0 };
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = geo.w;
		bmi.bmiHeader.biHeight = -geo.h; // Negativt = top-down
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		HDC hdcScreen = GetDC(NULL);
		m_hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS,
			(void**)&m_pCanvasPixels, NULL, 0);
		ReleaseDC(NULL, hdcScreen);

		UINT dpi = GetDpiForWindow(m_windowHandle);
		int scale = dpi * 64 / 96; // 96 DPI is 100% scaling


		auto pCanvas = wg::SoftSurface::createInPlace({
			.canvas = true,
			.format = PixelFormat::BGRA_8,
			.scale = scale,
			.size = { (int) size.w, (int) size.h}
			}, (uint8_t*) m_pCanvasPixels);


		m_pRootPanel = RootPanel::create(pCanvas, Base::defaultGfxDevice());

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
	DeleteObject(m_hBitmap);
}


//____ render() _______________________________________________________________

void Win32Window::render()
{
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

//____ onResize() ____________________________________________________________

void Win32Window::onResize(int widthInPixels, int heightInPixels)
{
	// Resize bitmap

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = widthInPixels;
	bmi.bmiHeader.biHeight = -heightInPixels; // Negativt = top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Delete old bitmap

	DeleteObject(m_hBitmap);

	// Create new bitmap

	HDC hdcScreen = GetDC(NULL);
	m_hBitmap = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS,
		(void**)&m_pCanvasPixels, NULL, 0);
	ReleaseDC(NULL, hdcScreen);



	UINT dpi = GetDpiForWindow(m_windowHandle);
	int scale = dpi * 64 / 96; // 96 DPI is 100% scaling

	// Update root panel's surface

	auto pCanvas = wg::SoftSurface::createInPlace({
		.canvas = true,
		.format = PixelFormat::BGRA_8,
		.scale = scale,
		.size = { widthInPixels, heightInPixels }
		}, (uint8_t*)m_pCanvasPixels);

	m_pRootPanel->setCanvas(pCanvas);
	m_pUserWindow->onResize({ pts(widthInPixels*scale/64), pts(heightInPixels*scale/64) });
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


