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
#include <wondergui.h>
#include <wg_debugger.h>
#include <wg_freetypefont.h>
#include <themes/simplistic/wg_simplistic.h>

#ifdef WIN32
#	include <SDL.h>
#	include <SDL_image.h>
#	include <Windows.h>
#	include <libloaderapi.h>
#elif __APPLE__
#	include <SDL2/SDL.h>
#	include <SDL2_image/SDL_image.h>
#	include <dlfcn.h>
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_image.h>
#	include <dlfcn.h>
#endif


#include <fstream>
#include <iostream>
#include <mutex>

#include "sdlwindow.h"
#include "tinyfiledialogs.h"

using namespace wg;

//____ MyAppVisitor ________________________________________________________

class MyAppVisitor : public WonderApp::Visitor
{
public:

	std::vector<std::string> programArguments() const override;

	int64_t			time() override
	{
		return SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency();
	}

	Blob_p 			loadBlob(const std::string& path) override;
	Surface_p 		loadSurface(const std::string& path, SurfaceFactory* pFactory = nullptr, const Surface::Blueprint& bp = Surface::Blueprint() ) override;

	Theme_p			initDefaultTheme() override;

	bool			notifyPopup(const std::string& title, const std::string& message, WonderApp::IconType iconType) override;

	WonderApp::DialogButton	messageBox(	const std::string& title, const std::string& message, WonderApp::DialogType dialogType,
										WonderApp::IconType iconType, WonderApp::DialogButton defaultButton = WonderApp::DialogButton::Undefined) override;

	std::string		inputBox( const std::string& title, const std::string& message, const std::string& defaultInput) override;

	std::string		saveFileDialog(	const std::string& title, const std::string& defaultPathAndFile,
									const std::vector<std::string>& filterPatterns,
									const std::string& singleFilterDescription) override;

	std::string		openFileDialog( const std::string& title, const std::string& defaultPathAndFile,
									const std::vector<std::string>& filterPatterns,
									const std::string& singleFilterDescription) override;

	std::vector<std::string> openMultiFileDialog(	const std::string& title, const std::string& defaultPathAndFile,
													const std::vector<std::string>& filterPatterns,
													const std::string& singleFilterDescription) override;

	std::string		selectFolderDialog(const std::string& title, const std::string& defaultPath) override;
	
	Window_p	createWindow(const Window::Blueprint& blueprint) override;

	WonderApp::LibId	openLibrary(const std::string& path) override;
	void*			loadSymbol(WonderApp::LibId lib, const std::string& symbol) override;
	bool			closeLibrary(WonderApp::LibId lib) override;

	std::string		resourceDirectory() override;

protected:
	void			convertSDLFormat(PixelDescription* pWGFormat, const SDL_PixelFormat* pSDLFormat);

};

//____ MyHostBridge ___________________________________________________________

class MyHostBridge : public HostBridge
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


//______________________________________________________________________________

bool		init_system();
void		exit_system();

bool		process_system_events();

bool		init_wondergui();
void		exit_wondergui();

bool		backend_specific_init();
void		backend_specific_exit();


bool		init_debugger(MyAppVisitor * pVisitor);
void		exit_debugger();


MouseButton translateSDLMouseButton(Uint8 button);

Surface_p generateWindowSurface(SDL_Window* pWindow, int widht, int height);


float				g_scale;

MyHostBridge *		g_pHostBridge = nullptr;

wg::DebugFrontend_p	g_pDebugFrontend;
wg::DebugBackend_p	g_pDebugBackend;

Window_p			g_pDebugWindow;

std::vector<SDLWindow_wp>	g_windows;

int			g_argc = 0;
char**		g_argv = nullptr;


static const char * iconNames[4] = { "info", "warning", "error", "question" };
static const char * dialogNames[4] = { "ok", "okcancel", "yesno", "yesnocancel" };


std::mutex g_winResizeEventMutex;

SDL_Window*	g_pFocusedWindow = nullptr;

CoordI		g_lockedPointerPos;
CoordI		g_relModeVirtualPos;

SDL_Cursor * g_mousePointers[PointerStyle_size];



//____ main() _________________________________________________________________

