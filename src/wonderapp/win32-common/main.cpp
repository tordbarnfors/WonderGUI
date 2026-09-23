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

#include <wondergui.h>
#include <windows.h>

#include <win32window.h>
#include <win32api.h>
#include <win32gfxbackend.h>

#include <wg_debugger.h>

#include <vector>

using namespace wg;
using namespace wapp;
using namespace std;

WonderApp_p					g_pApp;

std::vector<Win32Window*>	g_win32Windows;
float						g_ticksToMicroseconds;

wchar_t						g_highSurrogate = 0;

POINT						g_mouseLockPos = { -1, -1 };

PointerStyle				g_currentPointerStyle = PointerStyle::Undefined;

int							g_mouseCaptureRefCount = 0;

DebugFrontend_p				g_pDebugFrontend;
DebugBackend_p				g_pDebugBackend;

Window_p					g_pDebugWindow;

std::wstring _stringToWString(const std::string& str);

static void _setMouseButton(HWND hwnd, MouseButton button, bool bPressed);
static void _setPointer();

bool		init_debugger(Win32API* pAPI);
void		exit_debugger();


//____ Win32HostBridge ___________________________________________________________

class Win32HostBridge : public HostBridge
{
public:
	bool		hidePointer() override;
	bool		showPointer() override;

	bool		setPointerStyle(PointerStyle style) override;

	bool		lockHidePointer() override;
	bool		unlockShowPointer() override;

	std::string	getClipboardText() override;
	bool		setClipboardText(const std::string& text) override;

	bool		requestFocus(uintptr_t windowRef) override;
	bool		yieldFocus(uintptr_t windowRef) override;
};

//____ windowProc() ___________________________________________________________

LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	switch (msg)
	{
		case WM_NCCREATE:
		{
			LPCREATESTRUCT param = reinterpret_cast<LPCREATESTRUCT>(lparam);
			Win32Window* pointer = reinterpret_cast<Win32Window*>(param->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pointer));
			break;
		}

		case WM_CREATE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			g_win32Windows.push_back(pointer);
			break;
		}

		case WM_CLOSE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			bool bClose = pointer->userWindow()->onClose();

			if (bClose)
			{
				if (pointer->userWindow() == g_pDebugWindow)
				{
					g_pDebugWindow = nullptr;
					g_pDebugFrontend->deactivate();
				}
				else
					g_pApp->closeWindow(pointer->userWindow());

			}
			return 0;
		}

		case WM_DESTROY:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			delete pointer;

			auto it = std::find(g_win32Windows.begin(), g_win32Windows.end(), pointer);
			if (it != g_win32Windows.end())
				g_win32Windows.erase(it);
			else
				assert(false);				// Should never happen

			break;
		}

		case WM_PAINT:
		{
			// Backend-specific: software blits the offscreen bitmap, DX12 presents
			// the swap chain. Either way, the window itself knows what to do.

			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			pointer->paint();
			return 0;
		}

		case WM_SIZE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			RECT rect;
			GetClientRect(hwnd, &rect);
			UINT width = rect.right - rect.left;
			UINT height = rect.bottom - rect.top;

			pointer->onResize( width, height );
			pointer->render();
			return 0;		// Returning 0 here is according to Win32 docs.
		}

		case WM_DPICHANGED:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			RECT* const prcNewWindow = (RECT*)lparam;
			SetWindowPos(hwnd,
				NULL,
				prcNewWindow->left,
				prcNewWindow->top,
				prcNewWindow->right - prcNewWindow->left,
				prcNewWindow->bottom - prcNewWindow->top,
				SWP_NOZORDER | SWP_NOACTIVATE);

			// prcNewWindow includes borders and title bar, onResize() wants the client area.
			// Called even if SetWindowPos() already caused a WM_SIZE, since the scale has changed.

			RECT rect;
			GetClientRect(hwnd, &rect);
			pointer->onResize(rect.right - rect.left, rect.bottom - rect.top);
			pointer->render();
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			float scaleFactor = 64.f / pointer->rootPanel()->scale();

			Coord	pos;

			if (g_mouseLockPos.x != -1 && g_mouseLockPos.y != -1)
			{
				POINT currentPos;
				GetCursorPos(&currentPos);

				pos = Base::inputHandler()->pointerPos();
				pos.x += (currentPos.x - g_mouseLockPos.x) * scaleFactor;
				pos.y += (currentPos.y - g_mouseLockPos.y) * scaleFactor;

				SetCursorPos(g_mouseLockPos.x, g_mouseLockPos.y);
			}
			else
				pos = { (int16_t)LOWORD(lparam)*scaleFactor, (int16_t)HIWORD(lparam)*scaleFactor };

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setPointer(pointer->rootPanel(), pos, timestamp);
			break;
		}

		case WM_MOUSELEAVE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setPointer(pointer->rootPanel(), { -1, -1 }, timestamp);
			break;
		}

		case WM_LBUTTONDOWN:
		{
			_setMouseButton(hwnd, MouseButton::Left, true);
			break;
		}

		case WM_LBUTTONUP:
		{
			_setMouseButton(hwnd, MouseButton::Left, false);
			break;
		}

		case WM_RBUTTONDOWN:
		{
			_setMouseButton(hwnd, MouseButton::Right, true);
			break;
		}

		case WM_RBUTTONUP:
		{
			_setMouseButton(hwnd, MouseButton::Right, false);
			break;
		}

		case WM_MBUTTONDOWN:
		{
			_setMouseButton(hwnd, MouseButton::Middle, true);
			break;
		}

		case WM_MBUTTONUP:
		{
			_setMouseButton(hwnd, MouseButton::Middle, false);
			break;
		}

		case WM_CAPTURECHANGED:
		{
			//Set mouse button up for all pressed buttons.

			auto p = Base::inputHandler();
			for (int i = (int)MouseButton_min; i < (int)MouseButton_max; i++)
			{
				if (p->isButtonPressed(MouseButton(i)))
					p->setButton(MouseButton(i), false);
			}

			g_mouseCaptureRefCount = 0;
			break;
		}

		case WM_SETFOCUS:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			Base::inputHandler()->setFocusedWindow(pointer->rootPanel());
			return 0;
		}

		case WM_KILLFOCUS:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			if (Base::inputHandler()->focusedWindow() == pointer->rootPanel())
				Base::inputHandler()->setFocusedWindow(nullptr);
			return 0;
		}

		case WM_SETCURSOR:
		{
			if (LOWORD(lparam) == HTCLIENT)
			{
				_setPointer();
				return TRUE;
			}
			break;
		}

		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (HIWORD(lparam) & KF_REPEAT)
				break;				// This is a key repeat message. Input handler generates our own key repeats, so ignore these.

			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setKey(static_cast<int>(wparam), true, timestamp);
			break;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setKey(static_cast<int>(wparam), false, timestamp);
			break;
		}

		case WM_CHAR:
		{

			wchar_t wch = (wchar_t)wparam;

			// Ignore control characters - handle them in WM_KEYDOWN instead
			if (wch < 32 && wch != '\t' && wch != '\r' && wch != '\n') {
				return 0;  // or break to let DefWindowProc handle it
			}

//			if (wch == 0x08)
//				break; // Ignore backspace, handled as a key event

			if (wch >= 0xD800 && wch <= 0xDBFF) {
				// High surrogate - save it
				g_highSurrogate = wch;
				return 0;
			}

			wchar_t buffer[3] = { 0 };
			int charCount = 1;

			if (wch >= 0xDC00 && wch <= 0xDFFF && g_highSurrogate != 0) {
				// Low surrogate - combine with saved high surrogate
				buffer[0] = g_highSurrogate;
				buffer[1] = wch;
				charCount = 2;
				g_highSurrogate = 0;
			}
			else {
				buffer[0] = wch;
			}

			char utf8[8];
			int len = WideCharToMultiByte(CP_UTF8, 0, buffer, charCount,
				utf8, sizeof(utf8), NULL, NULL);
			if (len > 0) {
				utf8[len] = '\0';

				Base::inputHandler()->putText(utf8);
			}
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//____ initWin32() ____________________________________________________________

void initWin32()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	WNDCLASS wndClass = {};
	wndClass.lpszClassName = "WappWindowClass";
	wndClass.style = 0;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndClass.lpszMenuName = 0;
	wndClass.hInstance = 0;
	wndClass.lpfnWndProc = windowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;

	RegisterClass(&wndClass);

	LARGE_INTEGER	frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	g_ticksToMicroseconds = 1000000.f / float(frequency.QuadPart);
}

