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

#include <cocoawindow.h>

//____ Backend-specific functions ______________________________________________
//
// Implemented once for each rendering backend (software, Metal, ...).

// Setup and teardown of WonderGUI's default GfxDevice and factories.

bool			initGfxBackend();
void			exitGfxBackend();

// Creates a window rendered by this backend. Its rootPanel() is null if it failed.

CocoaWindow *	createBackendWindow(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
