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
#pragma once


#include <wonderapp.h>
#include <wondergui.h>

#include <wappwindowapi.h>
#include <windows.h>


//____ Win32Window _______________________________________________________________

class Win32Window;
typedef	wg::StrongPtr<Win32Window>	Win32Window_p;
typedef	wg::WeakPtr<Win32Window>	Win32Window_wp;


class Win32Window : public wg::Object, public wapp::WindowAPI::SysCalls
{
public:

    //.____ Creation __________________________________________

    static Win32Window_p	create(wapp::WindowAPI* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);

    //.____ Identification __________________________________________

    const wg::TypeInfo&		typeInfo(void) const override;
    const static wg::TypeInfo   TYPEINFO;

    //.____ Misc ____________________________________________________

    void	render();
	wapp::WindowAPI* userWindow() const { return m_pUserWindow; }
	wg::RootPanel_p	 rootPanel() const { return m_pRootPanel; }


protected:
    Win32Window(wapp::WindowAPI* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
    virtual ~Win32Window();

	void			_destroy() override;
	wg::Rect		_setGeo(const wg::Rect& geo) override;
	bool			_requestFocus() override;
	bool			_releaseFocus() override;
	bool			_minimize() override;
	bool			_restore() override;

    HWND				m_windowHandle;

	wapp::WindowAPI *	m_pUserWindow;
	wg::RootPanel_p		m_pRootPanel;

};