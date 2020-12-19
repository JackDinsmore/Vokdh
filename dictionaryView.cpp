#include "dictionaryView.h"
#include <sstream>
#include <stdlib.h>
#include "constants.h"

#define NUM_RESULTS 20

const std::array<int, NUM_CONSONANTS> frequencies = { 5, 4, 10, 2, 9, 13, 17, 7, 15, 3, 20, 14, 23, 7, 9, 4, 10, 8, 9 };


DictionaryView::DictionaryView(TextTree& textTree) : View(textTree) {
	srand(time(NULL));
}

void DictionaryView::handleReopen() {
	wordIndex = 0;
	allWords = false;
	wordIndex = 0;
	selectionIndex = 0;
	if (enterEnglish) {
		bigWindowIterator = dictionary->englishTobairMap.begin();
	}
	else {
		bigWindowIterator = dictionary->tobairEnglishMap.begin();
	}
}

bool DictionaryView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["selection"], &selectBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)D2D1::ColorF::Black, &blackBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)D2D1::ColorF::Red, &redBrush);
	return true;
}

void DictionaryView::extraDiscardDeviceDependentResources() {
	SafeRelease(&selectBrush);
	SafeRelease(&blackBrush);
	SafeRelease(&redBrush);
}

bool DictionaryView::extraCreateDeviceIndependentResources(HINSTANCE hInst) {
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 16, L"", &searchTextFormat);
	return true;
}

void DictionaryView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	if (allWords) {
		displayBigWindow(renderTarget);
	}
	else {
		displaySmallWindow(renderTarget);
	}
}

void DictionaryView::displaySmallWindow(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER, SUB_MENU_TB_BUFFER, screenWidth - SUB_MENU_LR_BUFFER, screenHeight - SUB_MENU_TB_BUFFER }, backBrush);

	ID2D1SolidColorBrush* textBrush = (enterEnglish ? englishBrush : tobairBrush);
	if (isDefiningNewWord) {
		textBrush = isDefiningTobair ? tobairBrush : englishBrush;
	}
	if (isDefiningNewWord && isDefiningTobair && !isValidTobair) {
		textBrush = redBrush;
	}

	// Search box
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER,
		screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 25 }, blackBrush);
	renderTarget->DrawRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER,
		screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 25 }, tobairBrush);
	
	// Selection
	if (selection) {
		FLOAT left = getScreenPos(min(cursorPos, selectionCursor));
		FLOAT right = getScreenPos(max(cursorPos, selectionCursor));
		renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER + left, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 3,
			SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER + right, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 25 }, selectBrush);
	}

	// Text
	std::wstring wtext = std::wstring(text.begin(), text.end());
	renderTarget->DrawText(wtext.c_str(), wtext.size(), searchTextFormat, { SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER+3,
		screenWidth - SUB_MENU_LR_BUFFER - 2 * TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 25 }, textBrush);

	// Cursor
	int screen = getScreenPos(cursorPos) + SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER;
	renderTarget->DrawLine({ (FLOAT)screen, SUB_MENU_TB_BUFFER + TEXT_BUFFER }, { (FLOAT)screen, (FLOAT)(SUB_MENU_TB_BUFFER + TEXT_BUFFER + 25) }, foreBrush, 2);

	if (!isDefiningNewWord) {
		// Results
		FLOAT ypos = SUB_MENU_TB_BUFFER + TEXT_BUFFER + 30;
		for (int i = wordIndex; i < min(wordIndex + numResultsDisplayed, actualNumResults); i++) {
			if (selectionIndex == i) {
				renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos,
					screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 50 }, blackBrush);
			}
			std::wstring r = std::wstring(results[i].first.begin(), results[i].first.end());
			std::wstring trans = std::wstring(results[i].second.begin(), results[i].second.end());
			renderTarget->DrawText(r.c_str(), r.size(), searchTextFormat, { SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER, ypos + 10,
				screenWidth / 2 + TEXT_BUFFER, ypos + 30 }, (enterEnglish ? englishBrush : tobairBrush));
			renderTarget->DrawText(trans.c_str(), trans.size(), searchTextFormat, { screenWidth / 2 - TEXT_BUFFER, ypos + 10,
				screenWidth - SUB_MENU_LR_BUFFER - 2 * TEXT_BUFFER, ypos + 30 }, (enterEnglish ? tobairBrush : englishBrush));
			ypos += 50;
		}
	}
	else {
		searchTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		std::wstring buttonText = L"Create\nReturn";
		renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20, SUB_MENU_TB_BUFFER + 50,
			SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130, SUB_MENU_TB_BUFFER + 90}, blackBrush);
		renderTarget->DrawRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20, SUB_MENU_TB_BUFFER + 50,
			SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130, SUB_MENU_TB_BUFFER + 90 }, tobairBrush);
		renderTarget->DrawText(buttonText.c_str(), buttonText.size(), searchTextFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20, SUB_MENU_TB_BUFFER + 50,
			SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130, SUB_MENU_TB_BUFFER + 90 }, tobairBrush);

		if (isDefiningTobair) {
			buttonText = L"Random\nCtrl-R";
			renderTarget->FillRectangle({ (screenWidth - 130) / 2, SUB_MENU_TB_BUFFER + 50,
				(screenWidth + 130) / 2, SUB_MENU_TB_BUFFER + 90 }, blackBrush);
			renderTarget->DrawRectangle({ (screenWidth - 130) / 2, SUB_MENU_TB_BUFFER + 50,
				(screenWidth + 130) / 2, SUB_MENU_TB_BUFFER + 90 }, tobairBrush);
			renderTarget->DrawText(buttonText.c_str(), buttonText.size(), searchTextFormat, { (screenWidth - 130) / 2, SUB_MENU_TB_BUFFER + 50,
				(screenWidth + 130) / 2, SUB_MENU_TB_BUFFER + 90 }, tobairBrush);
		}

		buttonText = L"Cancel\nEsc";
		renderTarget->FillRectangle({ screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130), SUB_MENU_TB_BUFFER + 50,
			screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20), SUB_MENU_TB_BUFFER + 90 }, blackBrush);
		renderTarget->DrawRectangle({ screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130), SUB_MENU_TB_BUFFER + 50,
			screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20), SUB_MENU_TB_BUFFER + 90 }, tobairBrush);
		renderTarget->DrawText(buttonText.c_str(), buttonText.size(), searchTextFormat, { screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130), SUB_MENU_TB_BUFFER + 50,
			screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20), SUB_MENU_TB_BUFFER + 90 }, tobairBrush);

		searchTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	}
}\