int main(int argc, char *argv[] )
{
	g_argc = argc;
	g_argv = argv;

	g_scale = 1.f;

	if (!init_system() )
		return -1;
	
	if (!init_wondergui() )
		return -1;

	if( !backend_specific_init() )
		return -1;


	// Create app and visitor, make any app-specific initialization

	auto pApp = WonderApp::create();
	auto pVisitor = new MyAppVisitor();

	// Setup debugger

	init_debugger(pVisitor);

	// Initialize the app

	bool bContinue = pApp->init( pVisitor );

	// Main loop

	while (bContinue)
	{
		// Handle system events

		bContinue = process_system_events();

		if (bContinue)
		{
			// Don't collide with eventWatcher.
			
			const std::lock_guard<std::mutex> lock(g_winResizeEventMutex);

			// Uppdate the app

			bContinue = pApp->update();

			// Dispatch messages

			Base::msgRouter()->dispatch();

			// Periodic update

			Base::update(SDL_GetPerformanceCounter() * 1000000 / SDL_GetPerformanceFrequency());

			// Remove any windows pointers for erased windows.

			g_windows.erase(std::remove(g_windows.begin(), g_windows.end(), nullptr), g_windows.end());


			// Render. We do this outside the app since we might want to 
			// handle updated rectangles in a system specific way.

			for (auto& pWin : g_windows )
			{
				pWin->render();
			}

			// Sleep for a while

			SDL_Delay(10);
		}
	}

	// Exit the app

	pApp->exit();

	//

	pApp = nullptr;
	
	Base::setErrorHandler(nullptr);		//TODO: WAPP-framework should have its own error-handler.

	g_windows.clear();

	delete pVisitor;

	exit_debugger();
	backend_specific_exit();
	exit_wondergui();
	exit_system();
	return 0;
}


//____ init_wondergui() _______________________________________________________

bool init_wondergui()
{
	g_pHostBridge = new MyHostBridge();
	
	Base::init(g_pHostBridge);

	Base::setErrorHandler([](Error& e) 
	{ 
		std::cerr << "ERROR:" << e.message << std::endl;  
	});

	Base::setDefaultToSRGB(true);

	InputHandler_p pInput = Base::inputHandler();
	
	pInput->mapKey(SDLK_LEFT, Key::Left);
	pInput->mapKey(SDLK_RIGHT, Key::Right);
	pInput->mapKey(SDLK_UP, Key::Up);
	pInput->mapKey(SDLK_DOWN, Key::Down);
	pInput->mapKey(SDLK_BACKSPACE, Key::Backspace);
	pInput->mapKey(SDLK_DELETE, Key::Delete);
	pInput->mapKey(SDLK_END, Key::End);
	pInput->mapKey(SDLK_ESCAPE, Key::Escape);
	pInput->mapKey(SDLK_HOME, Key::Home);
	pInput->mapKey(SDLK_PAGEDOWN, Key::PageDown);
	pInput->mapKey(SDLK_PAGEUP, Key::PageUp);
	pInput->mapKey(SDLK_RETURN, Key::Return);
	pInput->mapKey(SDLK_SPACE, Key::Space);
	pInput->mapKey(SDLK_TAB, Key::Tab);
	pInput->mapKey(SDLK_F1, Key::F1);
	pInput->mapKey(SDLK_F2, Key::F2);
	pInput->mapKey(SDLK_F3, Key::F3);
	pInput->mapKey(SDLK_F4, Key::F4);
	pInput->mapKey(SDLK_F5, Key::F5);
	pInput->mapKey(SDLK_F6, Key::F6);
	pInput->mapKey(SDLK_F7, Key::F7);
	pInput->mapKey(SDLK_F8, Key::F8);
	pInput->mapKey(SDLK_F9, Key::F9);
	pInput->mapKey(SDLK_F10, Key::F10);
	pInput->mapKey(SDLK_F11, Key::F11);
	pInput->mapKey(SDLK_F12, Key::F12);


	pInput->mapKey(SDLK_LSHIFT, Key::Shift);
	pInput->mapKey(SDLK_RSHIFT, Key::Shift);

	pInput->mapKey(SDLK_LALT, Key::Alt);
	pInput->mapKey(SDLK_RALT, Key::Alt);

#ifdef __APPLE__
	pInput->mapKey(SDLK_LCTRL, Key::MacControl);
	pInput->mapKey(SDLK_RCTRL, Key::MacControl);

	pInput->mapKey(SDLK_LGUI, Key::Command);
	pInput->mapKey(SDLK_RGUI, Key::Command);
#else
	pInput->mapKey(SDLK_LCTRL, Key::StdControl);
	pInput->mapKey(SDLK_RCTRL, Key::StdControl);

	pInput->mapKey(SDLK_LGUI, Key::OSKey);
	pInput->mapKey(SDLK_RGUI, Key::OSKey);

#endif
	
	pInput->mapKey(SDLK_KP_ENTER, Key::Return);


	pInput->mapCommand(SDLK_x, ModKeys::Command, EditCmd::Cut);
	pInput->mapCommand(SDLK_c, ModKeys::Command, EditCmd::Copy);
	pInput->mapCommand(SDLK_v, ModKeys::Command, EditCmd::Paste);

	pInput->mapCommand(SDLK_a, ModKeys::Command, EditCmd::SelectAll);

	pInput->mapCommand(SDLK_z, ModKeys::Command, EditCmd::Undo);
	pInput->mapCommand(SDLK_z, ModKeys::CommandShift, EditCmd::Redo);
	
	pInput->mapCommand(SDLK_ESCAPE, ModKeys::None, EditCmd::Escape);
	
//	g_pRoot->setDebugMode(true);

	return true;
}

