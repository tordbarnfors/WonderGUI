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

#include <windows.h>


//____ Win32Window _______________________________________________________________

class Win32Window;
typedef	wg::StrongPtr<Win32Window>	Win32Window_p;
typedef	wg::WeakPtr<Win32Window>	Win32Window_wp;


class Win32Window : public Window
{
public:

    //.____ Creation __________________________________________

    static Win32Window_p        create(const Blueprint& blueprint);

    //.____ Identification __________________________________________

    const wg::TypeInfo& typeInfo(void) const override;
    const static wg::TypeInfo   TYPEINFO;

    //.____ Misc ____________________________________________________

    bool			setTitle(std::string& title) override;
    std::string		title() const override;

    bool			setIcon(wg::Surface* pIcon) override;

    virtual void	render();

protected:
    Win32Window(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo);
    virtual ~Win32Window();

    wg::Rect        _updateWindowGeo(const wg::Rect& geo) override;

    HWND            m_windowHandle;
};