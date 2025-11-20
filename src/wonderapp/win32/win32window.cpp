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
		

		auto pCanvas = Base::defaultSurfaceFactory()->createSurface({
			.canvas = true,
			.format = PixelFormat::BGRA_8,
			.size = { (int) size.w, (int) size.h}
			});


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
}


//____ render() _______________________________________________________________

void Win32Window::render()
{

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


