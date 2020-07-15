#include "vokdh.h"

#pragma comment(lib, "Shcore")

#include <stack>
#include <shellscalingapi.h>
#include <windowsx.h>

Vokdh::Vokdh(std::string commandLine) : loader(textTree), translationView(textTree) {
	postMessage(MESSAGE_TYPE::M_INFO, "Command line was " + commandLine);
	if (commandLine != "" && loader.loadFile(commandLine)) {

	}
	else {
		openFilePath = "C:/Users/goods/Desktop/Projects/Language/Vokdh/Beowulf.vkd";
		loader.loadFile(openFilePath);
	}
}

BOOL Vokdh::createDeviceIndependentResources(HINSTANCE hInstance) {
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = Vokdh::windowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"Vokdh class";

	RegisterClass(&wc);

	/*hwnd = CreateWindowEx(0, L"Vokdh class", L"Vokdh",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(NULL), this);*/
	hwnd = CreateWindow(L"Vokdh class", L"Vokdh", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, this);

	view->createDeviceIndependentResources();

	return (hwnd ? TRUE : FALSE);
}

void Vokdh::update() {
	Message msg;
	for (int i = 0; i < getQueueSize(); i++) {
		peekMessage(&msg, i);
		switch (msg.type) {
		case MESSAGE_TYPE::M_TERMINATE:
		case MESSAGE_TYPE::M_ERROR:
			std::wstring wmsg = std::wstring(msg.m.begin(), msg.m.end());
			MessageBox(NULL, wmsg.c_str(), L"Error", MB_ICONERROR | MB_OK);
			if (msg.type == MESSAGE_TYPE::M_ERROR) {
				break;
			}

			quit = true;
			PostQuitMessage(0);
			break;
		}
	}

	RECT rect;
	GetClientRect(hwnd, &rect);
	InvalidateRect(hwnd, &rect, FALSE);

	logger.update();
}

LRESULT CALLBACK Vokdh::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Vokdh* pThis = NULL;

	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		pThis = (Vokdh*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

		pThis->hwnd = hwnd;
	}
	else {
		pThis = (Vokdh*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	}
	if (pThis) {
		return pThis->handleMessage(uMsg, wParam, lParam);
	}
	else {
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT Vokdh::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))) {
			return -1;  // Fail CreateWindowEx.
		}
		return 0;

	case WM_KEYDOWN:
		handleKeyPress(wParam);
		return 0;

	case WM_LBUTTONDOWN:
		handleLeftClick(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		view->handleScroll(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
		return 0;

	case WM_DESTROY:
		discardDeviceDependentResources();
		SafeRelease(&factory);
		PostQuitMessage(0);
		quit = true;
		return 0;

	case WM_PAINT:
		paint();
		return 0;

	case WM_SIZE:
		resize();
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT Vokdh::createDeviceDependentResources() {
	HRESULT hr = S_OK;
	if (!renderTarget) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size), &renderTarget);
	}
	view->createDeviceDependentResources(renderTarget);
	return hr;
}

void Vokdh::discardDeviceDependentResources() {
	SafeRelease(&renderTarget);

	view->discardDeviceDependentResources();
}

void Vokdh::paint() {
	HRESULT hr = createDeviceDependentResources();
	if (SUCCEEDED(hr)) {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);

		renderTarget->BeginDraw();

		view->draw(renderTarget);

		hr = renderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
			discardDeviceDependentResources();
		}
		EndPaint(hwnd, &ps);
	}
}

void Vokdh::resize() {
	if (renderTarget != NULL) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		renderTarget->Resize(size);
		InvalidateRect(hwnd, NULL, FALSE);
	}

	view->stageResize = true;
}

void Vokdh::handleLeftClick(int keydown, int posx, int posy) {
	view->handleLeftClick(posx, posy);
}

void Vokdh::handleKeyPress(int key) {
	if (GetKeyState(VK_CONTROL) & 0x8000) {
		if (GetKeyState(VK_SHIFT) & 0x8000) {
			// Control and shift
			switch (key) {
			case 'S':
				saveAs();
				return;
			}
			view->handleControlShiftKeyPress(key);
		}
		switch (key) {
		case 'S':
			if (!loader.thisPath.empty()) {
				loader.save();
			}
			else {
				saveAs();
			}
			return;
		case 'O':
			open();
			return;
		case 'N':
			newFile();
			return;
		case 'D':
			// Dictionary
			return;
		case 'G':
			// Grammar page
			return;
		case 'P':
			// Preposition page
			return;
		case 'W':
			// Short words page
			return;
		case 'T':
			// Notes
			return;
		}
		view->handleControlKeyPress(key);
	}
	switch (key) {
	case VK_F11:
		toggleFullscreen();
		return;
	default:
		view->handleKeyPress(key);
		return;
	}
}

void Vokdh::toggleFullscreen() {
	DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	WINDOWPLACEMENT placement;
	if (dwStyle & WS_OVERLAPPEDWINDOW) {
		MONITORINFO mi = { sizeof(mi) };
		if (GetWindowPlacement(hwnd, &placement) &&
			GetMonitorInfo(MonitorFromWindow(hwnd,
				MONITOR_DEFAULTTOPRIMARY), &mi)) {
			SetWindowLong(hwnd, GWL_STYLE,
				dwStyle & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(hwnd, HWND_TOP,
				mi.rcMonitor.left, mi.rcMonitor.top,
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else {
		SetWindowLong(hwnd, GWL_STYLE,
			dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hwnd, &placement);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	resize();
}

void Vokdh::saveAs() {
	char filename[MAX_PATH];

	OPENFILENAMEA parameters;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&parameters, sizeof(parameters));
	parameters.lStructSize = sizeof(parameters);
	parameters.hwndOwner = hwnd;  // If you have a window to center over, put its HANDLE here
	parameters.lpstrFilter = "*.vkd\0Any File\0*.*\0";
	parameters.lpstrFile = filename;
	parameters.nMaxFile = MAX_PATH;
	parameters.lpstrTitle = "Save as";
	parameters.Flags = OFN_DONTADDTORECENT;

	if (GetOpenFileNameA(&parameters)) {
		loader.saveFile(std::string(filename));
	}
}

void Vokdh::open() {
	/// Check if the current file has been saved.

	char filename[MAX_PATH];

	OPENFILENAMEA parameters;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&parameters, sizeof(parameters));
	parameters.lStructSize = sizeof(parameters);
	parameters.hwndOwner = hwnd;  // If you have a window to center over, put its HANDLE here
	parameters.lpstrFilter = "*.vkd\0Any File\0*.*\0";
	parameters.lpstrFile = filename;
	parameters.nMaxFile = MAX_PATH;
	parameters.lpstrTitle = "Open";
	parameters.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&parameters)) {
		loader.unload();
		loader.loadFile(std::string(filename));
	}
}

void Vokdh::newFile() {
	/// Check if current file has been saved.
	loader.unload();
	loader.newFile();
	view->open();
}