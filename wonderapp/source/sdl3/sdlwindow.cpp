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
#include <sdlwindow.h>


using namespace wg;


//____ constructor ___________________________________________________

bool SDLWindow::s_bHoldShow = false;

SDLWindow::SDLWindow()
{
}

//____ destructor _____________________________________________________________

SDLWindow::~SDLWindow()
{
}

//____ _createSDLWindow() _____________________________________________________

bool SDLWindow::_createSDLWindow( const std::string& title, wg::Placement origin, wg::Coord pos, wg::Size size, SDL_WindowFlags flags, bool resizable, bool open )
{
	if (resizable)
		flags |= SDL_WINDOW_RESIZABLE;

	// Hidden until it has the right size.

	flags |= SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

	m_pSDLWindow = SDL_CreateWindow(title.c_str(), int(size.w), int(size.h), flags);
	if (m_pSDLWindow == nullptr)
	{
		printf("Unable to create SDL window: %s\n", SDL_GetError());
		return false;
	}

	// Now that the window exists we know its display scale and pixel density
	// and can give it the right size in window coordinates.

	CoordF winSize = pointsToWindowCoord( { size.w, size.h } );
	SDL_SetWindowSize(m_pSDLWindow, int(winSize.x), int(winSize.y));

	// Place the window on its display, with origin deciding which part of the
	// display pos is relative to.

	SDL_Rect bounds;
	if( SDL_GetDisplayUsableBounds(SDL_GetDisplayForWindow(m_pSDLWindow), &bounds) )
	{
		float fx = 0.5f, fy = 0.5f;

		switch( origin )
		{
			case Placement::NorthWest:	fx = 0.f;	fy = 0.f;	break;
			case Placement::North:		fx = 0.5f;	fy = 0.f;	break;
			case Placement::NorthEast:	fx = 1.f;	fy = 0.f;	break;
			case Placement::East:		fx = 1.f;	fy = 0.5f;	break;
			case Placement::SouthEast:	fx = 1.f;	fy = 1.f;	break;
			case Placement::South:		fx = 0.5f;	fy = 1.f;	break;
			case Placement::SouthWest:	fx = 0.f;	fy = 1.f;	break;
			case Placement::West:		fx = 0.f;	fy = 0.5f;	break;
			default:					fx = 0.5f;	fy = 0.5f;	break;
		}

		CoordF offset = pointsToWindowCoord( pos );

		int x = bounds.x + int((bounds.w - winSize.x) * fx + offset.x);
		int y = bounds.y + int((bounds.h - winSize.y) * fy + offset.y);

		SDL_SetWindowPosition(m_pSDLWindow, x, y);
	}

	SDL_StartTextInput(m_pSDLWindow);

	if( open )
	{
		if( s_bHoldShow )
			m_bShowHeld = true;
		else
			SDL_ShowWindow(m_pSDLWindow);
	}

	return true;
}

//____ showIfHeld() ___________________________________________________________

void SDLWindow::showIfHeld()
{
	if( m_bShowHeld && m_pSDLWindow )
		SDL_ShowWindow(m_pSDLWindow);

	m_bShowHeld = false;
}

//____ _scale() _______________________________________________________________

int SDLWindow::_scale()
{
	float displayScale = SDL_GetWindowDisplayScale(m_pSDLWindow);
	if (displayScale <= 0.f)
		displayScale = 1.f;

	return int(displayScale * 64);
}

//____ _sizeInPixels() ________________________________________________________

SizeI SDLWindow::_sizeInPixels()
{
	int w = 0, h = 0;
	SDL_GetWindowSizeInPixels(m_pSDLWindow, &w, &h);
	return { w, h };
}

//____ windowCoordToPoints() __________________________________________________

Coord SDLWindow::windowCoordToPoints( float x, float y )
{
	float pixelDensity = SDL_GetWindowPixelDensity(m_pSDLWindow);
	float scale = m_pRootPanel ? m_pRootPanel->scale() / 64.f : _scale() / 64.f;

	return Coord( x * pixelDensity / scale, y * pixelDensity / scale );
}

//____ pointsToWindowCoord() __________________________________________________

CoordF SDLWindow::pointsToWindowCoord( Coord points )
{
	float pixelDensity = SDL_GetWindowPixelDensity(m_pSDLWindow);
	if (pixelDensity <= 0.f)
		pixelDensity = 1.f;

	float scale = _scale() / 64.f;

	return CoordF( points.x * scale / pixelDensity, points.y * scale / pixelDensity );
}

//____ _notifyUserWindowOfResize() ____________________________________________

void SDLWindow::_notifyUserWindowOfResize()
{
	if (!m_pUserWindow || !m_pRootPanel)
		return;

	SizeI pixels = _sizeInPixels();
	int scale = m_pRootPanel->scale();

	m_pUserWindow->onResize({ pts(pixels.w * 64 / scale), pts(pixels.h * 64 / scale) });
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
	CoordF size = pointsToWindowCoord( { geo.w, geo.h } );
	SDL_SetWindowSize(m_pSDLWindow, int(size.x), int(size.y));

	CoordF pos = pointsToWindowCoord( geo.pos() );
	SDL_SetWindowPosition(m_pSDLWindow, int(pos.x), int(pos.y));
	return geo;
}

//____ requestFocus() _________________________________________________________

bool SDLWindow::requestFocus()
{
	return SDL_RaiseWindow(m_pSDLWindow);
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
	return SDL_MinimizeWindow(m_pSDLWindow);
}

//____ restore() ______________________________________________________________

bool SDLWindow::restore()
{
	return SDL_RestoreWindow(m_pSDLWindow);
}

//___ setTitle() ______________________________________________________________

bool SDLWindow::setTitle(std::string& title)
{
    return SDL_SetWindowTitle(m_pSDLWindow, title.c_str());
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
