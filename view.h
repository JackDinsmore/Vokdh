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

#define MAX_HEIGHT 2000
#define TEXT_BUFFER 5


class View : protected Poster {
public:
	View(TextTree& textTree) : textTree(textTree) { open(); }

public:
	virtual void draw(ID2D1HwndRenderTarget* renderTarget);
	virtual void resize(int width, int height) = 0;

	bool createDeviceIndependentResources();
	bool createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget);
	void discardDeviceDependentResources();
	virtual void handleControlShiftKeyPress(int key) {}
	virtual void handleControlKeyPress(int key) {}
	virtual void handleKeyPress(int key) {}
	void handleLeftClick(int posx, int posy) { extraHandleLeftClick(posx, posy); }
	void handleScroll(int scrollTimes);
	void open();

public:
	bool stageResize = true;

protected:
	virtual void extraDraw(ID2D1HwndRenderTarget* renderTarget) const = 0;
	void drawOutline(ID2D1HwndRenderTarget* renderTarget) const;

	virtual void extraInit() {}
	virtual bool extraCreateDeviceIndependentResources() { return true; }
	virtual bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) { return true; }
	virtual void extraDiscardDeviceDependentResources() {}
	virtual void extraHandleLeftClick(int posx, int posy) {}

protected:
	float scrollAmount = 0;
	int outlinePos = 200;
	TextTree& textTree;
	int screenWidth;
	int screenHeight;

	ID2D1SolidColorBrush* darkBGBrush;

	IDWriteFactory* writeFactory;

	StyleMap styleMap = styleMap.summon();
};


class TranslationView : public View {
public:
	TranslationView(TextTree& textTree) : View(textTree) {}

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	void handleControlShiftKeyPress(int key) override;
	void handleControlKeyPress(int key) override;
	void handleKeyPress(int key) override;
	void resize(int width, int height) { screenWidth = width, screenHeight = height; }

protected:
	bool extraCreateDeviceIndependentResources() override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void extraHandleLeftClick(int posx, int posy) override;

private:
	int getLineHeight(std::string tag) const;
	IDWriteTextFormat* getTextFormat(std::string tag) const;
	void indexToScreen(int indexX, int indexY, int* screenX, int* screenY) const;
	void screenToIndex(int screenX, int screenY, int* indexX, int* indexY) const;
	int getIndexFromLine(int cursorPosY, int screenX) const;

private:
	int h1Size;
	int h2Size;
	int h3Size;
	int pSize;

	IDWriteTextFormat* h1EnglishTextFormat;
	IDWriteTextFormat* h2EnglishTextFormat;
	IDWriteTextFormat* h3EnglishTextFormat;
	IDWriteTextFormat* pEnglishTextFormat;

	ID2D1SolidColorBrush* foreBrush;
	ID2D1SolidColorBrush* englishBrush;
	ID2D1SolidColorBrush* tobairBrush;
	D2D1::ColorF bgColor = D2D1::ColorF(D2D1::ColorF::Black);

	int cursorPosX = 0;
	int cursorPosY = 0;
};