#pragma comment(lib, "d2d1")

#include <windows.h>
#include <d2d1.h>

#include "vokdh.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
	Vokdh win;

	if (!win.create(L"Circle", WS_OVERLAPPEDWINDOW)) { return 0; }

	ShowWindow(win.window(), nCmdShow);

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}