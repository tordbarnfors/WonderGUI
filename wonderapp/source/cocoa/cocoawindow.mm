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
#include <cocoawindow.h>

#include <Carbon/Carbon.h>		// For the kVK_* key codes only.

#include <cmath>

using namespace wg;

bool CocoaWindow::s_bHoldShow = false;

static NSCursor *	g_pCurrentCursor = nil;		// Cursor for our views, nil for arrow.

static bool			g_bPointerLocked = false;
static Coord		g_lockedVirtualPos;			// Pointer position while locked, in points.

//____ _topOfMainScreen() ______________________________________________________
//
// AppKit has its origin in the bottom left corner of the main screen, we have
// ours in its top left corner.

static CGFloat _topOfMainScreen()
{
	NSArray<NSScreen*> * screens = [NSScreen screens];
	return screens.count > 0 ? NSMaxY(screens[0].frame) : 0;
}


//____ WappView ________________________________________________________________

@implementation WappView
{
	CocoaWindow *		m_pWindow;
	NSTrackingArea *	m_trackingArea;
}

//____ initWithFrame() _________________________________________________________

- (instancetype) initWithFrame:(NSRect)frame cocoaWindow:(CocoaWindow*)pWindow
{
	self = [super initWithFrame:frame];
	if( self )
	{
		m_pWindow = pWindow;
		m_trackingArea = nil;
	}
	return self;
}

//____ dealloc() _______________________________________________________________

- (void) dealloc
{
	[m_trackingArea release];
	[super dealloc];
}

//____ detach() ________________________________________________________________

- (void) detach
{
	m_pWindow = nullptr;
}

//____ View properties _________________________________________________________

- (BOOL) isFlipped { return YES; }							// Top left origin, like us.
- (BOOL) acceptsFirstResponder { return YES; }
- (BOOL) acceptsFirstMouse:(NSEvent *)event { return YES; }	// Click on an inactive window goes through.

//____ updateTrackingAreas() ___________________________________________________

- (void) updateTrackingAreas
{
	if( m_trackingArea )
	{
		[self removeTrackingArea:m_trackingArea];
		[m_trackingArea release];
	}

	m_trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
												  options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingCursorUpdate |
														  NSTrackingActiveAlways | NSTrackingInVisibleRect
													owner:self userInfo:nil];
	[self addTrackingArea:m_trackingArea];
	[super updateTrackingAreas];
}

//____ cursorUpdate() __________________________________________________________

- (void) cursorUpdate:(NSEvent *)event
{
	[(g_pCurrentCursor ? g_pCurrentCursor : [NSCursor arrowCursor]) set];
}

//____ _root() _________________________________________________________________

- (RootPanel*) _root
{
	return (m_pWindow && !m_pWindow->isDestroyed()) ? m_pWindow->rootPanel().rawPtr() : nullptr;
}

//____ _setPointer() ___________________________________________________________

- (void) _setPointer:(NSEvent *)event
{
	RootPanel * pRoot = [self _root];
	if( !pRoot )
		return;

	Coord pos;

	if( g_bPointerLocked )
	{
		// The cursor stays put while locked, so we move a virtual pointer by the deltas.

		g_lockedVirtualPos.x += event.deltaX;
		g_lockedVirtualPos.y += event.deltaY;
		pos = g_lockedVirtualPos;
	}
	else
	{
		NSPoint p = [self convertPoint:event.locationInWindow fromView:nil];
		pos = Coord( p.x, p.y );
	}

	Base::inputHandler()->setPointer(pRoot, pos, cocoaTimestamp());
}

//____ _setButton() ____________________________________________________________

- (void) _setButton:(NSEvent *)event pressed:(bool)bPressed
{
	if( ![self _root] )
		return;

	MouseButton button;

	switch( event.buttonNumber )
	{
		case 0:		button = MouseButton::Left;		break;
		case 1:		button = MouseButton::Right;	break;
		case 2:		button = MouseButton::Middle;	break;
		case 3:		button = MouseButton::X1;		break;
		case 4:		button = MouseButton::X2;		break;
		default:	return;
	}

	// Make sure the pointer is where the button was pressed.

	[self _setPointer:event];
	Base::inputHandler()->setButton(button, bPressed, cocoaTimestamp());
}

//____ Mouse events ____________________________________________________________

