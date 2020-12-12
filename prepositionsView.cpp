#include "prepositionsView.h"


void PrepositionsView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER, SUB_MENU_TB_BUFFER, screenWidth - SUB_MENU_LR_BUFFER, screenHeight - SUB_MENU_TB_BUFFER }, backBrush);

	FLOAT ypos = SUB_MENU_TB_BUFFER + TEXT_BUFFER;
	auto p = dictionary->englishTobairMapPrepositions.begin();
	for (int i = 0; i < textIndex; i++) {
		p++;
	}
	for (; p != dictionary->englishTobairMapPrepositions.end() && ypos < screenHeight - SUB_MENU_TB_BUFFER - TEXT_BUFFER - 20; p++) {
		std::wstring english = std::wstring(p->first.begin(), p->first.end());
		std::wstring tobair = std::wstring(p->second.begin(), p->second.end());
		renderTarget->DrawText(english.c_str(), english.size(), textFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos,
			 screenWidth / 2 - TEXT_BUFFER, ypos + 20 }, englishBrush);
		renderTarget->DrawText(tobair.c_str(), tobair.size(), textFormat, { screenWidth / 2 + TEXT_BUFFER, ypos,
			screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 20 }, tobairBrush);
		ypos += 20;
	}
}

bool PrepositionsView::extraCreateDeviceIndependentResources(HINSTANCE hInst) {
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14, L"", &textFormat);
	return true;
}

bool PrepositionsView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	return true;
}

void PrepositionsView::extraDiscardDeviceDependentResources() {
}

void PrepositionsView::handleScroll(int scrollTimes) {
	textIndex -= scrollTimes;
	textIndex = max(textIndex, 0);

	textIndex = min(textIndex, dictionary->englishTobairMapPrepositions.size() - 1);
}