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
#define OUTLINE_INDENT 30
#define SUB_MENU_LR_BUFFER 200
#define SUB_MENU_TB_BUFFER 120


enum class VIEW_TYPE {
	TRANSLATION,
	HELP,
	DICTIONARY,
	GRAMMAR,
	PREPOSITIONS,
	SHORT_WORDS,
};


class TranslationView;
class HelpView;
class DictionaryView;
class GrammarView; 
class PrepositionsView;
class ShortWordsView;

class View : protected Poster {
public:
	View(TextTree& textTree) : textTree(textTree) { open(); }

public:
	virtual void draw(ID2D1HwndRenderTarget* renderTarget);
	virtual void resize(int width, int height) = 0;

	bool createDeviceIndependentResources();
	bool createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget);
	void discardDeviceDependentResources();
	virtual bool handleControlShiftKeyPress(int key) { return false; }
	virtual bool handleControlKeyPress(int key) { return false; }
	virtual bool handleKeyPress(int key) {return false;}
	virtual void handleDrag(int posx, int posy) {}
	virtual void handleLeftUnclick(int posx, int posy) {}
	void handleLeftClick(int posx, int posy) { extraHandleLeftClick(posx, posy); }
	virtual void handleScroll(int scrollTimes) {}
	void open();

public:
	bool stageResize = true;
	FLOAT screenWidth;
	FLOAT screenHeight;

protected:
	virtual void extraDraw(ID2D1HwndRenderTarget* renderTarget) const = 0;

	virtual void extraInit() {}
	virtual bool extraCreateDeviceIndependentResources() { return true; }
	virtual bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) { return true; }
	virtual void extraDiscardDeviceDependentResources() {}
	virtual void extraHandleLeftClick(int posx, int posy) {}

protected:
	TextTree& textTree;

	ID2D1SolidColorBrush* darkBGBrush;

	IDWriteFactory* writeFactory;

	StyleMap styleMap = styleMap.summon();

	ID2D1SolidColorBrush* foreBrush;
	ID2D1SolidColorBrush* backBrush;
	ID2D1SolidColorBrush* englishBrush;
	ID2D1SolidColorBrush* tobairBrush;
	D2D1::ColorF bgColor = D2D1::ColorF(D2D1::ColorF::Black);
};

class ViewHandler {
	friend class Vokdh;
public:
	static ViewHandler& summon() {
		static ViewHandler handler;
		return handler;
	}
	
	bool handleControlShiftKeyPress(int key) { return view()->handleControlShiftKeyPress(key); }
	bool handleKeyPress(int key);
	bool handleControlKeyPress(int key);
	void createDeviceIndependentResources() { view()->createDeviceIndependentResources(); }
	void createDeviceDependentResources(ID2D1HwndRenderTarget* rt) { view()->createDeviceDependentResources(rt); }
	void discardDeviceDependentResources() { view()->discardDeviceDependentResources(); }
	void handleLeftUnclick(int x, int y) { view()->handleLeftUnclick(x, y); }
	void handleLeftClick(int x, int y) { view()->handleLeftClick(x, y); }
	void handleScroll(int amt) { view()->handleScroll(amt); }
	void handleDrag(int x, int y) { view()->handleDrag(x, y); }
	void draw(ID2D1HwndRenderTarget* rt);
	void stageResize() { view()->stageResize = true; }
	void open() { view()->open(); }

	void switchTo(VIEW_TYPE);

protected:
	ViewHandler(TextTree& textTree);

private:
	ViewHandler() {}
	~ViewHandler();
	ViewHandler(ViewHandler const&) = delete;
	ViewHandler(ViewHandler&&) = delete;
	ViewHandler& operator=(ViewHandler const&) = delete;
	ViewHandler& operator=(ViewHandler&&) = delete;
	View* view();

	bool drawGrayOut = false;

	TranslationView* translationView;
	HelpView* helpView;
	DictionaryView* dictionaryView;
	GrammarView* grammarView;
	PrepositionsView* prepositionsView;
	ShortWordsView* shortWordsView;
	VIEW_TYPE viewType;
};