//____ exit_wondergui() _______________________________________________________

void exit_wondergui()
{
	Base::exit();
	delete g_pHostBridge;
	g_pHostBridge = nullptr;
}

//____ init_debugger() ________________________________________________________

bool init_debugger(MyAppVisitor* pAPI)
{
	auto pTheme = pAPI->initDefaultTheme();
	auto pIconSurface = pAPI->loadSurface("resources/debugger_gfx.png");
	auto pTransparencyGrid = pAPI->loadSurface("resources/checkboardtile.png", nullptr, { .tiling = true } );

	if( !pTheme || !pIconSurface || !pTransparencyGrid )
		return false;

	g_pDebugBackend = DebugBackend::create();

	g_pDebugFrontend = WGCREATE(DebugFrontend, _.backend = g_pDebugBackend, _.theme = pTheme, _.icons = pIconSurface, _.transparencyGrid = pTransparencyGrid );

	Base::msgRouter()->addRoute(MsgType::KeyPress, [pAPI](Msg * _pMsg) {

		KeyPressMsg * pMsg = static_cast<KeyPressMsg*>(_pMsg);

		if( pMsg->translatedKeyCode() == Key::F12 && (pMsg->modKeys() == ModKeys::MacCtrlAlt || pMsg->modKeys() == ModKeys::StdCtrlAlt) )
		{
			if( !g_pDebugWindow )
			{
				auto pFocusedWindow = g_pFocusedWindow;
				SizeI size = g_pDebugFrontend->spxSize() / 64;

				auto pWindow = SDLWindow::create({ .size = Size(size), .title = "Debugger"  });
				g_windows.push_back(pWindow.rawPtr());
				g_pDebugWindow = pWindow;

				pWindow->setCloseRequestHandler([](void) {

					g_pDebugWindow = nullptr;
					g_pDebugFrontend->deactivate();
					return true;
				});

				pWindow->setContent(g_pDebugFrontend);
				g_pDebugFrontend->activate();
			}
		}

	});

	return true;
}

//____ exitDebugger() _________________________________________________________

void exit_debugger()
{
	g_pDebugWindow = nullptr;
	g_pDebugFrontend = nullptr;
	g_pDebugBackend = nullptr;
}

//____ eventWatcher() _________________________________________________________

int eventWatcher(void * pNull, SDL_Event* pEvent)
{
	//WARNING! This function could potentially run in another thread!
	
	if (pEvent->type == SDL_WINDOWEVENT && pEvent->window.event == SDL_WINDOWEVENT_RESIZED)
	{
		const std::lock_guard<std::mutex> lock(g_winResizeEventMutex);

		for( int i = 0 ; i < g_windows.size() ; i++ )
		{
			if( g_windows[i] && g_windows[i]->SDLWindowId() == pEvent->window.windowID )
			{
				SDLWindow * pWindow = g_windows[i];

                pWindow->onWindowSizeUpdated(pEvent->window.data1, pEvent->window.data2);
				pWindow->render();
				break;
			}
		}
	}
	return 0;
}

//____ init_system() _______________________________________________________

bool init_system()
{
	for( int i = 0 ; i < PointerStyle_size ; i++ )
		g_mousePointers[i] = NULL;

	
	// initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

	SDL_AddEventWatch(eventWatcher, nullptr);
	
	// make sure SDL cleans up before exit
	atexit(SDL_Quit);
 
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

	return true;
}

//____ exit_system() __________________________________________________________

void exit_system()
{
//	SDL_DelEventWatch(eventWatcher, nullptr);

	for( int i = 0 ; i < PointerStyle_size ; i++ )
	{
		if( g_mousePointers[i] != NULL )
			SDL_FreeCursor(g_mousePointers[i]);
	}
	
	IMG_Quit();
}

//____ update_window_rects() __________________________________________________
/*
void update_window_rects(const Rect* pRects, int nRects)
{
	if (nRects == 0)
		return;

	std::vector<SDL_Rect>	rects;

	for (int i = 0; i < nRects; i++)
	{
		SDL_Rect r = { (int) pRects[i].x, (int) pRects[i].y, (int) pRects[i].w, (int) pRects[i].h };
		rects.push_back(r);
	}

	SDL_UpdateWindowSurfaceRects(g_pSDLWindow, &rects.front(), nRects);
}
*/


//____ process_system_events() ________________________________________________

