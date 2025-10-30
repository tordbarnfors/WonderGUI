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

//____ SDLWindow _______________________________________________________________

class SDLWindow;
typedef	wg::StrongPtr<SDLWindow>	SDLWindow_p;
typedef	wg::WeakPtr<SDLWindow>		SDLWindow_wp;

struct SDL_Window;

class SDLWindow : public Window
{
public:

    //.____ Creation __________________________________________
    
    static SDLWindow_p        create(const Blueprint& blueprint);       // Implemented separately for Software, OpenGL and Metal.

    //.____ Identification __________________________________________

    const wg::TypeInfo&     typeInfo(void) const override;
    const static wg::TypeInfo   TYPEINFO;
    
    //.____ Misc ____________________________________________________

	bool			setTitle(std::string& title) override;
	std::string		title() const override;

	bool			setIcon(wg::Surface * pIcon) override;

	virtual void	render();

    uint32_t		SDLWindowId();
    SDL_Window*		SDLWindowPtr() { return m_pSDLWindow; }
    
    virtual void    onWindowSizeUpdated( int w, int h ) = 0;

    
protected:
	SDLWindow(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow);
    virtual ~SDLWindow();

    wg::Rect        _updateWindowGeo(const wg::Rect& geo) override;

	SDL_Window*		m_pSDLWindow;
};
