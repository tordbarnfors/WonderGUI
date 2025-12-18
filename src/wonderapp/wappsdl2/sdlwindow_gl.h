/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#pragma once

#include <wonderapp.h>
#include <wondergui.h>
#include <sdlwindow.h>

#ifdef WIN32
#    include <SDL.h>
#elif __APPLE__
#    include <SDL2/SDL.h>
#else
#    include <SDL2/SDL.h>
#endif


//____ SDLWindowGL _______________________________________________________________

class SDLWindowGL;
typedef	wg::StrongPtr<SDLWindowGL>		SDLWindowGL_p;
typedef	wg::WeakPtr<SDLWindowGL>		SDLWindowGL_wp;


class SDLWindowGL : public SDLWindow
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
	SDLWindowGL(const std::string& title, wg::RootPanel* pRootPanel, const wg::Rect& geo, SDL_Window* pSDLWindow, SDL_GLContext glContext );
    ~SDLWindowGL() { s_windowCounter--; }

    SDL_GLContext       m_SDLGLContext = 0;

    static int          s_windowCounter;
    static bool         s_bInitialized;

};
