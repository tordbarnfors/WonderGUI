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

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "workbench.h"

#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>
#include <wax_freetypefont.h>

#include <widgetkits/wg_oldskool.h>
#include <wg_debugger.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>

using namespace wg;
using namespace wax;

namespace workbench
{
	//____ SDLHostBridge ______________________________________________________

	class SDLHostBridge : public HostBridge
	{
	public:
		bool		hidePointer() override { return SDL_HideCursor(); }
		bool		showPointer() override { return SDL_ShowCursor(); }

		bool		lockHidePointer() override { return false; }
		bool		unlockShowPointer() override { return false; }

		bool		setPointerStyle(PointerStyle style) override { return false; }

		std::string	getClipboardText() override
		{
			char* pText = SDL_GetClipboardText();
			std::string text = pText ? pText : "";
			SDL_free(pText);
			return text;
		}

		bool		setClipboardText(const std::string& text) override { return SDL_SetClipboardText(text.c_str()); }

		bool		requestFocus(uintptr_t windowRef) override { return true; }
		bool		yieldFocus(uintptr_t windowRef) override { return true; }
	};

	//____ App ________________________________________________________________

	class App
	{
	public:
		bool	init(const char* pStartTest);
		void	run();
		void	exit();

	private:

		bool	_initWindow();
		bool	_initGUI();
		void	_mapKeys();

		Widget_p	_createBrowser();
		void		_fillTestList(const std::string& filter);

		void	_startTest(int index);
		void	_stopTest();
		void	_stepTest(int steps);

		int		_findTest(const std::string& spec) const;
		std::string	_lastTestPath() const;

		void	_processEvents();
		bool	_handleWorkbenchKey(const SDL_KeyboardEvent& e);
		void	_updateCanvas();
		void	_updateWindowRects();

		int		_scale() const;
		Coord	_windowToPoints(float x, float y) const;

		SDL_Window*			m_pWindow = nullptr;
		SDLHostBridge*		m_pHostBridge = nullptr;
		bool				m_bQuit = false;

		RootPanel_p			m_pRoot;
		DebugOverlay_p		m_pDebugOverlay;
		LineEditor_p		m_pFilter;
		SelectCapsule_p		m_pTestList;
		TextDisplay_p		m_pTitle;
		PaddingCapsule_p	m_pTestArea;

		std::vector<int>	m_visibleTests;		// Tests in the list, in order, with the filter applied.
		std::vector<Widget*> m_testEntries;		// Entry in the list for each test, or null if filtered out.

		int					m_currentTest = -1;
		Context*			m_pContext = nullptr;
	};

	//____ init() _____________________________________________________________

	bool App::init(const char* pStartTest)
	{
		if (!_initWindow())
			return false;

		m_pHostBridge = new SDLHostBridge();
		Base::init(m_pHostBridge);

		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);

