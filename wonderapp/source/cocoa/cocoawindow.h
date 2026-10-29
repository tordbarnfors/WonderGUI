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

// Objective-C++ header, only to be included from .mm files.

#import <Cocoa/Cocoa.h>

#include <wonderapp.h>
#include <wondergui.h>
#include <wappwindow.h>

#include <time.h>

class CocoaWindow;

//____ WappView ________________________________________________________________
//
// Content view of our windows. Turns mouse, scroll and key events into calls to
// the InputHandler. Rendering is done to a layer set by the backend.

@interface WappView : NSView <NSTextInputClient>
- (instancetype) initWithFrame:(NSRect)frame cocoaWindow:(CocoaWindow*)pWindow;
- (void) detach;
@end

//____ WappWindowDelegate ______________________________________________________

@interface WappWindowDelegate : NSObject <NSWindowDelegate>
- (instancetype) initWithCocoaWindow:(CocoaWindow*)pWindow;
- (void) detach;
@end


//____ CocoaWindow _____________________________________________________________
//
// Base class for the windows of the Cocoa WonderApp frameworks, one subclass per
// rendering backend.
//
// Sizes and positions in the WonderApp API are in points, which are the same as
// the points of AppKit. Positions are relative to the top left corner of the
// main screen. The root panel's scale is the backing scale factor (64 for 100%),
// so the canvas is the window's size in pixels.

class CocoaWindow : public wapp::WindowAPI::SysCalls
{
public:
	virtual ~CocoaWindow();

	//.____ Misc ____________________________________________________

	virtual void		render() = 0;

	// Called when the size in pixels or the backing scale factor of the window has changed.

	virtual void		onWindowSizeUpdated() = 0;

	wapp::Window*		userWindow() const { return m_pUserWindow; }
	wg::RootPanel_p		rootPanel() const { return m_pRootPanel; }
	NSWindow*			nsWindow() const { return m_window; }

	bool				isDestroyed() const { return m_bDestroyed; }

	// While s_bHoldShow is set, windows created open are kept hidden until
	// showIfHeld() is called. Used to keep windows hidden while the app
	// initializes, since no events are processed until then.

	static bool			s_bHoldShow;
	void				showIfHeld();

	// Called by the window delegate.

	void				onCloseRequested();
	void				onFocusGained();
	void				onFocusLost();
	void				onMoved();
	void				onMinimized();
	void				onRestored();

protected:

	// Creates the window with the given size in points, placed on the main screen
	// according to origin, with pos as an offset in points. The backend gives the
	// view its layer afterwards.

	bool				_createNSWindow( const std::string& title, wg::Placement origin, wg::Coord pos, wg::Size size, bool resizable, bool open );

	int					_scale();					// Backing scale factor of the window, 64 for 100%.
	wg::SizeI			_sizeInPixels();

	void				_notifyUserWindowOfResize();

	// SysCalls interface

	void				destroy() override;
	wg::Rect			setGeo(const wg::Rect& geo) override;
	bool				requestFocus() override;
	bool				releaseFocus() override;
	bool				minimize() override;
	bool				restore() override;
	bool 				setTitle(std::string& title) override;
	std::string 		title() override;

	//

	NSWindow*			m_window = nil;
	WappView*			m_view = nil;
	WappWindowDelegate*	m_delegate = nil;

	wapp::Window *		m_pUserWindow = nullptr;
	wg::RootPanel_p		m_pRootPanel;

	bool				m_bShowHeld = false;
	bool				m_bDestroyed = false;
};

//____ Shared functions ________________________________________________________

// Microseconds since an arbitrary point in time. Used for all timestamps.

inline int64_t cocoaTimestamp() { return int64_t(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1000); }

// Pointer style and locking, implemented in cocoawindow.mm since our views need them.

void	cocoaSetPointerStyle(wg::PointerStyle style);
bool	cocoaLockPointer();
bool	cocoaUnlockPointer();

// Implemented in main.mm.

void	cocoaWindowCloseRequested(CocoaWindow* pWindow);
