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
#include <sdlwindow.h>



//____ SDLWindowSW _______________________________________________________________


class SDLWindowSW;
typedef	wg::StrongPtr<SDLWindowSW>		SDLWindowSW_p;
typedef	wg::WeakPtr<SDLWindowSW>		SDLWindowSW_wp;

struct SDL_Window;

class SDLWindowSW : public SDLWindow
{
    friend class SDLWindow;
public:

    //.____ Identification __________________________________________

    const wg::TypeInfo&     typeInfo(void) const override;
    const static wg::TypeInfo   TYPEINFO;
    
    //.____ Misc _______________________________________________
    
    void    onWindowSizeUpdated( int w, int h ) override;

protected:
	SDLWindowSW(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow);
	~SDLWindowSW() {}

    static wg::Surface_p   _generateWindowSurface(SDL_Window* pWindow, int width, int height );

};
