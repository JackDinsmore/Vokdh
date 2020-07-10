#include "vokdh.h"

BOOL Vokdh::create(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu) {
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = Vokdh::windowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className();

	RegisterClass(&wc);

	hwnd = CreateWindowEx(
		dwExStyle, className(), lpWindowName, dwStyle, x, y,
		nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
	);

	return (hwnd ? TRUE : FALSE);
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
		discardGraphicsResources();
		SafeRelease(&factory);
		PostQuitMessage(0);
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

void Vokdh::calculateLayout() {
	if (renderTarget != NULL) {
		D2D1_SIZE_F size = renderTarget->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y);
		ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
	}
}

HRESULT Vokdh::createGraphicsResources() {
	HRESULT hr = S_OK;
	if (renderTarget == NULL) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		hr = factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&renderTarget);

		if (SUCCEEDED(hr)) {
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
			hr = renderTarget->CreateSolidColorBrush(color, &brush);

			if (SUCCEEDED(hr)) {
				calculateLayout();
			}
		}
	}
	return hr;
}

void Vokdh::discardGraphicsResources() {
	SafeRelease(&renderTarget);
	SafeRelease(&brush);
}

void Vokdh::paint() {
	HRESULT hr = createGraphicsResources();
	if (SUCCEEDED(hr)) {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);

		renderTarget->BeginDraw();

		renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
		renderTarget->FillEllipse(ellipse, brush);

		view->draw(renderTarget);

		hr = renderTarget->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
			discardGraphicsResources();
		}
		EndPaint(hwnd, &ps);
	}
}

void Vokdh::resize()
{
	if (renderTarget != NULL) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		renderTarget->Resize(size);
		calculateLayout();
		InvalidateRect(hwnd, NULL, FALSE);
	}

	view->resize();
}