bool process_system_events()
{
/*
	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type)
		{
		case SDL_QUIT:
			return false;
		default:
			break;
		}
	}
*/

	// Process all the SDL events in a loop

	InputHandler_p pInput = Base::inputHandler();

	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type)
		{
		case SDL_QUIT:
			return false;

		case SDL_MOUSEMOTION:
		{
			RootPanel* pRootPanel = nullptr;

			for (auto& wpWindow : g_windows)
			{
				if (wpWindow == nullptr)
					continue;

				SDLWindow* pWin = wpWindow;
				if (pWin->SDLWindowId() == e.motion.windowID)
				{
					pRootPanel = pWin->rootPanel();
					break;
				}

			}

		
			if (SDL_GetRelativeMouseMode() && g_pFocusedWindow)
			{
				g_relModeVirtualPos.x += e.motion.xrel;
				g_relModeVirtualPos.y += e.motion.yrel;

				pInput->setPointer(pRootPanel, { float(g_relModeVirtualPos.x), float(g_relModeVirtualPos.y) } );
			}
			else
				pInput->setPointer(pRootPanel, Coord(e.motion.x, e.motion.y));

			break;
		}

		case SDL_MOUSEBUTTONDOWN:
			pInput->setButton(translateSDLMouseButton(e.button.button), true);
			break;

		case SDL_MOUSEBUTTONUP:
			pInput->setButton(translateSDLMouseButton(e.button.button), false);
			break;

		case SDL_MOUSEWHEEL:
		{
			bool bInvertScroll = false;
			Coord distance(e.wheel.x, e.wheel.y);
			if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
			{
				bInvertScroll = true;
				distance *= -1;
			}

			if (e.wheel.y != 0)
				pInput->setWheelRoll(1, distance.y, bInvertScroll);

			if (e.wheel.x != 0)
				pInput->setWheelRoll(2, distance.x, bInvertScroll);
			break;
		}

		case SDL_KEYDOWN:
		{
			pInput->setKey(e.key.keysym.sym, true);
			break;
		}

		case SDL_KEYUP:
		{
			pInput->setKey(e.key.keysym.sym, false);
			break;
		}

		case SDL_TEXTINPUT:
			pInput->putText(e.text.text);
			break;

		case SDL_WINDOWEVENT:
		{
			SDLWindow* pWindow = nullptr;

			for (auto& pWin : g_windows)
			{
				if (pWin && e.window.windowID == pWin->SDLWindowId())
				{
					pWindow = pWin;
					break;
				}
			}
			if (pWindow == nullptr)
				break;

			switch (e.window.event)
			{
				// See https://stackoverflow.com/questions/32294913/getting-continuous-window-resize-event-in-sdl-2
				// for how to do continous resize.

				case SDL_WINDOWEVENT_SIZE_CHANGED:		// Called for all in-between sizes.			
				{
					break;
				}

				case SDL_WINDOWEVENT_RESIZED:			// Called for final size.
				{
					Size size(e.window.data1, e.window.data2);

					if (size != pWindow->rootPanel()->geo().size())
                        pWindow->onWindowSizeUpdated( e.window.data1, e.window.data2 );

                    break;
				}
					
				case SDL_WINDOWEVENT_CLOSE:
				{
					pWindow->retain();			// We want to delay window destruction until _onCloseRequest() has returned.
					bool bClose = pWindow->_onCloseRequest();
					pWindow->release();
					break;
				}

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					g_pFocusedWindow = pWindow->SDLWindowPtr();
					Base::inputHandler()->setFocusedWindow(pWindow->rootPanel());
					break;
				}

				case SDL_WINDOWEVENT_FOCUS_LOST:
				{
					g_pFocusedWindow = nullptr;
					Base::inputHandler()->setFocusedWindow(nullptr);
					break;
				}

					
				default:
					break;
			}
			break;
		}



		default:
			break;
		}
	}

	return true;
}

//____ translateSDLMouseButton() __________________________________________________
//
// Translate SDL mouse button enums to WonderGUI equivalents.
//
MouseButton translateSDLMouseButton(Uint8 button)
{
	switch (button)
	{
	default:
	case SDL_BUTTON_LEFT:
		return MouseButton::Left;
	case SDL_BUTTON_MIDDLE:
		return MouseButton::Middle;
	case SDL_BUTTON_RIGHT:
		return MouseButton::Right;
	case SDL_BUTTON_X1:
		return MouseButton::X1;
	case SDL_BUTTON_X2:
		return MouseButton::X2;
	}
}

//____ convertSDLFormat() ______________________________________________________

void MyAppVisitor::convertSDLFormat(PixelDescription* pWGFormat, const SDL_PixelFormat* pSDLFormat)
{
	if( (pSDLFormat->palette != nullptr) )
		pWGFormat->type = PixelType::Index;
	else
		pWGFormat->type = PixelType::Chunky;

	pWGFormat->bits = pSDLFormat->BitsPerPixel;
	
	pWGFormat->R_mask = pSDLFormat->Rmask;
	pWGFormat->G_mask = pSDLFormat->Gmask;
	pWGFormat->B_mask = pSDLFormat->Bmask;
	pWGFormat->A_mask = pSDLFormat->Amask;
}

