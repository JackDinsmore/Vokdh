#pragma once
#include "view.h"
#include "dictionary.h"


class TranslationView : public View {
public:
	TranslationView(TextTree& textTree) : View(textTree) {}

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	bool handleControlShiftKeyPress(int key) override;
	bool handleControlKeyPress(int key) override;
	bool handleKeyPress(int key) override;
	void resize(int width, int height) { screenWidth = width, screenHeight = height; }

protected:
	bool extraCreateDeviceIndependentResources(HINSTANCE hInst) override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void extraHandleLeftClick(int posx, int posy) override;
	void extraHandleLeftDoubleClick(int posx, int posy) override;
	void handleLeftUnclick(int posx, int posy) override;
	void handleDrag(int posx, int posy) override; 
	void handleScroll(int scrollTimes) override;
	void handleMouseMotion(int x, int y) override;

private:
	void indexToScreen(int indexX, int indexY, int* screenX, int* screenY) const;
	void screenToIndex(int screenX, int screenY, int* indexX, int* indexY, bool keepParity=false, bool accurate=false) const;
	void deleteSelection();
	void copySelectBoth();
	void copySelected();
	void paste();
	int getIndexFromLine(int cursorPosY, int screenX, bool accurate=false) const;
	int getLineHeight(std::string tag) const;
	IDWriteTextFormat* getTextFormat(std::string tag) const;
	void drawOutline(ID2D1HwndRenderTarget* renderTarget) const;
	void drawHover(ID2D1HwndRenderTarget* rt) const;
	bool terminatesWord(char letter) const;
	void breakNode();

private:
	float scrollAmount = 0;
	int outlinePos = 200;
	int cursorPosX = 0;
	int cursorPosY = 0;
	int selectionCursorX;
	int selectionCursorY;
	int hoverIndexX;
	int hoverIndexY;
	int hoverScreenX;
	int hoverScreenY;
	bool selection = false;
	bool selectBoth = false;
	bool selectedWhileClicking;

	ID2D1SolidColorBrush* selectBrush;

	IDWriteTextFormat* h1EnglishTextFormat;
	IDWriteTextFormat* h2EnglishTextFormat;
	IDWriteTextFormat* h3EnglishTextFormat;
	IDWriteTextFormat* pEnglishTextFormat;
	int h1Size;
	int h2Size;
	int h3Size;
	int pSize;

	Dictionary* dictionary = dictionary->summon();
	Grammar grammar;

	bool hoverBoxOpen = false;
};