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
#include <cocoaapi.h>
#include <cocoagfxbackend.h>

#include <wondergui.h>
#include <wg_debugger.h>
#include <widgetkits/wg_oldskool.h>

#include <Carbon/Carbon.h>		// For the kVK_* key codes only.

#include <algorithm>
#include <iostream>
#include <vector>

using namespace wg;
using namespace wapp;

//____ CocoaHostBridge _________________________________________________________

class CocoaHostBridge : public HostBridge
{
public:
	bool		hidePointer() override;
	bool		showPointer() override;

	bool		setPointerStyle( PointerStyle style ) override;

	bool		lockHidePointer() override;
	bool		unlockShowPointer() override;

	std::string	getClipboardText() override;
	bool		setClipboardText(const std::string& text) override;

	bool		requestFocus(uintptr_t windowRef) override;
	bool		yieldFocus(uintptr_t windowRef) override;
};

//____ WappMenuTarget __________________________________________________________

@interface WappMenuTarget : NSObject
- (void) quit:(id)sender;
@end


//______________________________________________________________________________

bool		init_system();
void		init_wondergui();
void		exit_wondergui();
bool		init_debugger(CocoaAPI * pAPI);
void		exit_debugger();
void		process_system_events();
void		delete_destroyed_windows();


WonderApp_p					g_pApp;

CocoaHostBridge *			g_pHostBridge = nullptr;
WappMenuTarget *			g_pMenuTarget = nil;

std::vector<CocoaWindow*>	g_windows;

wg::DebugFrontend_p			g_pDebugFrontend;
wg::DebugBackend_p			g_pDebugBackend;
wapp::Window_p				g_pDebugWindow;

int							g_argc = 0;
char**						g_argv = nullptr;


//____ main() _________________________________________________________________

int main(int argc, char *argv[] )
{
	@autoreleasepool {

		g_argc = argc;
		g_argv = argv;

		if( !init_system() )
			return -1;

		init_wondergui();

		if( !initGfxBackend() )
			return -1;

		// Create app and API visitor

		g_pApp = WonderApp::create();
		auto pAPI = new CocoaAPI();

		// Setup debugger

		init_debugger(pAPI);

		// Initialize the app.
		//
		// No events are processed until init() returns. A window shown before then
		// doesn't become key when we are activated, so it gets no mouse motion and
		// the first click only activates it. So we keep the windows hidden until
		// init() is done, then show them, which makes the last one key.

		CocoaWindow::s_bHoldShow = true;

		bool bContinue = g_pApp->init( pAPI );

		CocoaWindow::s_bHoldShow = false;

		for( auto pWin : g_windows )
			pWin->showIfHeld();

		[NSApp activateIgnoringOtherApps:YES];

		// Main loop

		while (bContinue)
		{
			@autoreleasepool {

				// Handle system events, waiting a little for them if there are none.

				process_system_events();

				// Update the app

				bContinue = g_pApp->update();

				// Dispatch messages

				Base::msgRouter()->dispatch();

				// Periodic update

				Base::update(cocoaTimestamp());

				// Delete windows whose user windows have been released.

				delete_destroyed_windows();

				// Render

				for( auto pWin : g_windows )
					pWin->render();
			}
		}

		// Exit the app

		g_pApp->exit();
		g_pApp = nullptr;

		Base::setErrorHandler(nullptr);		//TODO: WAPP-framework should have its own error-handler.

		delete pAPI;

		exit_debugger();
		delete_destroyed_windows();

		exitGfxBackend();
		exit_wondergui();
	}
	return 0;
}

//____ init_system() __________________________________________________________

bool init_system()
{
	[NSApplication sharedApplication];

	// We are not an app bundle, so we have to tell that we are a regular app with
	// a menu bar and Dock icon.

	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

	// Application menu, which is all we need for Hide and Quit.

	g_pMenuTarget = [[WappMenuTarget alloc] init];

	NSString * pAppName = [[NSProcessInfo processInfo] processName];

	NSMenu * pMainMenu = [[[NSMenu alloc] init] autorelease];
	NSMenuItem * pAppMenuItem = [[[NSMenuItem alloc] init] autorelease];
	[pMainMenu addItem:pAppMenuItem];

	NSMenu * pAppMenu = [[[NSMenu alloc] init] autorelease];
	[pAppMenu addItemWithTitle:[@"Hide " stringByAppendingString:pAppName] action:@selector(hide:) keyEquivalent:@"h"];

	NSMenuItem * pHideOthers = [pAppMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
	pHideOthers.keyEquivalentModifierMask = NSEventModifierFlagOption | NSEventModifierFlagCommand;

	[pAppMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];
	[pAppMenu addItem:[NSMenuItem separatorItem]];

	NSMenuItem * pQuit = [pAppMenu addItemWithTitle:[@"Quit " stringByAppendingString:pAppName] action:@selector(quit:) keyEquivalent:@"q"];
	pQuit.target = g_pMenuTarget;

	pAppMenuItem.submenu = pAppMenu;
	[NSApp setMainMenu:pMainMenu];

	[NSApp finishLaunching];
	return true;
}

//____ process_system_events() ________________________________________________

void process_system_events()
{
	NSDate * pUntil = [NSDate dateWithTimeIntervalSinceNow:0.01];

	while( NSEvent * pEvent = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:pUntil inMode:NSDefaultRunLoopMode dequeue:YES] )
	{
		// AppKit doesn't send key up events to the window while Command is held,
		// so we do it ourselves. Without it we would think the key is still pressed.

		if( pEvent.type == NSEventTypeKeyUp && (pEvent.modifierFlags & NSEventModifierFlagCommand) )
			[[NSApp keyWindow] sendEvent:pEvent];
		else
			[NSApp sendEvent:pEvent];

		pUntil = [NSDate distantPast];		// Just take what is already there from now on.
	}

	[NSApp updateWindows];
}

