

#include <wondergui.h>
#include <windows.h>

#include <wg_softsurfacefactory.h>
#include <wg_softbackend.h>

#include <win32window.h>
#include <win32api.h>

using namespace wg;




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
			break;
		}

		case WM_CLOSE:
		{
			Win32Window* pointer = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			bool bClose = pointer->userWindow()->onClose();

			if (bClose)
				DestroyWindow(hwnd);
			return 0;
		}

		case WM_DESTROY:
		{
			break;
		}

	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//____ initWin32() ____________________________________________________________

void initWin32()
{
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
}

//____ exitWin32() ____________________________________________________________

void exitWin32()
{

}

//____ main() _________________________________________________________________

int main(int arch, char * argv[] ) {

	initWin32();

	Win32HostBridge hostBridge;

	Base::init(&hostBridge);


	auto pFactory = wg::SoftSurfaceFactory::create();

	auto pBackend = wg::SoftBackend::create();
	auto pGfxDevice = wg::GfxDeviceGen2::create(pBackend);

	Base::setDefaultSurfaceFactory(pFactory);


	// Create app and API visitor, make any app-specific initialization

	auto pApp = WonderApp::create();
	auto pAPI = new Win32API();

	// Initialize the app

	bool bContinue = pApp->init(pAPI);


//	auto pWin = Win32Window::create({ .size = {800,600}, .title = "WonderGUI TEST" } );


	while (bContinue)
	{
		MSG msg;

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		bContinue = pApp->update();

		Sleep(1);
	}

	pApp->exit();

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