//____ exitWin32() ____________________________________________________________

void exitWin32()
{

}

//____ initInputDevices() ______________________________________________________

void initInputDevices()
{
	// Map keycodes

	auto pIH = Base::inputHandler();

	pIH->mapKey(VK_LEFT, Key::Left);
	pIH->mapKey(VK_RIGHT, Key::Right);
	pIH->mapKey(VK_UP, Key::Up);
	pIH->mapKey(VK_DOWN, Key::Down);

	pIH->mapKey(VK_HOME, Key::Home);
	pIH->mapKey(VK_END, Key::End);
	pIH->mapKey(VK_PRIOR, Key::PageUp);
	pIH->mapKey(VK_NEXT, Key::PageDown);

	pIH->mapKey(VK_RETURN, Key::Return);
	pIH->mapKey(VK_BACK, Key::Backspace);
	pIH->mapKey(VK_DELETE, Key::Delete);
	pIH->mapKey(VK_TAB, Key::Tab);
	pIH->mapKey(VK_ESCAPE, Key::Escape);
	pIH->mapKey(VK_SPACE, Key::Space);

	pIH->mapKey(VK_F1, Key::F1);
	pIH->mapKey(VK_F2, Key::F2);
	pIH->mapKey(VK_F3, Key::F3);
	pIH->mapKey(VK_F4, Key::F4);
	pIH->mapKey(VK_F5, Key::F5);
	pIH->mapKey(VK_F6, Key::F6);
	pIH->mapKey(VK_F7, Key::F7);
	pIH->mapKey(VK_F8, Key::F8);
	pIH->mapKey(VK_F9, Key::F9);
	pIH->mapKey(VK_F10, Key::F10);
	pIH->mapKey(VK_F11, Key::F11);
	pIH->mapKey(VK_F12, Key::F12);

	pIH->mapKey(VK_SHIFT, Key::Shift);
	pIH->mapKey(VK_CONTROL, Key::StdControl);

	//TODO: Handle Alt and other modifier keys!!!!!!!


	// Map key commands.

	pIH->mapCommand('X', ModKeys::Command, EditCmd::Cut);
	pIH->mapCommand('C', ModKeys::Command, EditCmd::Copy);
	pIH->mapCommand('V', ModKeys::Command, EditCmd::Paste);

	pIH->mapCommand('A', ModKeys::Command, EditCmd::SelectAll);

	pIH->mapCommand('Z', ModKeys::Command, EditCmd::Undo);
	pIH->mapCommand('Z', ModKeys::CommandShift, EditCmd::Redo);

	pIH->mapCommand(VK_ESCAPE, ModKeys::None, EditCmd::Escape);

	// Map key repeat to those of Windows

	DWORD repeatSpeed = 0;
	SystemParametersInfo(
		SPI_GETKEYBOARDSPEED,  // Get the repeat rate
		0,                      // Not used
		&repeatSpeed,           // Receives value 0-31
		0);                     // No flags

	DWORD repeatDelay = 0;
	SystemParametersInfo(
		SPI_GETKEYBOARDDELAY,  // Get the delay before repeat starts
		0,                      // Not used
		&repeatDelay,           // Receives value 0-3
		0);                     // No flags


	pIH->setKeyRepeat(250000 + repeatDelay * 250000, (int)(1000000.0 / (2.5 + (repeatSpeed * 27.5 / 31.0))));	// Approx conversion from characters per second to milliseconds
}

//____ main() _________________________________________________________________