void DictionaryView::displayBigWindow(ID2D1HwndRenderTarget* renderTarget) const {
	// Show entire dictionary
	renderTarget->FillRectangle({ 0, 0, screenWidth, screenHeight }, backBrush);

	// Results
	FLOAT ypos = 30;
	auto tempIterator = bigWindowIterator;
	for (int i = 0; i < bigWindowNumWords; i++) {
		if (selectionIndex == i + wordIndex) {
			renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos,
				screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 50 }, blackBrush);
		}

		std::wstring text, trans;
		text = std::wstring(tempIterator->first.begin(), tempIterator->first.end());
		trans = std::wstring(tempIterator->second.begin(), tempIterator->second.end());
		renderTarget->DrawText(text.c_str(), text.size(), searchTextFormat, { SUB_MENU_LR_BUFFER + 2 * TEXT_BUFFER, ypos + 10,
			screenWidth / 2 + TEXT_BUFFER, ypos + 30 }, (enterEnglish ? englishBrush : tobairBrush));
		renderTarget->DrawText(trans.c_str(), trans.size(), searchTextFormat, { screenWidth / 2 - TEXT_BUFFER, ypos + 10,
			screenWidth - SUB_MENU_LR_BUFFER - 2 * TEXT_BUFFER, ypos + 30 }, (enterEnglish ? tobairBrush : englishBrush));
		ypos += 50;
		tempIterator++;
		if (enterEnglish && tempIterator == dictionary->englishTobairMap.end()) {
			return;
		}
		if (!enterEnglish && tempIterator == dictionary->tobairEnglishMap.end()) {
			return;
		}
	}
}

bool DictionaryView::handleControlShiftKeyPress(int key) {
	switch (key) {
	case 'N':
		isDefiningNewWord = !isDefiningNewWord;
		if (isDefiningNewWord) {
			isDefiningTobair = true;
		}
		return true;
	case 'D':
		allWords = true;
		wordIndex = 0;
		selectionIndex = 0;
		if (enterEnglish) {
			bigWindowIterator = dictionary->englishTobairMap.begin();
		}
		else {
			bigWindowIterator = dictionary->tobairEnglishMap.begin();
		}
		return true;
	}
	return false;
}

bool DictionaryView::handleControlKeyPress(int key) {
	switch (key) {
	case 'A':
		selectionCursor = 0;
		cursorPos = text.size();
		selection = true;
		return true;
	case 'C':
		if (!selection) { return true; }
		copySelected();
		// Copy
		return true;
	case 'X':
		if (!selection) { return true; }
		copySelected();
		deleteSelection();
		// Cut
		return true;
	case 'V':
		if (selection) { deleteSelection(); }
		paste();
		// Paste
		return true;
	case 'R':
		if (isDefiningNewWord) {
			randomText();
			return true;
		}
		break;
	case 'U':
		text = "";
		cursorPos = 0;
		return true;
	}
	return false;
}

