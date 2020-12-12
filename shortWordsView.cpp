#include "shortWordsView.h"


void ShortWordsView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER, SUB_MENU_TB_BUFFER, screenWidth - SUB_MENU_LR_BUFFER, screenHeight - SUB_MENU_TB_BUFFER }, backBrush);
	
	FLOAT ypos = SUB_MENU_TB_BUFFER + TEXT_BUFFER;
	int i;
	for (i = textIndex; i < dictionary->shortWords.size() && ypos < screenHeight - SUB_MENU_TB_BUFFER - TEXT_BUFFER - 20; i++) {
		std::wstring tobair = std::wstring(dictionary->shortWords[i].begin(), dictionary->shortWords[i].end());
		std::string senglish = dictionary->findEnglish(dictionary->shortWords[i]);
		std::wstring english = std::wstring(senglish.begin(), senglish.end());
		renderTarget->DrawText(english.c_str(), english.size(), textFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos,
			 screenWidth / 2 - TEXT_BUFFER, ypos + 20 }, englishBrush);
		renderTarget->DrawText(tobair.c_str(), tobair.size(), textFormat, { screenWidth / 2 + TEXT_BUFFER, ypos,
			screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 20 }, tobairBrush);
		ypos += 20;
	}
	
	for (i -= dictionary->shortWords.size(); i < dictionary->pronouns.size() && ypos < screenHeight - SUB_MENU_TB_BUFFER - TEXT_BUFFER - 20; i++) {
		std::wstring tobair = std::wstring(dictionary->pronouns[i].begin(), dictionary->pronouns[i].end());
		std::string senglish = dictionary->findEnglish(dictionary->pronouns[i]);
		std::wstring english = std::wstring(senglish.begin(), senglish.end());
		renderTarget->DrawText(english.c_str(), english.size(), textFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos,
			 screenWidth / 2 - TEXT_BUFFER, ypos + 20 }, englishBrush);
		renderTarget->DrawText(tobair.c_str(), tobair.size(), textFormat, { screenWidth / 2 + TEXT_BUFFER, ypos,
			screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 20 }, tobairBrush);
		ypos += 20;
	}
}

bool ShortWordsView::extraCreateDeviceIndependentResources(HINSTANCE hInst) {
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14, L"", &textFormat);
	return true;
}

bool ShortWordsView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	return true;
}

void ShortWordsView::extraDiscardDeviceDependentResources() {
}

void ShortWordsView::handleScroll(int scrollTimes) {
	textIndex -= scrollTimes;
	textIndex = max(textIndex, 0);

	textIndex = min(textIndex, dictionary->shortWords.size() + dictionary->pronouns.size() - 1);
}