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
	bool extraCreateDeviceIndependentResources() override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void extraHandleLeftClick(int posx, int posy) override;
	void handleLeftUnclick(int posx, int posy) override;
	void handleDrag(int posx, int posy) override;

private:
	int getScreenPos(int index) const;
	int getIndex(int screen) const;
	void deleteSelection();
	void copySelected();
	void paste();
	void getResults();
	void randomText();
	void submitWord();

private:
	int cursorPos = 0;
	int selectionCursor;
	bool selection = false;
	bool selectedWhileClicking;
	std::string text;
	bool enterEnglish = true;
	int numResults;
	int actualNumResults;
	WordPair* results = nullptr;
	int selectionIndex = 0;

	ID2D1SolidColorBrush* selectBrush = nullptr;
	ID2D1SolidColorBrush* blackBrush = nullptr;
	ID2D1SolidColorBrush* redBrush = nullptr;
	IDWriteTextFormat* searchTextFormat = nullptr;

	bool isDefiningNewWord = false;
	bool isDefiningTobair;
	bool isValidTobair = false;
	std::string newTobairWord;

	Dictionary dictionary = dictionary.summon();
};