//____ programArguments() _________________________________________________

std::vector<std::string> MyAppVisitor::programArguments() const
{
	std::vector<std::string>	args;

	for (int i = 1; i < g_argc; i++)
		args.push_back(g_argv[i]);

	return args;
}

//____ loadBlob() _________________________________________________________

Blob_p MyAppVisitor::loadBlob(const std::string& path)
{
	FILE* fp;

#ifdef WIN32
	errno_t err = fopen_s(&fp, path.c_str(), "rb");
#else
	fp = fopen(path.c_str(), "rb");
#endif
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	int size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	Blob_p pBlob = Blob::create(size);

	int nRead = (int)fread(pBlob->data(), 1, size, fp);
	fclose(fp);

	if (nRead < size)
		return 0;

	return pBlob;
}

//____ loadSurface() ______________________________________________________

Surface_p MyAppVisitor::loadSurface(const std::string& path, SurfaceFactory* pFactory, const Surface::Blueprint& _bp)
{
	if (path.rfind(".surf") == path.size() - 5 || path.rfind(".srf") == path.size() - 4)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.good())
			return nullptr;

		auto pReader = SurfaceReader::create({ .factory = Base::defaultSurfaceFactory() });
		Surface_p pSurface = pReader->readSurfaceFromStream(input, _bp);
		input.close();
		return pSurface;
	}
	else
	{
		Surface::Blueprint bp = _bp;

		PixelDescription format;

		auto pSDLSurf = IMG_Load(path.c_str());
		if (pSDLSurf == NULL)
			return nullptr;
		convertSDLFormat(&format, pSDLSurf->format);

		PixelFormat px;
		Color8* pPalette = nullptr;

		Color8 palette[256];

		if (format.type == PixelType::Index)
		{
			px = PixelFormat::Index_8;

			for (int i = 0; i < 256; i++)
			{
				palette[i].r = pSDLSurf->format->palette->colors[i].r;
				palette[i].g = pSDLSurf->format->palette->colors[i].g;
				palette[i].b = pSDLSurf->format->palette->colors[i].b;
				palette[i].a = pSDLSurf->format->palette->colors[i].a;
			}
			pPalette = palette;

			// Special copying of indexed pixels until copyPixels() supports indexed source.

			if (!pFactory)
				pFactory = Base::defaultSurfaceFactory();

			bp.format = px;
			bp.palette = pPalette;
			bp.size = { pSDLSurf->w, pSDLSurf->h };

			auto pSurface = pFactory->createSurface(bp);

			auto pixBuf = pSurface->allocPixelBuffer();

			auto pDest = pixBuf.pixels;
			uint8_t * pSrc = (uint8_t*) pSDLSurf->pixels;

			for(int y = 0; y < pSDLSurf->h; y++)
			{
				memcpy(pDest, pSrc, pSDLSurf->w);
				pDest += pixBuf.pitch;
				pSrc += pSDLSurf->pitch;
			}

			pSurface->pullPixels(pixBuf);
			pSurface->freePixelBuffer(pixBuf);

			SDL_FreeSurface(pSDLSurf);
			return pSurface;
		}

		else if (format.A_mask > 0)
			px = PixelFormat::BGRA_8;
		else
			px = PixelFormat::BGRX_8;
		
		if (!pFactory)
			pFactory = Base::defaultSurfaceFactory();

		bp.format = px;
		bp.palette = pPalette;
		bp.size = { pSDLSurf->w, pSDLSurf->h };

		auto pSurface = pFactory->createSurface(bp, (unsigned char*)pSDLSurf->pixels, format, pSDLSurf->pitch);
		SDL_FreeSurface(pSDLSurf);
		return pSurface;
	}
	
	
}

//____ initDefaultTheme() ____________________________________________________

Theme_p MyAppVisitor::initDefaultTheme()
{
	// Create the default theme, which is a simplistic theme.
	
	auto pFont1Blob = loadBlob("resources/NotoSans-Regular.ttf");
	auto pFont2Blob = loadBlob("resources/NotoSans-Bold.ttf");
	auto pFont3Blob = loadBlob("resources/NotoSans-Italic.ttf");
	auto pFont4Blob = loadBlob("resources/DroidSansMono.ttf");

	auto pFont1 = FreeTypeFont::create(pFont1Blob);
	auto pFont2 = FreeTypeFont::create(pFont2Blob);
	auto pFont3 = FreeTypeFont::create(pFont3Blob);
	auto pFont4 = FreeTypeFont::create(pFont4Blob);

	auto pThemeSurface = loadSurface("resources/skin_widgets.png");

	auto pTheme = Simplistic::create(pFont1,pFont2,pFont3,pFont4,pThemeSurface);
	if (!pTheme)
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create default theme", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return nullptr;
	}
	Base::setDefaultTheme(pTheme);
	Base::setDefaultStyle(pTheme->defaultStyle());


	return pTheme;
}