- (void) mouseMoved:(NSEvent *)event			{ [self _setPointer:event]; }
- (void) mouseDragged:(NSEvent *)event			{ [self _setPointer:event]; }
- (void) rightMouseDragged:(NSEvent *)event		{ [self _setPointer:event]; }
- (void) otherMouseDragged:(NSEvent *)event		{ [self _setPointer:event]; }
- (void) mouseEntered:(NSEvent *)event			{ [self _setPointer:event]; }

- (void) mouseDown:(NSEvent *)event				{ [self _setButton:event pressed:true]; }
- (void) mouseUp:(NSEvent *)event				{ [self _setButton:event pressed:false]; }
- (void) rightMouseDown:(NSEvent *)event		{ [self _setButton:event pressed:true]; }
- (void) rightMouseUp:(NSEvent *)event			{ [self _setButton:event pressed:false]; }
- (void) otherMouseDown:(NSEvent *)event		{ [self _setButton:event pressed:true]; }
- (void) otherMouseUp:(NSEvent *)event			{ [self _setButton:event pressed:false]; }

//____ mouseExited() ___________________________________________________________

- (void) mouseExited:(NSEvent *)event
{
	// While a button is pressed we keep getting drag events outside the window,
	// so we only tell that the pointer has left when none is.

	RootPanel * pRoot = [self _root];
	if( pRoot && !g_bPointerLocked && [NSEvent pressedMouseButtons] == 0 )
		Base::inputHandler()->setPointer(pRoot, { -1, -1 }, cocoaTimestamp());
}

//____ scrollWheel() ___________________________________________________________

- (void) scrollWheel:(NSEvent *)event
{
	if( ![self _root] )
		return;

	// Scaled like SDL does, so wheels and trackpads feel the same as with the SDL
	// frameworks. Trackpads give precise deltas in points, mouse wheels in lines.

	CGFloat x, y;

	if( event.hasPreciseScrollingDeltas )
	{
		x = -event.scrollingDeltaX * 0.1;
		y = event.scrollingDeltaY * 0.1;
	}
	else
	{
		x = -event.deltaX;
		y = event.deltaY;

		x = x > 0 ? std::ceil(x) : std::floor(x);
		y = y > 0 ? std::ceil(y) : std::floor(y);
	}

	// Deltas follow the content, so with natural scrolling they are inverted
	// compared to the device.

	bool bInvertScroll = event.isDirectionInvertedFromDevice;
	if( bInvertScroll )
	{
		x = -x;
		y = -y;
	}

	auto timestamp = cocoaTimestamp();

	if( y != 0 )
		Base::inputHandler()->setWheelRoll(1, pts(y), bInvertScroll, timestamp);

	if( x != 0 )
		Base::inputHandler()->setWheelRoll(2, pts(x), bInvertScroll, timestamp);
}

//____ keyDown() _______________________________________________________________

- (void) keyDown:(NSEvent *)event
{
	if( ![self _root] )
		return;

	// The InputHandler generates its own key repeats.

	if( !event.isARepeat )
		Base::inputHandler()->setKey(event.keyCode, true, cocoaTimestamp());

	// Let the text input system turn it into text, unless it is a command.

	if( (event.modifierFlags & (NSEventModifierFlagCommand | NSEventModifierFlagControl)) == 0 )
		[self interpretKeyEvents:@[event]];
}

//____ keyUp() _________________________________________________________________

- (void) keyUp:(NSEvent *)event
{
	if( ![self _root] )
		return;

	Base::inputHandler()->setKey(event.keyCode, false, cocoaTimestamp());
}

//____ flagsChanged() __________________________________________________________
//
// Modifier keys don't get key down/up events, just this. The device dependent
// bits of the flags tell us whether the left or right key is pressed.

- (void) flagsChanged:(NSEvent *)event
{
	if( ![self _root] )
		return;

	NSUInteger mask;

	switch( event.keyCode )
	{
		case kVK_Shift:			mask = NX_DEVICELSHIFTKEYMASK;	break;
		case kVK_RightShift:	mask = NX_DEVICERSHIFTKEYMASK;	break;
		case kVK_Control:		mask = NX_DEVICELCTLKEYMASK;	break;
		case kVK_RightControl:	mask = NX_DEVICERCTLKEYMASK;	break;
		case kVK_Option:		mask = NX_DEVICELALTKEYMASK;	break;
		case kVK_RightOption:	mask = NX_DEVICERALTKEYMASK;	break;
		case kVK_Command:		mask = NX_DEVICELCMDKEYMASK;	break;
		case kVK_RightCommand:	mask = NX_DEVICERCMDKEYMASK;	break;
		default:				return;
	}

	bool bPressed = (event.modifierFlags & mask) != 0;
	Base::inputHandler()->setKey(event.keyCode, bPressed, cocoaTimestamp());
}

