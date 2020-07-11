#pragma once
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma comment(lib, "dwrite")

#include <d2d1.h>
#include <vector>
#include <string>
#include <dwrite.h>

#include "text.h"
#include "style.h"
#include "d2tools.h"


class View : protected Poster {
public:
	View(TextTree& textTree) : textTree(textTree) {}

public:
	virtual void draw(ID2D1HwndRenderTarget* renderTarget) const = 0;
	virtual void resize() {}

	bool createDeviceIndependentResources();
	bool createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget);
	void discardDeviceDependentResources();

protected:
	void drawOutline(ID2D1HwndRenderTarget* renderTarget) const;

	virtual void extraInit() {}
	virtual bool extraCreateDeviceIndependentResources() { return true; }
	virtual bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) { return true; }
	virtual void extraDiscardDeviceDependentResources() {}

protected:
	float scrollAmount = 0;
	int outlinePos = 3;
	TextTree& textTree;

	IDWriteFactory* writeFactory;
};


class TranslationView : public View {
public:
	TranslationView(TextTree& textTree) : View(textTree) {}

public:
	void draw(ID2D1HwndRenderTarget* renderTarget) const override;

protected:
	bool extraCreateDeviceIndependentResources() override;
	virtual bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	virtual void extraDiscardDeviceDependentResources() override;

private:
	int getLineHeight(std::string tag) const;
	IDWriteTextFormat* getTextFormat(std::string tag) const;

private:
	int h1Size;
	int h2Size;
	int h3Size;
	int pSize;

	IDWriteTextFormat* h1EnglishTextFormat;
	IDWriteTextFormat* h2EnglishTextFormat;
	IDWriteTextFormat* h3EnglishTextFormat;
	IDWriteTextFormat* pEnglishTextFormat;

	ID2D1SolidColorBrush* blackBrush;
};