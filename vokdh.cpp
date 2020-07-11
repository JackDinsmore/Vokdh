#include "vokdh.h"

#include <stack>

Vokdh::Vokdh(std::string commandLine) : loader(textTree), translationView(textTree) {
	postMessage(MESSAGE_TYPE::M_INFO, "Command line was " + commandLine);
	if (commandLine != "" && loader.loadFile(commandLine)) {

	}
	else {
		openFilePath = "C:/Users/goods/Desktop/Projects/Language/Vokdh/Beowulf.vkd";
		loader.loadFile(openFilePath);
	}
}

BOOL Vokdh::createDeviceIndependentResources(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu) {
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = Vokdh::windowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className();

	RegisterClass(&wc);

	hwnd = CreateWindowEx(
		dwExStyle, className(), lpWindowName, dwStyle, x, y,
		nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
	);

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

		renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

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

	view->resize();
}