		Base::setDefaultDevice(GfxDeviceGen2::create(pBackend));
		Base::setDefaultSurfaceFactory(SoftSurfaceFactory::create());
		Base::setDefaultEdgemapFactory(SoftEdgemapFactory::create());
		Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());

		_mapKeys();

		if (!_initGUI())
			return false;

		if (tests().empty())
			return true;

		// Start the test asked for, else the one run last time, else the first.

		int index = -1;

		if (pStartTest)
		{
			index = _findTest(pStartTest);
			if (index < 0)
				printf("Workbench: No test matches '%s'\n", pStartTest);
		}

		if (index < 0)
		{
			std::ifstream file(_lastTestPath());
			std::string spec;
			if (file && std::getline(file, spec))
				index = _findTest(spec);
		}

		_startTest(index >= 0 ? index : 0);
		return true;
	}

	//____ _initWindow() ______________________________________________________

	bool App::_initWindow()
	{
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			printf("Workbench: Unable to initialize SDL: %s\n", SDL_GetError());
			return false;
		}

		m_pWindow = SDL_CreateWindow("Workbench", 1400, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
		if (!m_pWindow)
		{
			printf("Workbench: Unable to create window: %s\n", SDL_GetError());
			return false;
		}

		// The size above is in window coordinates, which aren't points on all
		// systems. Now that the window knows its display we can make it 1400x900 points.

		float scale = _scale() / 64.f;
		float density = SDL_GetWindowPixelDensity(m_pWindow);
		if (density <= 0.f)
			density = 1.f;

		SDL_SetWindowSize(m_pWindow, int(1400 * scale / density), int(900 * scale / density));
		SDL_SetWindowPosition(m_pWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_ShowWindow(m_pWindow);
		SDL_StartTextInput(m_pWindow);
		return true;
	}

	//____ _initGUI() _________________________________________________________

	bool App::_initGUI()
	{
		Context loader;

		auto pFontNormal = FreeTypeFont::create(loader.loadBlob("NotoSans-Regular.ttf"));
		auto pFontBold = FreeTypeFont::create(loader.loadBlob("NotoSans-Bold.ttf"));
		auto pFontItalic = FreeTypeFont::create(loader.loadBlob("NotoSans-Italic.ttf"));
		auto pFontMono = FreeTypeFont::create(loader.loadBlob("DroidSansMono.ttf"));

		if (!oldskool::init(pFontNormal, pFontBold, pFontItalic, pFontMono))
		{
			printf("Workbench: Failed to initialize the oldskool widget kit.\n");
			return false;
		}

		Base::setDefaultStyle(oldskool::TextStyles::Default);

		m_pRoot = RootPanel::create();
		_updateCanvas();

		Base::inputHandler()->setFocusedWindow(m_pRoot);

		// Root -> debugger -> drag'n'drop -> popups -> browser | test

		m_pDebugOverlay = DebugOverlay::create({ .backend = DebugBackend::create() });
		m_pRoot->slot = m_pDebugOverlay;

		auto pDnDOverlay = DragNDropOverlay::create();
		m_pDebugOverlay->mainSlot = pDnDOverlay;

		auto pPopupOverlay = PopupOverlay::create();
		pDnDOverlay->mainSlot = pPopupOverlay;

		auto pSplit = oldskool::SplitPanelX::create(WGBP(oldskool::SplitPanelX, _.resizeRatio = 0.f));
		pPopupOverlay->mainSlot = pSplit;

		m_pTitle = TextDisplay::create(WGBP(TextDisplay,
			_.display.style = oldskool::TextStyles::Heading4,
			_.skin = oldskool::Skins::Plate
		));

		m_pTestArea = PaddingCapsule::create(WGBP(PaddingCapsule, _.skin = ColorSkin::create(Color::White)));

		auto pTestPanel = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y));
		pTestPanel->slots.pushBack(m_pTitle, { .weight = 0.f });
		pTestPanel->slots.pushBack(m_pTestArea, { .weight = 1.f });

		pSplit->slots[0] = _createBrowser();
		pSplit->slots[1] = pTestPanel;
		pSplit->setSplit(0.18f);

		return true;
	}

	//____ _createBrowser() ___________________________________________________

	Widget_p App::_createBrowser()
	{
		m_pFilter = oldskool::LineEditor::create(WGBP(oldskool::LineEditor, _.defaultLengthInChars = 0));

		Base::msgRouter()->addRoute(m_pFilter, MsgType::TextEdit, [this](Msg* pMsg)
		{
			_fillTestList(m_pFilter->editor.text());
		});

		m_pTestList = SelectCapsule::create(WGBP(SelectCapsule, _.recursive = true));

		Base::msgRouter()->addRoute(m_pTestList, MsgType::Selected, [this](Msg* pMsg)
		{
			auto pSelected = static_cast<SelectedMsg*>(pMsg);
			if (pSelected->nbSelected() == 0)
				return;

			int index = pSelected->selected()[0]->id();
			if (index != m_currentTest)
				_startTest(index);
		});

		_fillTestList("");

		auto pScroll = oldskool::ScrollCapsuleY::create();
		pScroll->slot = m_pTestList;

		auto pBrowser = PackPanel::create(WGBP(PackPanel,
			_.axis = Axis::Y,
			_.skin = oldskool::Skins::Plate
		));

		auto pFilterLabel = TextDisplay::create(WGBP(TextDisplay, _.display.text = "Filter:"));

		pBrowser->slots.pushBack(pFilterLabel, { .weight = 0.f });
		pBrowser->slots.pushBack(m_pFilter, { .weight = 0.f });
		pBrowser->slots.pushBack(pScroll, { .weight = 1.f });
		return pBrowser;
	}

	//____ _fillTestList() ____________________________________________________
	//
	// One drawer per group, with the tests whose "group/name" contains the
	// filter, ignoring case.

	void App::_fillTestList(const std::string& filter)
	{
		auto lower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char) std::tolower(c); }); return s; };

		std::string lowerFilter = lower(filter);

		auto& allTests = tests();

		m_visibleTests.clear();
		m_testEntries.assign(allTests.size(), nullptr);

		auto pList = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y));

		PackPanel_p pGroupContent;

		for (int i = 0; i < (int)allTests.size(); i++)
		{
			auto& test = allTests[i];

			if (!lowerFilter.empty() && lower(test.group + "/" + test.name).find(lowerFilter) == std::string::npos)
				continue;

			if (!pGroupContent || allTests[m_visibleTests.back()].group != test.group)
			{
				auto pDrawer = oldskool::TreeListDrawer::create();

				auto pHeader = TextDisplay::create(WGBP(TextDisplay,
					_.display.text = test.group.c_str(),
					_.display.style = oldskool::TextStyles::Strong
				));

				pDrawer->slots[0] = oldskool::TreeListEntry::create(WGBP(oldskool::TreeListEntry,
					_.child = pHeader,
					_.padding = Border(2, 2, 2, 18),
					_.skin = nullptr
				));

				pGroupContent = PackPanel::create(WGBP(PackPanel, _.axis = Axis::Y));
				pDrawer->slots[1] = pGroupContent;
				pDrawer->openImmediately();

				pList->slots.pushBack(pDrawer, { .weight = 0.f });
			}

			auto pEntry = oldskool::TreeListEntry::create(WGBP(oldskool::TreeListEntry,
				_.child = TextDisplay::create(WGBP(TextDisplay, _.display.text = test.name.c_str())),
				_.id = i,
				_.padding = Border(2, 2, 2, 30),
				_.selectable = true
			));

			pGroupContent->slots.pushBack(pEntry, { .weight = 0.f });

			m_visibleTests.push_back(i);
			m_testEntries[i] = pEntry;
		}

		m_pTestList->slot = pList;

		if (m_currentTest >= 0 && m_testEntries[m_currentTest])
			m_pTestList->select(m_testEntries[m_currentTest]);
	}

	//____ _startTest() _______________________________________________________

	void App::_startTest(int index)
	{
		_stopTest();

		auto& test = tests()[index];
		printf("Workbench: Running %s/%s\n", test.group.c_str(), test.name.c_str());

		m_currentTest = index;
		m_pContext = new Context();

		Widget_p pWidget = test.func(*m_pContext);
		m_pTestArea->slot = pWidget;

		std::string title = test.group + " / " + test.name;
		m_pTitle->display.setText(title.c_str());
		SDL_SetWindowTitle(m_pWindow, ("Workbench - " + title).c_str());

		if (m_testEntries[index])
			m_pTestList->select(m_testEntries[index]);

		std::ofstream file(_lastTestPath());
		file << test.group << "/" << test.name << std::endl;
	}

	//____ _stopTest() ________________________________________________________

	void App::_stopTest()
	{
		if (!m_pContext)
			return;

		delete m_pContext;			// Runs the test's exit callbacks and removes its routes.
		m_pContext = nullptr;

		m_pTestArea->slot = nullptr;
		m_currentTest = -1;
	}

	//____ _stepTest() ________________________________________________________
	//
	// Moves to the previous or next test in the list as it is filtered.

	void App::_stepTest(int steps)
	{
		if (m_visibleTests.empty())
			return;

		auto it = std::find(m_visibleTests.begin(), m_visibleTests.end(), m_currentTest);
		int pos = it == m_visibleTests.end() ? 0 : int(it - m_visibleTests.begin()) + steps;

		pos = std::clamp(pos, 0, int(m_visibleTests.size()) - 1);
		if (m_visibleTests[pos] != m_currentTest)
			_startTest(m_visibleTests[pos]);
	}

	//____ _findTest() ________________________________________________________
	//
	// An exact "group/name" first, then the first test whose "group/name"
	// contains spec, ignoring case.

	int App::_findTest(const std::string& spec) const
	{
		auto lower = [](std::string s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char) std::tolower(c); }); return s; };

		auto& allTests = tests();

		for (int i = 0; i < (int)allTests.size(); i++)
			if (allTests[i].group + "/" + allTests[i].name == spec)
				return i;

		std::string lowerSpec = lower(spec);

		for (int i = 0; i < (int)allTests.size(); i++)
			if (lower(allTests[i].group + "/" + allTests[i].name).find(lowerSpec) != std::string::npos)
				return i;

		return -1;
	}

	//____ _lastTestPath() ____________________________________________________

	std::string App::_lastTestPath() const
	{
		std::string path;

		char* pPrefPath = SDL_GetPrefPath("WonderGUI", "Workbench");
		if (pPrefPath)
		{
			path = pPrefPath;
			SDL_free(pPrefPath);
		}

		return path + "lasttest.txt";
	}

	//____ run() ______________________________________________________________

	void App::run()
	{
		while (!m_bQuit)
		{
			_processEvents();

			int64_t microsec = int64_t(SDL_GetTicksNS() / 1000);

			if (m_pContext)
				m_pContext->_update(microsec);

			Base::update(microsec);
			m_pRoot->render();
			_updateWindowRects();

			SDL_Delay(10);
		}
	}

	//____ exit() _____________________________________________________________

	void App::exit()
	{
		if (Base::isInitialized())
		{
			_stopTest();

			m_pFilter = nullptr;
			m_pTestList = nullptr;
			m_pTitle = nullptr;
			m_pTestArea = nullptr;
			m_pDebugOverlay = nullptr;
			m_pRoot = nullptr;

			oldskool::exit();

			Base::exit();
		}

		delete m_pHostBridge;
		m_pHostBridge = nullptr;

		if (m_pWindow)
			SDL_DestroyWindow(m_pWindow);

		SDL_Quit();
	}

	//____ _processEvents() ___________________________________________________

	void App::_processEvents()
	{
		auto pInput = Base::inputHandler();

		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_EVENT_QUIT:
					m_bQuit = true;
					break;

				case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
					_updateCanvas();
					break;

				case SDL_EVENT_MOUSE_MOTION:
					pInput->setPointer(m_pRoot, _windowToPoints(e.motion.x, e.motion.y));
					break;

				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					MouseButton button;
					switch (e.button.button)
					{
						case SDL_BUTTON_MIDDLE:	button = MouseButton::Middle; break;
						case SDL_BUTTON_RIGHT:	button = MouseButton::Right; break;
						case SDL_BUTTON_X1:		button = MouseButton::X1; break;
						case SDL_BUTTON_X2:		button = MouseButton::X2; break;
						default:				button = MouseButton::Left; break;
					}
					pInput->setButton(button, e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
					break;
				}

				case SDL_EVENT_MOUSE_WHEEL:
				{
					bool bInvertScroll = (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED);
					Coord distance(e.wheel.x, e.wheel.y);
					if (bInvertScroll)
						distance *= -1;

					if (e.wheel.y != 0)
						pInput->setWheelRoll(1, distance.y, bInvertScroll);
					if (e.wheel.x != 0)
						pInput->setWheelRoll(2, distance.x, bInvertScroll);
					break;
				}

				case SDL_EVENT_KEY_DOWN:
					if (!_handleWorkbenchKey(e.key))
						pInput->setKey(e.key.key, true);
					break;

				case SDL_EVENT_KEY_UP:
					pInput->setKey(e.key.key, false);
					break;

				case SDL_EVENT_TEXT_INPUT:
					pInput->putText(e.text.text);
					break;

				default:
					break;
			}
		}

		Base::msgRouter()->dispatch();
	}

	//____ _handleWorkbenchKey() ______________________________________________
	//
	// Keys the workbench takes care of itself and that tests don't get.

	bool App::_handleWorkbenchKey(const SDL_KeyboardEvent& e)
	{
		bool bCtrl = (e.mod & SDL_KMOD_CTRL) != 0;

		if (e.key == SDLK_F5 && !e.repeat)
		{
			if (m_currentTest >= 0)
				_startTest(m_currentTest);
			return true;
		}

		if (e.key == SDLK_F12 && !e.repeat)
		{
			m_pDebugOverlay->setActivated(!m_pDebugOverlay->isActivated());
			return true;
		}

		if (bCtrl && e.key == SDLK_PAGEUP)
		{
			_stepTest(-1);
			return true;
		}

		if (bCtrl && e.key == SDLK_PAGEDOWN)
		{
			_stepTest(1);
			return true;
		}

		return false;
	}

	//____ _mapKeys() _________________________________________________________

	void App::_mapKeys()
	{
		auto pInput = Base::inputHandler();

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
		pInput->mapKey(SDLK_KP_ENTER, Key::Return);
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

		pInput->mapCommand(SDLK_X, ModKeys::Command, EditCmd::Cut);
		pInput->mapCommand(SDLK_C, ModKeys::Command, EditCmd::Copy);
		pInput->mapCommand(SDLK_V, ModKeys::Command, EditCmd::Paste);
		pInput->mapCommand(SDLK_A, ModKeys::Command, EditCmd::SelectAll);
		pInput->mapCommand(SDLK_Z, ModKeys::Command, EditCmd::Undo);
		pInput->mapCommand(SDLK_Z, ModKeys::CommandShift, EditCmd::Redo);
		pInput->mapCommand(SDLK_ESCAPE, ModKeys::None, EditCmd::Escape);
	}

	//____ _updateCanvas() ____________________________________________________
	//
	// Renders straight into the window surface, which SDL replaces when the
	// window changes size.

	void App::_updateCanvas()
	{
		SDL_Surface* pWinSurf = SDL_GetWindowSurface(m_pWindow);
		if (!pWinSurf || SDL_BYTESPERPIXEL(pWinSurf->format) != 4)
		{
			printf("Workbench: Unsupported window surface: %s\n", SDL_GetError());
			return;
		}

		auto pCanvas = SoftSurface::create(WGBP(Surface,
			_.canvas = true,
			_.format = PixelFormat::XRGB_8,
			_.scale = _scale(),
			_.size = SizeI(pWinSurf->w, pWinSurf->h)
		), Blob::create(pWinSurf->pixels, nullptr), pWinSurf->pitch);

		m_pRoot->setCanvas(pCanvas);
	}

	//____ _updateWindowRects() _______________________________________________

	void App::_updateWindowRects()
	{
		int nRects = m_pRoot->nbUpdatedRects();
		if (nRects == 0)
			return;

		const RectSPX* pUpdatedRects = m_pRoot->firstUpdatedRect();
		std::vector<SDL_Rect> rects(nRects);

		for (int i = 0; i < nRects; i++)
			rects[i] = { pUpdatedRects[i].x / 64, pUpdatedRects[i].y / 64, pUpdatedRects[i].w / 64, pUpdatedRects[i].h / 64 };

		SDL_UpdateWindowSurfaceRects(m_pWindow, rects.data(), nRects);
	}

	//____ _scale() ___________________________________________________________
	//
	// Points to pixels, times 64, from the display the window is on.

	int App::_scale() const
	{
		float displayScale = SDL_GetWindowDisplayScale(m_pWindow);
		if (displayScale <= 0.f)
			displayScale = 1.f;

		return int(displayScale * 64);
	}

	//____ _windowToPoints() __________________________________________________

	Coord App::_windowToPoints(float x, float y) const
	{
		float density = SDL_GetWindowPixelDensity(m_pWindow);
		float scale = _scale() / 64.f;

		return Coord(x * density / scale, y * density / scale);
	}
}

//____ main() _________________________________________________________________

int main(int argc, char** argv)
{
	workbench::App app;

	if (app.init(argc > 1 ? argv[1] : nullptr))
		app.run();

	app.exit();
	return 0;
}
