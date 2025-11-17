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

const TypeInfo Win32Window::TYPEINFO = { "Win32Window", &Object::TYPEINFO };


//____ create() _______________________________________________________________

Win32Window_p Win32Window::create(wapp::WindowAPI* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return Win32Window_p(new Win32Window(pUserWindow, origin, pos, size, title, resizable, open));
}

//____ constructor ___________________________________________________

Win32Window::Win32Window(wapp::WindowAPI* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
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

//____ typeInfo() _________________________________________________________

const wg::TypeInfo& Win32Window::typeInfo(void) const
{
    return TYPEINFO;
}


//____ render() _______________________________________________________________

void Win32Window::render()
{

}

//____ _destroy() _____________________________________________________________

void Win32Window::_destroy()
{
	//TODO: Implement!!!
}

//____ _setGeo() ______________________________________________________________

Rect Win32Window::_setGeo(const wg::Rect& geo)
{
	//TODO: Implement!!!
	return geo;
}

//____ _requestFocus() ________________________________________________________

bool Win32Window::_requestFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ _releaseFocus() ________________________________________________________

bool Win32Window::_releaseFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ _minimize() ____________________________________________________________

bool Win32Window::_minimize()
{
	//TODO: Implement!!!
	return false;
}

//____ _restore() _____________________________________________________________

bool Win32Window::_restore()
{
	//TODO: Implement!!!
	return false;
}

