#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <atlbase.h>
#include <memory>

#pragma comment(lib, "WindowsCodecs.lib")

class WICMachine {
public:
	static WICMachine* summon() {
		if (!instance) {
			instance = new WICMachine();
		}
		return instance;
	}

	HRESULT create(HINSTANCE hInstance);

	HRESULT LoadResourceBitmap(ID2D1RenderTarget* renderTarget, LPCWSTR resourceName,
		UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** bitmap);

private:
	WICMachine() {}
	~WICMachine() {};
	IWICImagingFactory* wicFactory = nullptr;
	HINSTANCE hInst;

	int foo = 12;

	inline static WICMachine *instance = nullptr;
};