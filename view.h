#pragma once
#include <d2d1.h>

class View {
public:
	View() {}

public:
	virtual void draw(ID2D1HwndRenderTarget* renderTarget) = 0;
	virtual void resize() = 0;

private:

};


class TranslationView : public View {
public:
	TranslationView() : View() {}

public:
	void draw(ID2D1HwndRenderTarget* renderTarget) override {}
	void resize() override {}
};