//____ NSTextInputClient _______________________________________________________
//
// Only enough to get composed text, such as from dead keys and input methods.
// Text being composed is not shown.

- (void) insertText:(id)string replacementRange:(NSRange)replacementRange
{
	if( ![self _root] )
		return;

	NSString * pText = [string isKindOfClass:[NSAttributedString class]] ? [(NSAttributedString*)string string] : (NSString*)string;

	// Skip control characters and the function key characters of AppKit, keys
	// such as return, tab and arrows are handled as keys.

	for( NSUInteger i = 0 ; i < pText.length ; i++ )
	{
		unichar c = [pText characterAtIndex:i];
		if( c < 32 || c == 127 || (c >= 0xF700 && c <= 0xF8FF) )
			return;
	}

	Base::inputHandler()->putText(pText.UTF8String);
}

- (void) doCommandBySelector:(SEL)selector {}				// Keys are handled as keys, no beeps.
- (void) setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {}
- (void) unmarkText {}
- (NSRange) selectedRange { return NSMakeRange(NSNotFound, 0); }
- (NSRange) markedRange { return NSMakeRange(NSNotFound, 0); }
- (BOOL) hasMarkedText { return NO; }
- (NSAttributedString *) attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange { return nil; }
- (NSArray<NSAttributedStringKey> *) validAttributesForMarkedText { return @[]; }
- (NSUInteger) characterIndexForPoint:(NSPoint)point { return NSNotFound; }

- (NSRect) firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
	// Where input method windows are placed. We don't know where the text cursor
	// is, so we use the window.

	return self.window ? [self.window convertRectToScreen:[self convertRect:self.bounds toView:nil]] : NSZeroRect;
}

@end


//____ WappWindowDelegate ______________________________________________________

@implementation WappWindowDelegate
{
	CocoaWindow *	m_pWindow;
}

- (instancetype) initWithCocoaWindow:(CocoaWindow*)pWindow
{
	self = [super init];
	if( self )
		m_pWindow = pWindow;
	return self;
}

- (void) detach
{
	m_pWindow = nullptr;
}

- (BOOL) windowShouldClose:(NSWindow *)sender
{
	// The app decides and closes the window by releasing it.

	if( m_pWindow )
		m_pWindow->onCloseRequested();
	return NO;
}

- (void) windowDidResize:(NSNotification *)notification
{
	// During live resize this is called from inside AppKit's own event loop, so
	// we render right away to keep up with the window.

	if( m_pWindow )
		m_pWindow->onWindowSizeUpdated();
}

- (void) windowDidChangeBackingProperties:(NSNotification *)notification
{
	if( m_pWindow )
		m_pWindow->onWindowSizeUpdated();
}

- (void) windowDidBecomeKey:(NSNotification *)notification		{ if( m_pWindow ) m_pWindow->onFocusGained(); }
- (void) windowDidResignKey:(NSNotification *)notification		{ if( m_pWindow ) m_pWindow->onFocusLost(); }
- (void) windowDidMove:(NSNotification *)notification			{ if( m_pWindow ) m_pWindow->onMoved(); }
- (void) windowDidMiniaturize:(NSNotification *)notification	{ if( m_pWindow ) m_pWindow->onMinimized(); }
- (void) windowDidDeminiaturize:(NSNotification *)notification	{ if( m_pWindow ) m_pWindow->onRestored(); }

@end


//____ ~CocoaWindow() __________________________________________________________

CocoaWindow::~CocoaWindow()
{
	[m_view detach];
	[m_delegate detach];

	[m_window setDelegate:nil];
	[m_window close];

	[m_window release];
	[m_view release];
	[m_delegate release];
}

//____ _createNSWindow() _______________________________________________________

