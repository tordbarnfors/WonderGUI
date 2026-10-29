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

#include <SDL3/SDL.h>


//____ SDLWindow _______________________________________________________________
//
// Base class for the windows of the SDL3 WonderApp frameworks, one subclass per
// rendering backend.
//
// Sizes and positions in the WonderApp API are in points. SDL works in window
// coordinates, which are points on macOS but pixels on Windows, and renders in
// pixels. The root panel's scale is the display scale (64 for 100%), so the
// canvas is the window's size in pixels.

class SDLWindow : public wapp::WindowAPI::SysCalls
{
public:
	SDLWindow();
	virtual ~SDLWindow();

    //.____ Misc ____________________________________________________

	virtual void		render() = 0;

	// Called when the size in pixels or the display scale of the window has changed.

	virtual void    	onWindowSizeUpdated() = 0;

	wapp::Window*		userWindow() const { return m_pUserWindow; }
	wg::RootPanel_p		rootPanel() const { return m_pRootPanel; }

	uint32_t			SDLWindowId();
	SDL_Window*			SDLWindowPtr() { return m_pSDLWindow; }

	wg::Coord			windowCoordToPoints( float x, float y );
	wg::CoordF			pointsToWindowCoord( wg::Coord points );

	// While s_bHoldShow is set, windows created open are kept hidden until
	// showIfHeld() is called. Used to keep windows hidden while the app
	// initializes, since no events are processed until then.

	static bool			s_bHoldShow;
	void				showIfHeld();

protected:

	// Creates the SDL window with the given size in points, placed on its display
	// according to origin, with pos as an offset in points.

	bool				_createSDLWindow( const std::string& title, wg::Placement origin, wg::Coord pos, wg::Size size, SDL_WindowFlags flags, bool resizable, bool open );

	int					_scale();					// Display scale of the window, 64 for 100%.
	wg::SizeI			_sizeInPixels();

	void				_notifyUserWindowOfResize();

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

	SDL_Window*		m_pSDLWindow = nullptr;

	wapp::Window *	m_pUserWindow = nullptr;
	bool			m_bShowHeld = false;
	wg::RootPanel_p	m_pRootPanel;
};