int main(int arch, char * argv[] ) {

	initWin32();

	// Setup WonderGUI base systems

	Win32HostBridge hostBridge;

	Base::init(&hostBridge);

	initInputDevices();

	// Setup WonderGUI default factories and devices. Backend-specific (soft/DX12/...),
	// see win32gfxbackend.h.

	initGfxBackend();

	// Create app and API visitor, make any app-specific initialization

	g_pApp = WonderApp::create();
	auto pAPI = new Win32API();

	// Create and initialize debugger

	init_debugger(pAPI);

	// Initialize the app

	bool bContinue = g_pApp->init(pAPI);


//	auto pWin = Win32Window::create({ .size = {800,600}, .title = "WonderGUI TEST" } );

	LARGE_INTEGER timeNow;

	while (bContinue)
	{
		MSG msg;

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		bContinue = g_pApp->update();

		// Dispatch messages

		Base::msgRouter()->dispatch();

		// Periodic update

		QueryPerformanceCounter(&timeNow);
		int64_t timestamp = int64_t(timeNow.QuadPart * g_ticksToMicroseconds);

		Base::update(timestamp);

		// Render all windows

		for (auto pWin : g_win32Windows)
			pWin->render();

		// Sleep a bit to avoid hogging the CPU

		Sleep(1);
	}

	exit_debugger();

	g_pApp->exit();
	g_pApp = nullptr;

	Base::exit();

	exitGfxBackend();
	exitWin32();
	return 0;
}




//____ hidePointer() __________________________________________________________

bool Win32HostBridge::hidePointer()
{
	return ShowCursor(FALSE) < 0;
}

//____ showPointer() __________________________________________________________

bool Win32HostBridge::showPointer()
{
	return ShowCursor(TRUE) >= 0;
}

//____ setPointerStyle() _______________________________________________________

bool Win32HostBridge::setPointerStyle(PointerStyle style)
{
	g_currentPointerStyle = style;
	_setPointer();
	return true;
}

//____ lockHidePointer() _______________________________________________________

bool Win32HostBridge::lockHidePointer()
{
	GetCursorPos(&g_mouseLockPos);
	ShowCursor(FALSE);
	return true;
}

//____ unlockShowPointer() ______________________________________________________

bool Win32HostBridge::unlockShowPointer()
{
	if (g_mouseLockPos.x != -1 && g_mouseLockPos.y != -1) {
		SetCursorPos(g_mouseLockPos.x, g_mouseLockPos.y);
		g_mouseLockPos.x = -1;
		g_mouseLockPos.y = -1;
	}
	return true;
}

//____ getClipboardText() ______________________________________________________

std::string	Win32HostBridge::getClipboardText()
{
	std::string clipboardText;

	if (OpenClipboard(NULL)) {
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (hData != NULL) {
			const wchar_t* pWide = static_cast<const wchar_t*>(GlobalLock(hData));
			if (pWide != NULL) {

				int len = WideCharToMultiByte(CP_UTF8, 0, pWide, -1, nullptr, 0, nullptr, nullptr);

				if (len > 0)
				{
					clipboardText.resize(len - 1);
					WideCharToMultiByte(CP_UTF8, 0, pWide, -1, clipboardText.data(), len, nullptr, nullptr);
				}

				GlobalUnlock(hData);

				// Remove any carriage return characters

				clipboardText.erase(
					std::remove(clipboardText.begin(), clipboardText.end(), '\r'),
					clipboardText.end());
			}
		}
		CloseClipboard();
	}

	return clipboardText;
}

//____ setClipboardText() ______________________________________________________

bool Win32HostBridge::setClipboardText(const std::string& text)
{
	bool success = false;

	if (OpenClipboard(NULL)) {
		EmptyClipboard();

		int len = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(),
			nullptr, 0);

		if (len > 0)
		{
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len+1) * sizeof(wchar_t) );
			if (hMem) {
				wchar_t* pWide = static_cast<wchar_t*>(GlobalLock(hMem));
				if (pWide)
				{
					MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), pWide, len);
					pWide[len] = L'\0';
					GlobalUnlock(hMem);
					SetClipboardData(CF_UNICODETEXT, hMem);
					success = true;
				}
				else
					GlobalFree(hMem);
			}
		}
		CloseClipboard();
	}

	return success;
}

//____ requestFocus() __________________________________________________________

bool Win32HostBridge::requestFocus(uintptr_t windowRef)
{
	//TODO: Implement!!!
	return false;
}

