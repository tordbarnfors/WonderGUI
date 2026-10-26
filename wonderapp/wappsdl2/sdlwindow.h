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

#include <wappwindow.h>

#ifdef WIN32
#    include <SDL.h>
#    include <Windows.h>
#    include <libloaderapi.h>
#elif __APPLE__
#    include <SDL2/SDL.h>
#    include <dlfcn.h>
#else
#    include <SDL2/SDL.h>
#    include <dlfcn.h>
#endif


//____ SDLWindow _______________________________________________________________

struct SDL_Window;

class SDLWindow : public wapp::WindowAPI::SysCalls
{
public:
	SDLWindow();
	virtual ~SDLWindow();

    //.____ Misc ____________________________________________________

	virtual void		render() = 0;
	virtual void    	onWindowSizeUpdated( int w, int h ) = 0;

	wapp::Window*		userWindow() const { return m_pUserWindow; }
	wg::RootPanel_p		rootPanel() const { return m_pRootPanel; }

	uint32_t			SDLWindowId();
	SDL_Window*			SDLWindowPtr() { return m_pSDLWindow; }

protected:

	// SysCalls interface

	void			destroy() override;
	wg::Rect		setGeo(const wg::Rect& geo) override;
	bool			requestFocus() override;
	bool			releaseFocus() override;
	bool			minimize() override;
	bool			restore() override;
	bool 			setTitle(std::string& title) override;
	std::string 	title() override;

	//


	SDL_Window*		m_pSDLWindow;

	wapp::Window *	m_pUserWindow;
	wg::RootPanel_p	m_pRootPanel;
};