bool CocoaWindow::_createNSWindow( const std::string& title, wg::Placement origin, wg::Coord pos, wg::Size size, bool resizable, bool open )
{
	NSRect contentRect = NSMakeRect(0, 0, size.w, size.h);

	NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
	if( resizable )
		style |= NSWindowStyleMaskResizable;

	m_window = [[NSWindow alloc] initWithContentRect:contentRect styleMask:style backing:NSBackingStoreBuffered defer:NO];
	if( !m_window )
	{
		printf("Unable to create NSWindow\n");
		return false;
	}

	[m_window setReleasedWhenClosed:NO];
	[m_window setTabbingMode:NSWindowTabbingModeDisallowed];
	[m_window setTitle:[NSString stringWithUTF8String:title.c_str()]];

	m_view = [[WappView alloc] initWithFrame:contentRect cocoaWindow:this];
	[m_window setContentView:m_view];
	[m_window makeFirstResponder:m_view];

	m_delegate = [[WappWindowDelegate alloc] initWithCocoaWindow:this];
	[m_window setDelegate:m_delegate];

	// Place the window on the main screen, with origin deciding which part of the
	// screen pos is relative to.

	NSScreen * pScreen = [NSScreen mainScreen];
	if( pScreen )
	{
		NSRect bounds = pScreen.visibleFrame;
		NSRect frame = [m_window frameRectForContentRect:contentRect];

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

		CGFloat x = bounds.origin.x + (bounds.size.width - frame.size.width) * fx + pos.x;
		CGFloat yFromTop = (bounds.size.height - frame.size.height) * fy + pos.y;

		[m_window setFrameOrigin:NSMakePoint(x, NSMaxY(bounds) - yFromTop - frame.size.height)];
	}

	if( open )
	{
		if( s_bHoldShow )
			m_bShowHeld = true;
		else
			[m_window makeKeyAndOrderFront:nil];
	}

	return true;
}

//____ showIfHeld() ___________________________________________________________

void CocoaWindow::showIfHeld()
{
	if( m_bShowHeld && m_window )
		[m_window makeKeyAndOrderFront:nil];

	m_bShowHeld = false;
}

//____ _scale() _______________________________________________________________

int CocoaWindow::_scale()
{
	CGFloat factor = m_window.backingScaleFactor;
	if( factor <= 0 )
		factor = 1;

	return int(factor * 64);
}

//____ _sizeInPixels() ________________________________________________________

SizeI CocoaWindow::_sizeInPixels()
{
	NSRect pixels = [m_view convertRectToBacking:m_view.bounds];
	return { int(std::lround(pixels.size.width)), int(std::lround(pixels.size.height)) };
}

//____ _notifyUserWindowOfResize() ____________________________________________

void CocoaWindow::_notifyUserWindowOfResize()
{
	if (!m_pUserWindow || !m_pRootPanel)
		return;

	NSSize size = m_view.bounds.size;
	m_pUserWindow->onResize({ pts(size.width), pts(size.height) });
}

//____ onCloseRequested() _____________________________________________________

void CocoaWindow::onCloseRequested()
{
	cocoaWindowCloseRequested(this);
}

//____ onFocusGained() ________________________________________________________

void CocoaWindow::onFocusGained()
{
	if( m_pRootPanel )
		Base::inputHandler()->setFocusedWindow(m_pRootPanel);

	if( m_pUserWindow )
		m_pUserWindow->onFocusGained();
}

//____ onFocusLost() __________________________________________________________

void CocoaWindow::onFocusLost()
{
	if( m_pRootPanel && Base::inputHandler()->focusedWindow() == m_pRootPanel )
		Base::inputHandler()->setFocusedWindow(nullptr);

	if( m_pUserWindow )
		m_pUserWindow->onFocusLost();
}

//____ onMoved() ______________________________________________________________

void CocoaWindow::onMoved()
{
	if( !m_pUserWindow )
		return;

	NSRect content = [m_window contentRectForFrameRect:m_window.frame];
	m_pUserWindow->onMove({ pts(content.origin.x), pts(_topOfMainScreen() - NSMaxY(content)) });
}

//____ onMinimized() __________________________________________________________

void CocoaWindow::onMinimized()
{
	if( m_pUserWindow )
		m_pUserWindow->onMinimized();
}

//____ onRestored() ___________________________________________________________

void CocoaWindow::onRestored()
{
	if( m_pUserWindow )
		m_pUserWindow->onRestored();
}

//____ destroy() ______________________________________________________________
//
// Called when the user window is released, which can be from inside a callback
// of this very window. So we just hide it here and leave the deletion to the
// main loop, which looks at isDestroyed().