//____ yieldFocus() ____________________________________________________________

bool Win32HostBridge::yieldFocus(uintptr_t windowRef)
{
	//TODO: Implement!!!
	return false;
}

//____ _setMouseButton() _______________________________________________________

static void _setMouseButton(HWND hwdn, MouseButton button, bool bPressed)
{
	if (bPressed)
	{
		if (g_mouseCaptureRefCount == 0)
			SetCapture( hwdn );
		g_mouseCaptureRefCount++;
	}
	else
	{
		g_mouseCaptureRefCount--;
		if(g_mouseCaptureRefCount == 0)
			ReleaseCapture();
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
	Base::inputHandler()->setButton(MouseButton::Left, bPressed, timestamp);
}

//____ _setPointer() ___________________________________________________________

static void _setPointer()
{
	switch (g_currentPointerStyle)
	{
		case PointerStyle::Arrow:
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		case PointerStyle::Ibeam:
			SetCursor(LoadCursor(NULL, IDC_IBEAM));
			break;
		case PointerStyle::Hourglass:
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			break;
		case PointerStyle::Crosshair:
			SetCursor(LoadCursor(NULL, IDC_CROSS));
			break;
		case PointerStyle::UpArrow:
			SetCursor(LoadCursor(NULL, IDC_UPARROW));
			break;
		case PointerStyle::ResizeNwSe:
			SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
			break;
		case PointerStyle::ResizeNeSw:
			SetCursor(LoadCursor(NULL, IDC_SIZENESW));
			break;
		case PointerStyle::ResizeBeamWE:
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			break;
		case PointerStyle::ResizeBeamNS:
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			break;
		case PointerStyle::ResizeAll:
			SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			break;
		case PointerStyle::Stop:
			SetCursor(LoadCursor(NULL, IDC_NO));
			break;
		case PointerStyle::OpenHand:
		case PointerStyle::ClosedHand:
		case PointerStyle::Hand:
			SetCursor(LoadCursor(NULL, IDC_HAND));
			break;
		case PointerStyle::Help:
			SetCursor(LoadCursor(NULL, IDC_HELP));
			break;
		default:
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
	}
}

//____ _stringToWString() _____________________________________________________

std::wstring _stringToWString(const std::string& str)
{
	if (str.empty())
		return std::wstring();

	// Get the required buffer size
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,
		nullptr, 0);
	if (sizeNeeded <= 0)
		return std::wstring();

	// Convert
	std::wstring result(sizeNeeded - 1, 0); // -1 to exclude null terminator
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,
		&result[0], sizeNeeded);

	return result;
}

//____ init_debugger() ________________________________________________________

bool init_debugger(Win32API* pAPI)
{
	pAPI->initDefaultWidgetKit();

	auto pIconSurface = pAPI->loadSurface("resources/debugger_gfx.png");
	auto pTransparencyGrid = pAPI->loadSurface("resources/checkboardtile.png", nullptr, { .tiling = true });

	if (!pIconSurface || !pTransparencyGrid)
		return false;

	g_pDebugBackend = DebugBackend::create();

	g_pDebugFrontend = WGCREATE(DebugFrontend, _.backend = g_pDebugBackend, _.icons = pIconSurface, _.transparencyGrid = pTransparencyGrid);

	Base::msgRouter()->addRoute(MsgType::KeyPress, [pAPI](Msg* _pMsg) {

		KeyPressMsg* pMsg = static_cast<KeyPressMsg*>(_pMsg);

		if (pMsg->translatedKeyCode() == Key::F12 && (pMsg->modKeys() == ModKeys::MacCtrlShift || pMsg->modKeys() == ModKeys::StdCtrlShift))
		{
			if (!g_pDebugWindow)
			{
				SizeI size = g_pDebugFrontend->spxSize() / 64;

				auto pWindow = wapp::Window::create(pAPI, { .debug = false, .size = Size(size), .title = "Debugger" });
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

//____ exitDebugger() _________________________________________________________

void exit_debugger()
{
	g_pDebugWindow = nullptr;
	g_pDebugFrontend = nullptr;
	g_pDebugBackend = nullptr;
}
