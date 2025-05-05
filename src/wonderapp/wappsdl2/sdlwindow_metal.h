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

#include <SDL2/SDL.h>


//____ SDLWindowMetal _______________________________________________________________

class SDLWindowMetal;
typedef	wg::StrongPtr<SDLWindowMetal>	SDLWindowMetal_p;
typedef	wg::WeakPtr<SDLWindowMetal>		SDLWindowMetal_wp;


class SDLWindowMetal : public SDLWindow
{
    friend class SDLWindow;
public:

    //.____ Identification __________________________________________

    const wg::TypeInfo&     typeInfo(void) const override;
    const static wg::TypeInfo   TYPEINFO;
    
    //.____ Misc _______________________________________________
    
    void    onWindowSizeUpdated( int w, int h ) override;

    void    render() override;

    
protected:
	SDLWindowMetal(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow, SDL_Renderer * pRenderer );
    ~SDLWindowMetal() { /*s_windowCounter--;*/ }

    SDL_Renderer *      m_pSDLRenderer = 0;

	wg::GfxBackend_p	m_pBackend;

//    static int          s_windowCounter;
//    static bool         s_bInitialized;

};
