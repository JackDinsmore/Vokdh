#pragma comment(lib, "d2d1")

#include <windows.h>
#include <d2d1.h>
#include <chrono>

#include "vokdh.h"
#include "constants.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int nCmdShow) {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    exePath = std::string(buffer).substr(0, pos);

    Vokdhi vokdh = Vokdhi(std::string(pCmdLine));

    if (!vokdh.createDeviceIndependentResources(hInstance)) { return 0; }

    ShowWindow(vokdh.window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    auto last = std::chrono::system_clock::now();
    auto now = last;
    while (!vokdh.quit) {
        now = std::chrono::system_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() >= FRAMERATE) {// Lock at 40 fps
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            vokdh.update();
        }
    }

    vokdh.close();

    return 0;
}