//____ delete_destroyed_windows() _____________________________________________

void delete_destroyed_windows()
{
	g_windows.erase(std::remove_if(g_windows.begin(), g_windows.end(), [](CocoaWindow* p)
	{
		if( p->isDestroyed() )
		{
			delete p;
			return true;
		}
		return false;
	}), g_windows.end());
}

//____ cocoaWindowCloseRequested() ____________________________________________

void cocoaWindowCloseRequested(CocoaWindow* pWindow)
{
	bool bClose = pWindow->userWindow()->onClose();
	if( bClose )
	{
		if( pWindow->userWindow() == g_pDebugWindow )
		{
			g_pDebugWindow = nullptr;
			g_pDebugFrontend->deactivate();
		}
		else if( g_pApp )
			g_pApp->closeWindow(pWindow->userWindow());
	}
}

//____ WappMenuTarget __________________________________________________________

@implementation WappMenuTarget

- (void) quit:(id)sender
{
	// WonderApps quit when they want to, so we ask all their windows to close,
	// just like when the user clicks their close buttons.

	auto windows = g_windows;

	for( auto pWin : windows )
	{
		if( !pWin->isDestroyed() && pWin->userWindow() != g_pDebugWindow )
			cocoaWindowCloseRequested(pWin);
	}
}

@end

//____ init_wondergui() _______________________________________________________

void init_wondergui()
{
	g_pHostBridge = new CocoaHostBridge();

	Base::init(g_pHostBridge);

	Base::setErrorHandler([](Error& e)
	{
		std::cerr << "ERROR:" << e.message << std::endl;
	});

	InputHandler_p pInput = Base::inputHandler();

	pInput->mapKey(kVK_LeftArrow, Key::Left);
	pInput->mapKey(kVK_RightArrow, Key::Right);
	pInput->mapKey(kVK_UpArrow, Key::Up);
	pInput->mapKey(kVK_DownArrow, Key::Down);
	pInput->mapKey(kVK_Delete, Key::Backspace);
	pInput->mapKey(kVK_ForwardDelete, Key::Delete);
	pInput->mapKey(kVK_End, Key::End);
	pInput->mapKey(kVK_Escape, Key::Escape);
	pInput->mapKey(kVK_Home, Key::Home);
	pInput->mapKey(kVK_PageDown, Key::PageDown);
	pInput->mapKey(kVK_PageUp, Key::PageUp);
	pInput->mapKey(kVK_Return, Key::Return);
	pInput->mapKey(kVK_ANSI_KeypadEnter, Key::Return);
	pInput->mapKey(kVK_Space, Key::Space);
	pInput->mapKey(kVK_Tab, Key::Tab);
	pInput->mapKey(kVK_F1, Key::F1);
	pInput->mapKey(kVK_F2, Key::F2);
	pInput->mapKey(kVK_F3, Key::F3);
	pInput->mapKey(kVK_F4, Key::F4);
	pInput->mapKey(kVK_F5, Key::F5);
	pInput->mapKey(kVK_F6, Key::F6);
	pInput->mapKey(kVK_F7, Key::F7);
	pInput->mapKey(kVK_F8, Key::F8);
	pInput->mapKey(kVK_F9, Key::F9);
	pInput->mapKey(kVK_F10, Key::F10);
	pInput->mapKey(kVK_F11, Key::F11);
	pInput->mapKey(kVK_F12, Key::F12);

	pInput->mapKey(kVK_Shift, Key::Shift);
	pInput->mapKey(kVK_RightShift, Key::Shift);

	pInput->mapKey(kVK_Option, Key::Alt);
	pInput->mapKey(kVK_RightOption, Key::Alt);

	pInput->mapKey(kVK_Control, Key::MacControl);
	pInput->mapKey(kVK_RightControl, Key::MacControl);

	pInput->mapKey(kVK_Command, Key::Command);
	pInput->mapKey(kVK_RightCommand, Key::Command);

	pInput->mapCommand(kVK_ANSI_X, ModKeys::Command, EditCmd::Cut);
	pInput->mapCommand(kVK_ANSI_C, ModKeys::Command, EditCmd::Copy);
	pInput->mapCommand(kVK_ANSI_V, ModKeys::Command, EditCmd::Paste);

	pInput->mapCommand(kVK_ANSI_A, ModKeys::Command, EditCmd::SelectAll);

	pInput->mapCommand(kVK_ANSI_Z, ModKeys::Command, EditCmd::Undo);
	pInput->mapCommand(kVK_ANSI_Z, ModKeys::CommandShift, EditCmd::Redo);

	pInput->mapCommand(kVK_Escape, ModKeys::None, EditCmd::Escape);

	// Key repeat as set in System Settings.

	pInput->setKeyRepeat( int(NSEvent.keyRepeatDelay * 1000000), int(NSEvent.keyRepeatInterval * 1000000) );

	//TODO: Set the double click time from NSEvent.doubleClickInterval once
	// InputHandler::setDoubleClickTresholds() can be linked (it is declared inline
	// but defined in wg_inputhandler.cpp).
}

