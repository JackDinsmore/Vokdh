#pragma once
#include "view.h"



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
	bool extraCreateDeviceIndependentResources() override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void extraHandleLeftClick(int posx, int posy) override;
	void handleLeftUnclick(int posx, int posy) override;
	void handleDrag(int posx, int posy) override;

private:
	void indexToScreen(int indexX, int indexY, int* screenX, int* screenY) const;
	void screenToIndex(int screenX, int screenY, int* indexX, int* indexY, bool keepParity=false) const;
	void deleteSelection();
	void copySelectBoth();
	void copySelected();
	void paste();
	int getIndexFromLine(int cursorPosY, int screenX) const;

private:
	int cursorPosX = 0;
	int cursorPosY = 0;
	int selectionCursorX;
	int selectionCursorY;
	bool selection = false;
	bool selectBoth = false;
	bool selectedWhileClicking;

	ID2D1SolidColorBrush* selectBrush;
};