//____ notifyPopup() __________________________________________________________

bool MyAppVisitor::notifyPopup(const std::string& title, const std::string& message, WonderApp::IconType iconType)
{
	if( iconType == WonderApp::IconType::Question )
		return false;
	
	tinyfd_notifyPopup( title.c_str(), message.c_str(), iconNames[int(iconType)]);
	return true;
}

//____ messageBox() ___________________________________________________________

WonderApp::DialogButton	MyAppVisitor::messageBox(	const std::string& title, const std::string& message, 
													WonderApp::DialogType dialogType, WonderApp::IconType iconType, 
													WonderApp::DialogButton defaultButton)
{
	int defaultButtonInt = 0;
	
	switch( dialogType )
	{
		case::WonderApp::DialogType::Ok:
		{
			if( defaultButton != WonderApp::DialogButton::Ok && defaultButton != WonderApp::DialogButton::Undefined )
				goto wrongDefaultButton;
			
			defaultButtonInt = 1;
			break;
		}

		case::WonderApp::DialogType::OkCancel:
		{
			if (defaultButton == WonderApp::DialogButton::Ok)
				defaultButtonInt = 1;
			else if (defaultButton != WonderApp::DialogButton::Cancel && defaultButton != WonderApp::DialogButton::Undefined)
				goto wrongDefaultButton;
			break;
		}

		case::WonderApp::DialogType::YesNo:
		{
			if (defaultButton == WonderApp::DialogButton::Yes)
				defaultButtonInt = 1;
			else if (defaultButton != WonderApp::DialogButton::No && defaultButton != WonderApp::DialogButton::Undefined)
				goto wrongDefaultButton;
			break;
		}

		case::WonderApp::DialogType::YesNoCancel:
		{
			if (defaultButton == WonderApp::DialogButton::Yes)
				defaultButtonInt = 1;
			else if (defaultButton == WonderApp::DialogButton::No)
				defaultButtonInt = 2;
			else if( defaultButton != WonderApp::DialogButton::Cancel && defaultButton != WonderApp::DialogButton::Undefined )
				goto wrongDefaultButton;
			break;
		}
	}
	
	{
		int retval = tinyfd_messageBox( title.c_str(), message.c_str(), dialogNames[int(dialogType)], iconNames[int(iconType)], defaultButtonInt);
		WonderApp::DialogButton selectedButton;
		
		switch( dialogType )
		{
			case::WonderApp::DialogType::Ok:
			{
				selectedButton = WonderApp::DialogButton::Ok;
				break;
			}

			case::WonderApp::DialogType::OkCancel:
			{
				selectedButton = retval == 1 ? WonderApp::DialogButton::Ok : WonderApp::DialogButton::Cancel;
				break;
			}

			case::WonderApp::DialogType::YesNo:
			{
				selectedButton = retval == 1 ? WonderApp::DialogButton::Yes : WonderApp::DialogButton::No;
				break;
			}

			default:
			case::WonderApp::DialogType::YesNoCancel:
			{
				if( retval == 0 )
					selectedButton = WonderApp::DialogButton::No;
				else if( retval == 1 )
					selectedButton = WonderApp::DialogButton::Yes;
				else
					selectedButton = WonderApp::DialogButton::Cancel;
				break;
			}
		}
		
		return selectedButton;
	}
	

wrongDefaultButton:
	
	Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Default button isn't a button of specified dialog type", nullptr, nullptr, __func__, __FILE__, __LINE__);
	
	return WonderApp::DialogButton::Undefined;

}

//____ inputBox() _____________________________________________________________

std::string MyAppVisitor::inputBox(const std::string& title, const std::string& message, const std::string& defaultInput)
{
	return tinyfd_inputBox( title.c_str(), message.c_str(), defaultInput.c_str() );
}

//____ saveFileDialog() _______________________________________________________

std::string MyAppVisitor::saveFileDialog(	const std::string& title, const std::string& defaultPathAndFile,
											const std::vector<std::string>& filterPatterns,
											const std::string& singleFilterDescription)
{
	const char* pPatternPointers[64];
	const char** pPatterns = filterPatterns.empty() ? NULL : pPatternPointers;

	for (int i = 0; i < filterPatterns.size(); i++)
		pPatternPointers[i] = filterPatterns[i].c_str();

	auto pResult = tinyfd_saveFileDialog( title.c_str(), defaultPathAndFile.c_str(), int(filterPatterns.size()), pPatterns, singleFilterDescription.c_str() );
	
	if(pResult == NULL)
		return std::string();
	else
		return std::string(pResult);
}

