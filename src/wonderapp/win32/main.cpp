

#include <wondergui.h>
#include <windows.h>

#include <wg_softkernels_default.h>
#include <wg_softsurfacefactory.h>
#include <wg_softbackend.h>

#include <win32window.h>
#include <win32api.h>

#include <vector>

using namespace wg;
using namespace wapp;
using namespace std;

WonderApp_p					g_pApp;
std::vector<Win32Window*>	g_win32Windows;
float						g_ticksToMicroseconds;		

wchar_t						g_highSurrogate = 0;

static void _setMouseButton(MouseButton button, bool bPressed);



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
				g_pApp->closeWindow(pointer->userWindow());
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
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP oldBitmap = (HBITMAP)SelectObject(hdcMem, pointer->canvasBitmap() );
			BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
				ps.rcPaint.right - ps.rcPaint.left,
				ps.rcPaint.bottom - ps.rcPaint.top,
				hdcMem,
				ps.rcPaint.left, ps.rcPaint.top,
				SRCCOPY);
			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_SIZE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			pointer->onResize( LOWORD(lparam), HIWORD(lparam) );
			pointer->render();
			break;
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
			pointer->onResize(prcNewWindow->right - prcNewWindow->left,
				prcNewWindow->bottom - prcNewWindow->top);
			pointer->render();
			break;
		}

		case WM_MOUSEMOVE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			int scale = pointer->rootPanel()->scale();

			Coord pos = { LOWORD(lparam)*64.f/scale, HIWORD(lparam)*64.f/scale };

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
			_setMouseButton(MouseButton::Left, true);
			break;
		}

		case WM_LBUTTONUP:
		{
			_setMouseButton(MouseButton::Left, false);
			break;
		}	

		case WM_RBUTTONDOWN:
		{
			_setMouseButton(MouseButton::Right, true);
			break;
		}

		case WM_RBUTTONUP:
		{
			_setMouseButton(MouseButton::Right, false);
			break;
		}

		case WM_MBUTTONDOWN:
		{
			_setMouseButton(MouseButton::Middle, true);
			break;
		}

		case WM_MBUTTONUP:
		{
			_setMouseButton(MouseButton::Middle, false);
			break;
		}

		case WM_SETFOCUS:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			Base::inputHandler()->setFocusedWindow(pointer->rootPanel());
			break;
		}

		case WM_KILLFOCUS:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			if (Base::inputHandler()->focusedWindow() == pointer->rootPanel())
				Base::inputHandler()->setFocusedWindow(nullptr);
			break;
		}


		case WM_KEYDOWN:
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setKey(static_cast<int>(wparam), true, timestamp);
			return 0;
		}

		case WM_KEYUP:
		{
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
			Base::inputHandler()->setKey(static_cast<int>(wparam), false, timestamp);
			return 0;
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

//____ main() _________________________________________________________________

int main(int arch, char * argv[] ) {

	initWin32();

	// Setup WonderGUI base systems

	Win32HostBridge hostBridge;

	Base::init(&hostBridge);

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

	//TODO: Handle Alt and others!!!!!!!


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


	pIH->setKeyRepeat(250000 + repeatDelay*250000, (int)(1000000.0 / (2.5 + (repeatSpeed * 27.5 / 31.0))) );	// Approx conversion from characters per second to milliseconds


	// Setup WonderGUI default factories and devices

	auto pFactory = wg::SoftSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pFactory);

	auto pBackend = wg::SoftBackend::create();

	addDefaultSoftKernels(pBackend);

	auto pGfxDevice = wg::GfxDeviceGen2::create(pBackend);
	Base::setDefaultGfxDevice(pGfxDevice);


	// Create app and API visitor, make any app-specific initialization

	g_pApp = WonderApp::create();
	auto pAPI = new Win32API();

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

	g_pApp->exit();
	g_pApp = nullptr;

	Base::exit();

	exitWin32();
	return 0;
}





bool Win32HostBridge::hidePointer()
{ 
	return false; 
}


bool Win32HostBridge::showPointer()
{ 
	return false; 
}

bool Win32HostBridge::setPointerStyle(PointerStyle style)
{
	return false;
}

bool Win32HostBridge::lockHidePointer()
{
	return false;
}

bool Win32HostBridge::unlockShowPointer()
{
	return false;
}

std::string	Win32HostBridge::getClipboardText()
{
	return "";
}

bool Win32HostBridge::setClipboardText(const std::string& text)
{
	return false;
}

bool Win32HostBridge::requestFocus(uintptr_t windowRef)
{
	return false;
}

bool Win32HostBridge::yieldFocus(uintptr_t windowRef)
{
	return false;
}

//____ _setMouseButton() _______________________________________________________

static void _setMouseButton(MouseButton button, bool bPressed)
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	int64_t timestamp = int64_t(counter.QuadPart * g_ticksToMicroseconds);
	Base::inputHandler()->setButton(MouseButton::Left, bPressed, timestamp);
}
