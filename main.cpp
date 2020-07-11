#pragma comment(lib, "d2d1")

#include <windows.h>
#include <d2d1.h>

#include "vokdh.h"
#include "constants.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int nCmdShow) {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	exePath = std::string(buffer).substr(0, pos);

	Vokdh vokdh = Vokdh(std::string(pCmdLine));

	if (!vokdh.create(L"Circle", WS_OVERLAPPEDWINDOW)) { return 0; }

	ShowWindow(vokdh.window(), nCmdShow);

	// Run the message loop.

	MSG msg = { };
	while(!vokdh.quit) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		vokdh.update();
	}

	return 0;
}