//____ openFileDialog() _______________________________________________________

std::string MyAppVisitor::openFileDialog(	const std::string& title, const std::string& defaultPathAndFile,
											const std::vector<std::string>& filterPatterns,
											const std::string& singleFilterDescription)
{
	const char* pPatternPointers[64];
	const char** pPatterns = filterPatterns.empty() ? NULL : pPatternPointers;

	for (int i = 0; i < filterPatterns.size(); i++)
		pPatternPointers[i] = filterPatterns[i].c_str();

	auto pResult = tinyfd_openFileDialog( title.c_str(), defaultPathAndFile.c_str(), int(filterPatterns.size()), pPatterns, singleFilterDescription.c_str(), false);

	if (pResult == NULL)
		return std::string();
	else
		return std::string(pResult);
}

//____ openMultiFileDialog() _______________________________________________________

std::vector<std::string> MyAppVisitor::openMultiFileDialog(	const std::string& title, 
															const std::string& defaultPathAndFile,
															const std::vector<std::string>& filterPatterns,
															const std::string& singleFilterDescription)
{
	const char* pPatternPointers[64];
	const char** pPatterns = filterPatterns.empty() ? NULL : pPatternPointers;

	for (int i = 0; i < filterPatterns.size(); i++)
		pPatternPointers[i] = filterPatterns[i].c_str();

	auto pResult = tinyfd_openFileDialog(title.c_str(), defaultPathAndFile.c_str(), int(filterPatterns.size()), pPatterns, singleFilterDescription.c_str(), true);

	if (pResult == NULL)
		return std::vector<std::string>();
	else
	{
		std::vector<std::string> paths;

		char* pBeg = pResult;
		while (*pBeg != 0)
		{
			char* pEnd = pBeg;
			while (*pEnd != 0 && *pEnd != '|')
				pEnd++;

			paths.push_back(std::string(pBeg, pEnd - pBeg));

			pBeg = pEnd;
			if (*pBeg == '|')
				pBeg++;
		}

		return paths;
	}
}

//____ selectFolderDialog() ___________________________________________________

std::string MyAppVisitor::selectFolderDialog(const std::string& title, const std::string& defaultPath)
{
	return tinyfd_selectFolderDialog( title.c_str(), defaultPath.c_str());

}

//____ createWindow() _________________________________________________________

Window_p MyAppVisitor::createWindow(const Window::Blueprint& blueprint)
{
	auto pWindow =  SDLWindow::create(blueprint);

	if( g_pDebugFrontend )
		pWindow->_setDebugger(g_pDebugFrontend);

	g_windows.push_back(pWindow.rawPtr());

	return pWindow;
}

//____ openLibrary() __________________________________________________________