//____ exit_wondergui() _______________________________________________________

void exit_wondergui()
{
	wkit::exit();
	Base::exit();
	delete g_pHostBridge;
	g_pHostBridge = nullptr;
}

//____ init_debugger() ________________________________________________________

bool init_debugger(CocoaAPI* pAPI)
{
	pAPI->initDefaultWidgetKit();

	g_pDebugBackend = DebugBackend::create();

	g_pDebugFrontend = WGCREATE(DebugFrontend, _.backend = g_pDebugBackend );

	Base::msgRouter()->addRoute(MsgType::KeyPress, [pAPI](Msg * _pMsg) {

		KeyPressMsg * pMsg = static_cast<KeyPressMsg*>(_pMsg);

		if( pMsg->translatedKeyCode() == Key::F12 && (pMsg->modKeys() == ModKeys::MacCtrlAlt || pMsg->modKeys() == ModKeys::StdCtrlAlt) )
		{
			if( !g_pDebugWindow )
			{
				wg::Size size = g_pDebugFrontend->size();		// In points, its spx size depends on the scale of the window it was last in.

				auto pWindow = wapp::Window::create(pAPI, { .debug = false, .size = size, .title = "Debugger"  });
				g_pDebugWindow = pWindow;

				pWindow->mainCapsule()->slot = g_pDebugFrontend;
				g_pDebugFrontend->activate();
			}
			else
			{
				g_pDebugWindow = nullptr;
				g_pDebugFrontend->deactivate();
			}
		}

	});

	return true;
}

//____ exit_debugger() ________________________________________________________

void exit_debugger()
{
	g_pDebugWindow = nullptr;
	g_pDebugFrontend = nullptr;
	g_pDebugBackend = nullptr;
}

//____ hidePointer() __________________________________________________________

bool CocoaHostBridge::hidePointer()
{
	[NSCursor hide];
	return true;
}

//____ showPointer() __________________________________________________________

bool CocoaHostBridge::showPointer()
{
	[NSCursor unhide];
	return true;
}

//____ setPointerStyle() ______________________________________________________

bool CocoaHostBridge::setPointerStyle( PointerStyle style )
{
	cocoaSetPointerStyle(style);
	return true;
}

//____ lockHidePointer() ______________________________________________________

bool CocoaHostBridge::lockHidePointer()
{
	return cocoaLockPointer();
}

//____ unlockShowPointer() ____________________________________________________

bool CocoaHostBridge::unlockShowPointer()
{
	return cocoaUnlockPointer();
}

//____ getClipboardText() _____________________________________________________

std::string CocoaHostBridge::getClipboardText()
{
	NSString * pText = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
	return pText ? std::string(pText.UTF8String) : std::string();
}

//____ setClipboardText() _____________________________________________________

bool CocoaHostBridge::setClipboardText(const std::string& text )
{
	NSPasteboard * pBoard = [NSPasteboard generalPasteboard];
	[pBoard clearContents];
	return [pBoard setString:[NSString stringWithUTF8String:text.c_str()] forType:NSPasteboardTypeString];
}

//____ requestFocus() _________________________________________________________

bool CocoaHostBridge::requestFocus(uintptr_t windowRef)
{
	auto pWindow = reinterpret_cast<CocoaWindow*>(windowRef);

	if( !pWindow || pWindow->isDestroyed() )
		return false;

	NSWindow * pNSWindow = pWindow->nsWindow();

	if( pNSWindow.miniaturized )
		[pNSWindow deminiaturize:nil];

	[pNSWindow makeKeyAndOrderFront:nil];
	[NSApp activateIgnoringOtherApps:YES];
	return true;
}

//____ yieldFocus() _________________________________________________________

bool CocoaHostBridge::yieldFocus(uintptr_t windowRef)
{
	return false;		// Window still has focus.
}
