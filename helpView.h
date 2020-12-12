#pragma once
#include "view.h"



class HelpView : public View {
public:
	HelpView(TextTree& textTree) : View(textTree) {}

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	void resize(int width, int height) { screenWidth = width, screenHeight = height; }

protected:
	bool extraCreateDeviceIndependentResources(HINSTANCE hInst) override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;

private:
	IDWriteTextFormat* boldTextFormat;
	IDWriteTextFormat* normalTextFormat;

	ID2D1SolidColorBrush* whiteBrush;
};