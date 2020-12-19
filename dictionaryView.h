#pragma once
#include "view.h"
#include "dictionary.h"


class DictionaryView : public View {
public:
	DictionaryView(TextTree& textTree);

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	bool handleControlShiftKeyPress(int key) override;
	bool handleControlKeyPress(int key) override;
	bool handleKeyPress(int key) override;
	void resize(int width, int height);

protected:
	bool extraCreateDeviceIndependentResources(HINSTANCE hInst) override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void extraHandleLeftClick(int posx, int posy) override;
	void handleLeftUnclick(int posx, int posy) override;
	void handleDrag(int posx, int posy) override;
	void handleScroll(int scrollTimes) override;
	void handleReopen() override;

private:
	int getScreenPos(int index) const;
	int getIndex(int screen) const;
	void deleteSelection();
	void copySelected();
	void paste();
	void getResults();
	void randomText();
	void submitWord();
	void displaySmallWindow(ID2D1HwndRenderTarget* renderTarget) const;
	void displayBigWindow(ID2D1HwndRenderTarget* renderTarget) const;

private:
	int cursorPos = 0;
	int selectionCursor;
	bool selection = false;
	bool selectedWhileClicking;
	std::string text;
	bool enterEnglish = true;
	int numResultsDisplayed;
	int bigWindowNumWords;
	int actualNumResults;
	WordPair* results = nullptr;
	int selectionIndex = 0;
	int wordIndex = 0;
	std::map<std::string, std::string>::iterator bigWindowIterator;

	ID2D1SolidColorBrush* selectBrush = nullptr;
	ID2D1SolidColorBrush* blackBrush = nullptr;
	ID2D1SolidColorBrush* redBrush = nullptr;
	IDWriteTextFormat* searchTextFormat = nullptr;

	bool isDefiningNewWord = false;
	bool isDefiningTobair;
	bool isValidTobair = false;
	bool allWords = false;
	std::string newTobairWord;

	Dictionary* dictionary = dictionary->summon();
};