void CocoaWindow::destroy()
{
	if( m_bDestroyed )
		return;

	m_bDestroyed = true;

	if( m_pRootPanel && Base::inputHandler()->focusedWindow() == m_pRootPanel )
		Base::inputHandler()->setFocusedWindow(nullptr);

	[m_delegate detach];
	[m_window orderOut:nil];
}

//____ setGeo() _______________________________________________________________

wg::Rect CocoaWindow::setGeo(const wg::Rect& geo)
{
	NSRect content = NSMakeRect(geo.x, _topOfMainScreen() - geo.y - geo.h, geo.w, geo.h);
	[m_window setFrame:[m_window frameRectForContentRect:content] display:YES];
	return geo;
}

//____ requestFocus() _________________________________________________________

bool CocoaWindow::requestFocus()
{
	if( m_window.miniaturized )
		[m_window deminiaturize:nil];

	[m_window makeKeyAndOrderFront:nil];
	[NSApp activateIgnoringOtherApps:YES];
	return true;
}

//____ releaseFocus() _________________________________________________________

bool CocoaWindow::releaseFocus()
{
	//TODO: Implement!!!
	return false;
}

//____ minimize() _____________________________________________________________

bool CocoaWindow::minimize()
{
	[m_window miniaturize:nil];
	return true;
}

//____ restore() ______________________________________________________________

bool CocoaWindow::restore()
{
	[m_window deminiaturize:nil];
	return true;
}

//___ setTitle() ______________________________________________________________

bool CocoaWindow::setTitle(std::string& title)
{
	[m_window setTitle:[NSString stringWithUTF8String:title.c_str()]];
	return true;
}

//____ title() ________________________________________________________________

std::string CocoaWindow::title()
{
	return std::string(m_window.title.UTF8String);
}


//____ cocoaSetPointerStyle() _________________________________________________

void cocoaSetPointerStyle(PointerStyle style)
{
	NSCursor * pCursor;

	switch( style )
	{
		case PointerStyle::Ibeam:			pCursor = [NSCursor IBeamCursor];					break;
		case PointerStyle::Crosshair:		pCursor = [NSCursor crosshairCursor];				break;
		case PointerStyle::Hand:			pCursor = [NSCursor pointingHandCursor];			break;
		case PointerStyle::OpenHand:		pCursor = [NSCursor openHandCursor];				break;
		case PointerStyle::ClosedHand:		pCursor = [NSCursor closedHandCursor];				break;
		case PointerStyle::Stop:			pCursor = [NSCursor operationNotAllowedCursor];		break;
		case PointerStyle::ResizeNS:
		case PointerStyle::ResizeBeamNS:	pCursor = [NSCursor resizeUpDownCursor];			break;
		case PointerStyle::ResizeWE:
		case PointerStyle::ResizeBeamWE:	pCursor = [NSCursor resizeLeftRightCursor];			break;

		// AppKit has no public cursors for these.

		default:							pCursor = [NSCursor arrowCursor];					break;
	}

	[pCursor retain];
	[g_pCurrentCursor release];
	g_pCurrentCursor = pCursor;

	[pCursor set];
}

//____ cocoaLockPointer() _____________________________________________________

bool cocoaLockPointer()
{
	if( g_bPointerLocked )
		return true;

	// Start the virtual pointer where the real one is, in the key window.

	NSWindow * pKeyWindow = [NSApp keyWindow];
	if( pKeyWindow && pKeyWindow.contentView )
	{
		NSView * pView = pKeyWindow.contentView;
		NSPoint p = [pView convertPoint:[pKeyWindow mouseLocationOutsideOfEventStream] fromView:nil];
		g_lockedVirtualPos = Coord(p.x, p.y);
	}

	if( CGAssociateMouseAndMouseCursorPosition(false) != kCGErrorSuccess )
		return false;

	[NSCursor hide];
	g_bPointerLocked = true;
	return true;
}

//____ cocoaUnlockPointer() ___________________________________________________

bool cocoaUnlockPointer()
{
	if( !g_bPointerLocked )
		return true;

	// The cursor didn't move while locked, so it is already back where it was.

	CGAssociateMouseAndMouseCursorPosition(true);
	[NSCursor unhide];
	g_bPointerLocked = false;
	return true;
}
