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

const TypeInfo Win32Window::TYPEINFO = { "Win32Window", &Window::TYPEINFO };


//____ create() _______________________________________________________________

Win32Window_p Win32Window::create(const Blueprint& blueprint)
{
	auto pRootPanel = RootPanel::create();


	return Win32Window_p(new Win32Window(blueprint.title, pRootPanel, blueprint.size ));
}

//____ constructor ___________________________________________________

Win32Window::Win32Window(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo)
    : Window(pRootPanel, geo)
{


	m_windowHandle = CreateWindow("WappWindowClass", title.c_str(), WS_OVERLAPPEDWINDOW, geo.x, geo.h, geo.w, geo.h, 0, 0, 0, this);

	if (!m_windowHandle)
	{
		int x = 0;
		// Error handling!
	}

	ShowWindow(m_windowHandle, SW_SHOW);
	UpdateWindow(m_windowHandle);

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


//____ setTitle() _____________________________________________________________

bool Win32Window::setTitle(std::string& title)
{
	return false;
}

//____ title() ________________________________________________________________

std::string Win32Window::title() const
{
	return "";
}

//____ setIcon() ______________________________________________________________

bool Win32Window::setIcon(wg::Surface* pIcon)
{
	return false;
}

//____ render() _______________________________________________________________

void Win32Window::render()
{

}

//____ _updateWindowGeo() _____________________________________________________

wg::Rect Win32Window::_updateWindowGeo(const wg::Rect& geo)
{
	return geo;
}