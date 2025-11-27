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

#include <wonderapp.h>
#include <sdlwindow.h>


using namespace wg;


//____ constructor ___________________________________________________

SDLWindow::SDLWindow()
{
}

//____ destructor _____________________________________________________________

SDLWindow::~SDLWindow()
{
}

//____ destroy() ______________________________________________________________

void SDLWindow::destroy()
{
	SDL_DestroyWindow(m_pSDLWindow);
	m_pSDLWindow = nullptr;					// Signal that window has been destroyed.
}

//____ setGeo() _______________________________________________________________

wg::Rect SDLWindow::setGeo(const wg::Rect& geo)
{
	//TODO: Calculate and update position as well.

	SDL_SetWindowSize(m_pSDLWindow, geo.w, geo.h);
	return geo;
}

//____ requestFocus() _________________________________________________________

bool SDLWindow::requestFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ releaseFocus() _________________________________________________________

bool SDLWindow::releaseFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ minimize() _____________________________________________________________

bool SDLWindow::minimize()
{
	//TODO: Implement!!!
	return false;
}

//____ restore() ______________________________________________________________

bool SDLWindow::restore()
{
	//TODO: Implement!!!
	return false;
}

//___ setTitle() ______________________________________________________________

bool SDLWindow::setTitle(std::string& title)
{
    SDL_SetWindowTitle(m_pSDLWindow, title.c_str());
    return true;
}

//____ title() ________________________________________________________________

std::string SDLWindow::title()
{
    return std::string(SDL_GetWindowTitle(m_pSDLWindow));
}

//____ SDLWindowId() __________________________________________________________

uint32_t SDLWindow::SDLWindowId()
{
    return SDL_GetWindowID(m_pSDLWindow);
}

//____ onWindowSizeUpdated() __________________________________________________

void SDLWindow::onWindowSizeUpdated( int w, int h )
{
}


