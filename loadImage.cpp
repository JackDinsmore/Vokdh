#include "loadImage.h"

HRESULT WICMachine::create(HINSTANCE _hInst) {
	hInst = _hInst;
	foo = 0;

	CoInitialize(NULL);

	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wicFactory));
	return hr;
}

HRESULT WICMachine::LoadResourceBitmap(ID2D1RenderTarget* renderTarget, LPCWSTR resourceName,
	UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** bitmap) {

	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICStream* pStream = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	IWICBitmapScaler* pScaler = nullptr;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = nullptr;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(hInst, resourceName, RT_BITMAP);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr)) {
		imageResDataHandle = LoadResource(hInst, imageResHandle);
		hr = imageResDataHandle ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr)) {
		pImageFile = LockResource(imageResDataHandle);
		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr)) {
		imageFileSize = SizeofResource(hInst, imageResHandle);
		hr = imageFileSize ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr)) {
		hr = wicFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr)) {
		hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
	}

	if (SUCCEEDED(hr)) {
		hr = wicFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder);
	}

	if (SUCCEEDED(hr)) {
		hr = pDecoder->GetFrame(0, &pSource);

		if (SUCCEEDED(hr)) {
			// Convert the image format to 32bppPBGRA
			hr = wicFactory->CreateFormatConverter(&pConverter);
		}

		if (SUCCEEDED(hr)) {
			hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		}

		if (SUCCEEDED(hr)) {
			hr = renderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, bitmap);
		}
	}
	else {
		hr = renderTarget->CreateBitmapFromWicBitmap(pSource, NULL, bitmap);
	}

	if (pDecoder) pDecoder->Release();
	if (pSource) pSource->Release();
	if (pStream) pStream->Release();
	if (pConverter) pConverter->Release();
	if (pScaler) pScaler->Release();
	
	return hr;
}