bool DictionaryView::handleKeyPress(int key) {
	if (GetKeyState(VK_SHIFT) & 0x8000) {
		if (key == VK_LEFT || key == VK_RIGHT) {
			if (!selection) {
				selection = true;
				selectionCursor = cursorPos;
			}
		}
	}
	else if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) {
		selection = false;
	}
	switch (key) {
	case VK_ESCAPE:
		if (isDefiningNewWord && !allWords) {
			isDefiningNewWord = false;
			return true;
		}
		break;
	case VK_TAB:
		enterEnglish = !enterEnglish;
		wordIndex = 0;
		selectionIndex = 0;
		if (enterEnglish) {
			bigWindowIterator = dictionary->englishTobairMap.begin();
		}
		else {
			bigWindowIterator = dictionary->tobairEnglishMap.begin();
		}
		getResults();
		return true;
	case VK_LEFT:
		if (cursorPos > 0) {
			cursorPos--;
		}
		return true;
	case VK_RIGHT:
		if (cursorPos < text.size()) {
			cursorPos++;
		}
		return true;
	case VK_UP:
		selectionIndex = max(selectionIndex - 1, 0);
		return true;
	case VK_DOWN:
		selectionIndex = min(selectionIndex + 1, actualNumResults - 1);
		return true;
	case VK_PRIOR:// Page up
		wordIndex = 0;
		return true;
	case VK_RETURN:
		if (isDefiningNewWord) {
			submitWord();
		}
		return true;
	case VK_BACK:
		if (!selection) {
			if (cursorPos != 0) {
				if (cursorPos != text.size()) {
					text = text.substr(0, cursorPos - 1) + text.substr(cursorPos);
				}
				else {
					text = text.substr(0, cursorPos - 1);
				}
				cursorPos--;
			}
		}
		else {
			deleteSelection();
		}
		getResults();
		return true;
	case VK_DELETE:
		if (!selection) {
			if (cursorPos != text.size()) {
				text = text.substr(0, cursorPos) + text.substr(cursorPos + 1);
			}
			return true;
		}
		else {
			deleteSelection();
		}
		return true;
	}
	if(allWords){
		if ('A' <= key && key <= 'Z' || key == VK_OEM_7) {// '
			// Scroll to letter
			if (enterEnglish) {
				bigWindowIterator = dictionary->englishTobairMap.begin();
			}
			else {
				bigWindowIterator = dictionary->tobairEnglishMap.begin();
			}
			wordIndex = 0;
			int alphabetKey = std::tolower(key);
			if (key == VK_OEM_7) { alphabetKey = '\''; }
			while (bigWindowIterator->first[0] < alphabetKey) {
				bigWindowIterator++;
				wordIndex++;
				if ((enterEnglish && bigWindowIterator == dictionary->englishTobairMap.end()) ||
					(!enterEnglish && bigWindowIterator == dictionary->tobairEnglishMap.end())) {
					bigWindowIterator--;
					wordIndex--;
					break;//return true;
				}
			}
		}
	}
	else {
		if (key == VK_SPACE || (48 <= key && key <= 57) || (65 <= key && key <= 90) || key == VK_OEM_PERIOD ||
			key == VK_OEM_COMMA || key == VK_OEM_2 || key == VK_OEM_7 || key == VK_OEM_1) {
			if (selection) { deleteSelection(); }
			std::string letter = std::string(1, key);
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				if (key == 49) { letter = "!"; }
				if (key == 50) { letter = "@"; }
				if (key == 51) { letter = "#"; }
				if (key == 52) { letter = "$"; }
				if (key == 53) { letter = "%"; }
				if (key == 54) { letter = "^"; }
				if (key == 55) { letter = "&"; }
				if (key == 56) { letter = "*"; }
				if (key == 57) { letter = "("; }
				if (key == 48) { letter = ")"; }
			}
			else {
				if (65 <= key && key <= 90) {
					letter = std::string(1, key + 32);
				}
			}
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				if (key == VK_OEM_PERIOD) { letter = '>'; }
				if (key == VK_OEM_COMMA) { letter = '<'; }
				if (key == VK_OEM_2) { letter = '?'; }
				if (key == VK_OEM_7) { letter = '"'; }
				if (key == VK_OEM_1) { letter = ':'; }
			}
			else {
				if (key == VK_OEM_PERIOD) { letter = '.'; }
				if (key == VK_OEM_COMMA) { letter = ','; }
				if (key == VK_OEM_2) { letter = '/'; }
				if (key == VK_OEM_7) { letter = '\''; }
				if (key == VK_OEM_1) { letter = ';'; }
			}
			text = text.substr(0, cursorPos) + letter + text.substr(cursorPos);
			cursorPos++;
			getResults();
			return true;
		}
	}
	return false;
}

