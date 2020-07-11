#pragma once
#include <windows.h>
#include <d2d1.h>

#include "d2tools.h"
#include "view.h"

class Vokdh {
public:
	Vokdh() : factory(NULL), renderTarget(NULL), brush(NULL), hwnd(NULL) {}

public:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	BOOL create(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT, int nHeight = CW_USEDEFAULT, HWND hWndParent = 0, HMENU hMenu = 0);


	HWND window() const { return hwnd; }

	PCWSTR className() const { return L"Vokdh class"; }
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void calculateLayout();
	HRESULT createGraphicsResources();
	void discardGraphicsResources();
	void paint();
	void resize();

private:
	ID2D1Factory* factory;
	ID2D1HwndRenderTarget* renderTarget;
	ID2D1SolidColorBrush* brush;
	D2D1_ELLIPSE ellipse;

	HWND hwnd;

	TranslationView translationView;
	View* view = &translationView;
};
