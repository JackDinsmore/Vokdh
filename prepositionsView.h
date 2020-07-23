#pragma once
#include "view.h"
#include "dictionary.h"



class PrepositionsView : public View {
public:
	PrepositionsView(TextTree& textTree) : View(textTree) {}

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	void resize(int width, int height) { screenWidth = width, screenHeight = height; }

private:
	bool extraCreateDeviceIndependentResources() override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void handleScroll(int scrollTimes) override;

private:
	IDWriteTextFormat* textFormat;

	ID2D1SolidColorBrush* whiteBrush;

	Dictionary dictionary = dictionary.summon();

	int textIndex = 0;
};