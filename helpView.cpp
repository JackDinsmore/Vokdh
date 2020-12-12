#include "helpView.h"


void HelpView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER, SUB_MENU_TB_BUFFER, screenWidth - SUB_MENU_LR_BUFFER, screenHeight - SUB_MENU_TB_BUFFER }, backBrush);
	FLOAT ypos = SUB_MENU_TB_BUFFER + TEXT_BUFFER;

	std::vector<std::pair<std::wstring, std::wstring>> text = {
		{L"Fullscreen", L"F-11"},
		{L"Translation", L"Escape"},
		{L"Help", L"Ctrl-H"},
		{L"Dictionary", L"Ctrl-D"},
		{L"Grammar", L"Ctrl-G"},
		{L"Prepositions", L"Ctrl-P"},
		{L"List of short words", L"Ctrl-W"},
		{L"Full Text view", L"Ctrl-T"},
	};
	for (int i = 0; i < text.size(); i++) {
		renderTarget->DrawText(text[i].first.c_str(), text[i].first.size(), boldTextFormat,
			{ SUB_MENU_LR_BUFFER + TEXT_BUFFER, ypos, screenWidth / 2 - TEXT_BUFFER, ypos + 20 }, whiteBrush);
		renderTarget->DrawText(text[i].second.c_str(), text[i].second.size(), normalTextFormat,
			{ screenWidth / 2 + TEXT_BUFFER, ypos, screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, ypos + 20 }, whiteBrush);
		ypos += 20;
	}
}

bool HelpView::extraCreateDeviceIndependentResources(HINSTANCE hInst) {
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14, L"", &boldTextFormat);
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14, L"", &normalTextFormat);
	return true;
}

bool HelpView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush);
	return true;
}

void HelpView::extraDiscardDeviceDependentResources() {
	SafeRelease(&whiteBrush);

}