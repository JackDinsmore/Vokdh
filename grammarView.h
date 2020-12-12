#pragma once
#include "view.h"



class GrammarView : public View {
public:
	GrammarView(TextTree& textTree) : View(textTree) {}

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	void resize(int width, int height) { screenWidth = width, screenHeight = height; }

private:
	bool extraCreateDeviceIndependentResources(HINSTANCE hInst) override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void handleScroll(int scrollTimes) override;

private:
	IDWriteTextFormat* headerFormat;
	IDWriteTextFormat* bodyFormat;

	ID2D1SolidColorBrush* whiteBrush;

	int textIndex = 0;
};