int DictionaryView::getScreenPos(int index) const {
	std::wstring wtext;
	if (index != 0 && text[index - 1] == ' ') {
		wtext = std::wstring(text.begin(), text.end()).substr(0, index - 1) + L"_";
	}
	else {
		wtext = std::wstring(text.begin(), text.end()).substr(0, index);
	}
	IDWriteTextLayout* layout;
	DWRITE_TEXT_METRICS metrics;
	writeFactory->CreateTextLayout(wtext.c_str(), wtext.size(), searchTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);

	SafeRelease(&layout);

	return metrics.width;
}

int DictionaryView::getIndex(int screen) const {
	std::wstring wtext = std::wstring(text.begin(), text.end());
	IDWriteTextLayout* layout;
	DWRITE_TEXT_METRICS metrics;

	// Check if cursor is even over line
	writeFactory->CreateTextLayout(wtext.c_str(), wtext.size(), searchTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	if (metrics.width < screen) {
		return wtext.size();
	}
	SafeRelease(&layout);

	// Binary search.
	int left = 0;
	int right = wtext.size();
	while (right - left > 1) {
		int middle = (right + left) / 2;
		writeFactory->CreateTextLayout(wtext.c_str(), middle, searchTextFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		if (metrics.width < screen) {
			left = middle;
		}
		else {
			right = middle;
		}
		SafeRelease(&layout);
	}

	// Make final choice
	if (right - left == 1) {
		writeFactory->CreateTextLayout(wtext.c_str(), right, searchTextFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		int width1 = metrics.width;
		SafeRelease(&layout);

		writeFactory->CreateTextLayout(wtext.c_str(), left, searchTextFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		int width2 = metrics.width;
		SafeRelease(&layout);

		if (abs(width1 - screen) < abs(width2 - screen)) {
			return right;
		}
		else {
			return left;
		}
	}
	return left;
}

void DictionaryView::extraHandleLeftClick(int posx, int posy) {
	if (allWords) {
		selectionIndex = max(0, wordIndex + (posy - 30) / 50);
		if (enterEnglish && selectionIndex >= dictionary->englishTobairMap.size()) {
			selectionIndex = dictionary->englishTobairMap.size() - 1;
		}
		if (!enterEnglish && selectionIndex >= dictionary->tobairEnglishMap.size()) {
			selectionIndex = dictionary->tobairEnglishMap.size() - 1;
		}
	}
	else {
		if (SUB_MENU_TB_BUFFER + TEXT_BUFFER * 2 < posy && posy < SUB_MENU_TB_BUFFER + TEXT_BUFFER * 2 + 25) {
			if (GetKeyState(VK_SHIFT) & 0x8000) {
				selectedWhileClicking = true;

				if (posx > TEXT_BUFFER * 2 + SUB_MENU_LR_BUFFER) {
					cursorPos = getIndex(posx - (TEXT_BUFFER * 2 + SUB_MENU_LR_BUFFER));
					selection = true;
				}
			}
			else {
				selectedWhileClicking = false;
				if (posx > TEXT_BUFFER * 2 + SUB_MENU_LR_BUFFER) {
					cursorPos = getIndex(posx - (TEXT_BUFFER * 2 + SUB_MENU_LR_BUFFER));
					selectionCursor = cursorPos;
				}
			}
		}
		else {
			// Results
		}

		if (isDefiningNewWord) {
			// Buttons
			if (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 < posx && posx < SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130 &&
				SUB_MENU_TB_BUFFER + 50 < posy && posy < SUB_MENU_TB_BUFFER + 90) {
				// Create
				submitWord();
			}

			if (isDefiningTobair && (screenWidth - 130) / 2 < posx && posx < (screenWidth + 130) / 2 &&
				SUB_MENU_TB_BUFFER + 50 < posy && posy < SUB_MENU_TB_BUFFER + 90) {
				// Random
				randomText();
			}

			if (screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20 + 130) < posx && posx < screenWidth - (SUB_MENU_LR_BUFFER + TEXT_BUFFER + 20) &&
				SUB_MENU_TB_BUFFER + 50 < posy && posy < SUB_MENU_TB_BUFFER + 90) {
				// Cancel
				isDefiningNewWord = false;
			}
		}
	}
}

void DictionaryView::handleLeftUnclick(int posx, int posy) {
	if (!selectedWhileClicking && !allWords) {
		selection = false;
	}
}

void DictionaryView::handleDrag(int posx, int posy) {
	if (!allWords) {
		if (posx < 0 || posy < 0) { return; }
		if (SUB_MENU_TB_BUFFER + TEXT_BUFFER * 2 < posy && posy < SUB_MENU_TB_BUFFER + TEXT_BUFFER * 2 + 25) {
			cursorPos = getIndex(posx - (TEXT_BUFFER * 2 + SUB_MENU_LR_BUFFER));
			if (selectionCursor != cursorPos) {
				selection = true;
				selectedWhileClicking = true;
			}
		}
	}
}

void DictionaryView::deleteSelection() {
	selection = false;
	text = text.substr(0, min(cursorPos, selectionCursor)) + text.substr(max(cursorPos, selectionCursor));
	cursorPos = min(selectionCursor, cursorPos);
}

void DictionaryView::copySelected() {
	std::string copyText = text.substr(min(cursorPos, selectionCursor), abs(cursorPos - selectionCursor));

	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, copyText.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), copyText.c_str(), copyText.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

void DictionaryView::paste() {
	std::stringstream ss;

	OpenClipboard(0);
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr) { return; }

	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr) { return; }
	ss << pszText;
	GlobalUnlock(hData);
	CloseClipboard();

	std::string addition;
	std::getline(ss, addition);
	if (cursorPos < text.size()) {
		text = text.substr(0, cursorPos) + addition + text.substr(cursorPos);
	}
	else {
		text += addition;
	}
	cursorPos += addition.size();
	getResults();
}

void DictionaryView::resize(int width, int height) {
	screenWidth = width;
	screenHeight = height;
	numResultsDisplayed = (screenHeight - SUB_MENU_TB_BUFFER * 2 - 35) / 50;
	bigWindowNumWords = (screenHeight - 35) / 50;
	if (results) { delete[] results; }
	results = new WordPair[NUM_RESULTS];
	getResults();
}

void DictionaryView::getResults() {
	if (isDefiningNewWord) {
		if (isDefiningTobair) {
			isValidTobair = dictionary->findEnglish(text) == "";
		}
	}
	else {
		if (enterEnglish) { actualNumResults = dictionary->searchEnglish(text, results, NUM_RESULTS); }
		else { actualNumResults = dictionary->searchTobair(text, results, NUM_RESULTS); }
	}
}

void DictionaryView::randomText() {
	int sumFrequencies = 0;
	for (int i : frequencies) { sumFrequencies += i; }
	bool failed = true;
	while (failed) {
		text = "";
		for (int j = 0; j < 3; j++) {
			int freq = (float(std::rand()) / RAND_MAX)* sumFrequencies;
			int i;
			int sumSoFar = 0;
			for (i = 0; i < frequencies.size() && sumSoFar <= freq; i++) {
				sumSoFar += frequencies[i];
			}
			text += consonants[i-1] + " ";
		}
		text = text.substr(0, text.size() - 1);
		failed = !dictionary->findEnglish(text).empty();
	}
	cursorPos = text.size();
}

void DictionaryView::submitWord() {
	if (isDefiningTobair) {
		if (!isValidTobair) { return; }
		newTobairWord = text;
		isDefiningTobair = false;
	}
	else {
		isDefiningNewWord = false;
		dictionary->addWord({ newTobairWord, text });
		dictionary->save();
	}
	text = "";
	cursorPos = 0;
}

void DictionaryView::handleScroll(int scrollTimes) {
	if (scrollTimes > 0) {
		wordIndex--;
	}
	if (scrollTimes < 0) {
		wordIndex++;
	}
	if (scrollTimes > 0 && wordIndex >= 0) {
		bigWindowIterator--;
	}
	if (scrollTimes < 0){
		int set = dictionary->englishTobairMap.size();
		int ste = dictionary->tobairEnglishMap.size();
		if ((enterEnglish && wordIndex < set) || (!enterEnglish && wordIndex < ste)) {
			bigWindowIterator++;
		}
	}
	if (allWords) {
		int s;
		if (enterEnglish) {
			s = dictionary->englishTobairMap.size();
		}
		else {
			s = dictionary->tobairEnglishMap.size();
		}
		wordIndex = max(min(wordIndex, s - 1), 0);
	}
	else {
		wordIndex = max(min(wordIndex, actualNumResults - 1), 0);
	}
}