WonderApp::LibId MyAppVisitor::openLibrary(const std::string& path)
{
#ifdef WIN32
	return (void*)LoadLibraryA(path.c_str());
#else														// Apple and Linux
	std::string fullPath = "lib" + path + ".dylib";
	return dlopen(fullPath.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
}

//____ loadSymbol() __________________________________________________________

void* MyAppVisitor::loadSymbol(WonderApp::LibId lib, const std::string& symbol)
{
#ifdef WIN32
	return GetProcAddress((HMODULE)lib, symbol.c_str());
#else														// Apple and Linux
	return dlsym(lib, symbol.c_str());
#endif
}

//____ closeLibrary() _________________________________________________________

bool MyAppVisitor::closeLibrary(WonderApp::LibId lib)
{
#ifdef WIN32
	return FreeLibrary((HMODULE)lib);
#else														// Apple and Linux
	return (dlclose(lib) == 0);
#endif
}

//____ resourceDirectory() ____________________________________________________

std::string MyAppVisitor::resourceDirectory()
{
//	char* pBasePath = SDL_GetBasePath();
	char* pBasePath = nullptr;

	if( pBasePath == nullptr )
		return "resources/";
	else
	{
		std::string str = pBasePath;
		SDL_free(pBasePath);
		return str;
	}
}

//____ hidePointer() __________________________________________________________

bool MyHostBridge::hidePointer()
{
	if( SDL_ShowCursor(SDL_DISABLE) == SDL_DISABLE )
		return true;
	
	return false;
}

//____ showPointer() __________________________________________________________

bool MyHostBridge::showPointer()
{
	if( SDL_ShowCursor(SDL_ENABLE) == SDL_ENABLE )
		return true;
	
	return false;
}

//____ setPointerStyle() ______________________________________________________

bool MyHostBridge::setPointerStyle( PointerStyle style )
{

	if( g_mousePointers[(int)style] != NULL )
	{
		SDL_SetCursor(g_mousePointers[(int)style]);
		return true;
	}
	else
	{
		SDL_SystemCursor cursor;
		switch( style )
		{
			case PointerStyle::Arrow:
				cursor = SDL_SYSTEM_CURSOR_ARROW;
				break;
				
			case PointerStyle::Hourglass:
				cursor = SDL_SYSTEM_CURSOR_WAIT;
				break;
				
			case PointerStyle::Hand:
			case PointerStyle::OpenHand:
			case PointerStyle::ClosedHand:
				cursor = SDL_SYSTEM_CURSOR_HAND;
				break;
				
			case PointerStyle::Crosshair:
				cursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
				break;
				
			case PointerStyle::Ibeam:
				cursor = SDL_SYSTEM_CURSOR_IBEAM;
				break;

			case PointerStyle::Stop:
				cursor = SDL_SYSTEM_CURSOR_NO;
				break;
				
			case PointerStyle::ResizeAll:
				cursor = SDL_SYSTEM_CURSOR_SIZEALL;
				break;

			case PointerStyle::ResizeNeSw:
				cursor = SDL_SYSTEM_CURSOR_SIZENESW;
				break;

			case PointerStyle::ResizeNwSe:
				cursor = SDL_SYSTEM_CURSOR_SIZENWSE;
				break;

			case PointerStyle::ResizeNS:
				cursor = SDL_SYSTEM_CURSOR_SIZENS;
				break;

			case PointerStyle::ResizeWE:
				cursor = SDL_SYSTEM_CURSOR_SIZEWE;
				break;

			case PointerStyle::ResizeBeamNS:
				cursor = SDL_SYSTEM_CURSOR_SIZENS;
				break;

			case PointerStyle::ResizeBeamWE:
				cursor = SDL_SYSTEM_CURSOR_SIZEWE;
				break;

			default:

				if( g_mousePointers[(int)PointerStyle::Arrow] == NULL )
					g_mousePointers[(int)PointerStyle::Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

				SDL_SetCursor(g_mousePointers[(int)PointerStyle::Arrow]);
				return false;
		}
		
		g_mousePointers[(int)style] = SDL_CreateSystemCursor(cursor);
		SDL_SetCursor(g_mousePointers[(int)style]);
		return true;
		
	}
}

//____ lockHidePointer() ______________________________________________________

bool MyHostBridge::lockHidePointer()
{


	if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
	{
		// Store mouse position and warp mouse to center of window so we can get large movements.

		if (g_pFocusedWindow)
		{
			int w, h;

			SDL_GetMouseState(&g_lockedPointerPos.x, &g_lockedPointerPos.y);
			g_relModeVirtualPos = g_lockedPointerPos;
		}

		return true;
	}

	return false;
}

//____ unlockShowPointer() ____________________________________________________

bool MyHostBridge::unlockShowPointer()
{
	if (SDL_SetRelativeMouseMode(SDL_FALSE) == 0)
	{
		// Restore mouse position

		if (g_pFocusedWindow)
		{
			SDL_WarpMouseInWindow(g_pFocusedWindow, g_lockedPointerPos.x, g_lockedPointerPos.y);
		}

		return true;
	}

	return false;
}


//____ getClipboardText() _____________________________________________________

std::string MyHostBridge::getClipboardText()
{
	char * pText = SDL_GetClipboardText();
	std::string text = pText;
	SDL_free(pText);
	return text;
}

//____ setClipboardText() _____________________________________________________

bool MyHostBridge::setClipboardText(const std::string& text )
{
	int retval = SDL_SetClipboardText( text.c_str() );
	if( retval != 0 )
	{
		//TODO: Error handling!

		return false;
	}
	
	return true;
}

//____ requestFocus() _________________________________________________________

bool MyHostBridge::requestFocus(uintptr_t windowRef)
{
	auto pWindow = reinterpret_cast<SDLWindow*>(windowRef);
	
	if( pWindow )
	{
		auto pWin = pWindow->SDLWindowPtr();
		
		auto flags = SDL_GetWindowFlags(pWin);
		
		if( flags & SDL_WINDOW_HIDDEN )
		{
			SDL_ShowWindow(pWin);
//			printf( "Show window: %d\n", pWin);
		}
		if( flags & SDL_WINDOW_MINIMIZED )
		{
			SDL_RestoreWindow(pWin);
//			printf( "Restored window: %d\n", pWin);
		}
		SDL_RaiseWindow(pWin);
//		printf( "Raised window: %d\n", pWin);
		return true;
	}
	
	return false;
}

//____ yieldFocus() _________________________________________________________

bool MyHostBridge::yieldFocus(uintptr_t windowRef)
{
	return false;		// Window still has focus.
}
