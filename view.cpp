#include "view.h"

#include "helpView.h"
#include "translationView.h"


ViewHandler::ViewHandler(TextTree& textTree) {
	translationView = new TranslationView(textTree);
	helpView = new HelpView(textTree);
	switchTo(VIEW_TYPE::TRANSLATION);
}

ViewHandler::~ViewHandler() {
	delete translationView;
	delete helpView;
}

View* ViewHandler::view() {
	switch (viewType) {
	case VIEW_TYPE::TRANSLATION:
		return translationView;
	case VIEW_TYPE::HELP:
		return helpView;
	}
	return nullptr;
}

bool ViewHandler::handleControlKeyPress(int key) {
	switch (key) {
	case 'D':
		// Dictionary
		return true;
	case 'G':
		// Grammar page
		return true;
	case 'P':
		// Preposition page
		return true;
	case 'W':
		// Short words page
		return true;
	case 'T':
		// Notes
		return true;
	case 'H':
		switchTo(VIEW_TYPE::HELP);
		return true;
	}
	return false;
}

bool ViewHandler::handleKeyPress(int key) {
	if (view()->handleKeyPress(key)) {
		return true;
	}
	if (key == VK_ESCAPE) {
		if (viewType != VIEW_TYPE::TRANSLATION) {
			switchTo(VIEW_TYPE::TRANSLATION);
			return true;
		}
	}
	return false;
}

void ViewHandler::switchTo(VIEW_TYPE vt) {
	int width = view()->screenWidth;
	int height = view()->screenHeight;
	if (viewType == vt && viewType != VIEW_TYPE::TRANSLATION) {
		switchTo(VIEW_TYPE::TRANSLATION);
		return;
	}
	if (viewType == VIEW_TYPE::TRANSLATION) {
		// Draw grayed-out thing.
		drawGrayOut = true;
	}
	viewType = vt;
	view()->resize(width, height);
	createDeviceIndependentResources();
}

void ViewHandler::draw(ID2D1HwndRenderTarget* rt) {
	if (drawGrayOut) {
		ID2D1SolidColorBrush* grayoutBrush;
		rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.5), &grayoutBrush);
		rt->FillRectangle({ 0, 0, (FLOAT)view()->screenWidth, (FLOAT)view()->screenHeight }, grayoutBrush);
		SafeRelease(&grayoutBrush);
		drawGrayOut = false;
	}

	view()->draw(rt);
}



void View::drawOutline(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->DrawLine({ (FLOAT)outlinePos, 0 }, { (FLOAT)outlinePos, (FLOAT)screenHeight }, darkBGBrush, 2);

	int ypos = TEXT_BUFFER;
	for (NodeCounter c = textTree.firstNode(); !c.isLast(); c++) {
		if (c.type() == "h1"){
			std::string text = c.text()[0];
			std::wstring wtext = std::wstring(text.begin(), text.end());
			int lineHeight = getLineHeight("h2");
			renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat("h2"),
				D2D1::RectF(TEXT_BUFFER, ypos, outlinePos, ypos + lineHeight), englishBrush);
			ypos += lineHeight;
		}
		if (c.type() == "h2") {
			std::string text = c.text()[0];
			std::wstring wtext = std::wstring(text.begin(), text.end());
			int lineHeight = getLineHeight("h3");
			renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat("h3"),
				D2D1::RectF(TEXT_BUFFER + OUTLINE_INDENT, ypos, outlinePos, ypos + lineHeight), englishBrush);
			ypos += lineHeight;
		}
		if (c.type() == "h3") {
			std::string text = c.text()[0];
			std::wstring wtext = std::wstring(text.begin(), text.end());
			int lineHeight = getLineHeight("p");
			renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat("p"),
				D2D1::RectF(TEXT_BUFFER + 2 * OUTLINE_INDENT, ypos, outlinePos, ypos + lineHeight), englishBrush);
			ypos += lineHeight;
		}
	}
}

bool View::createDeviceIndependentResources() {
	std::string englishFontName = (std::string)styleMap["fonts"]["english"];
	if (englishFontName.empty()) { englishFontName = "Calibri"; }

	h1Size = (int)styleMap["fonts"]["h1-size"];
	if (h1Size == 0) { h1Size = 32; }
	h2Size = (int)styleMap["fonts"]["h2-size"];
	if (h2Size == 0) { h2Size = 24; }
	h3Size = (int)styleMap["fonts"]["h3-size"];
	if (h3Size == 0) { h3Size = 18; }
	pSize = (int)styleMap["fonts"]["p-size"];
	if (pSize == 0) { pSize = 12; }

	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
	if (FAILED(hr)) {
		postMessage(MESSAGE_TYPE::M_TERMINATE, "Direct write factory creation failed.");
		return false;
	}

	hr = writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, h1Size, L"", &h1EnglishTextFormat);
	if (FAILED(hr)) {
		postMessage(MESSAGE_TYPE::M_TERMINATE, "Font " + englishFontName + " could not be used.");
		return false;
	}
	writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, h2Size, L"", &h2EnglishTextFormat);
	writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, h3Size, L"", &h3EnglishTextFormat);
	writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, pSize, L"", &pEnglishTextFormat);

	return extraCreateDeviceIndependentResources();
}

bool View::createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	styleMap = styleMap.summon();
	bgColor = (D2D1::ColorF)styleMap["colors-translation"]["background"];
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["foreground"], &foreBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["background"], &backBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["english"], &englishBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["tobair"], &tobairBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["dark-background"], &darkBGBrush);
	return extraCreateDeviceDependentResources(renderTarget);
}

void View::discardDeviceDependentResources() {
	SafeRelease(&darkBGBrush);
	SafeRelease(&foreBrush);
	SafeRelease(&englishBrush);
	SafeRelease(&tobairBrush);
	SafeRelease(&backBrush);
	extraDiscardDeviceDependentResources();
}

void View::draw(ID2D1HwndRenderTarget* renderTarget) {
	if (stageResize) {
		resize(renderTarget->GetSize().width, renderTarget->GetSize().height);
	}
	extraDraw(renderTarget);
}

void View::handleScroll(int scrollTimes) {
	scrollAmount -= scrollTimes;
	scrollAmount = max(scrollAmount, 0);

	scrollAmount = min(scrollAmount, textTree.size() - 1);
}

void View::open() {
}

int View::getLineHeight(std::string tag) const {
	if (tag == "h1") {
		return int(h1Size * 1.5f);
	}
	if (tag == "h2") {
		return int(h2Size * 1.5f);
	}
	if (tag == "h3") {
		return int(h3Size * 1.5f);
	}
	if (tag == "p") {
		return int(pSize * 1.5f);
	}
	return 0;
}

IDWriteTextFormat* View::getTextFormat(std::string tag) const {
	if (tag == "h1") {
		return h1EnglishTextFormat;
	}
	if (tag == "h2") {
		return h2EnglishTextFormat;
	}
	if (tag == "h3") {
		return h3EnglishTextFormat;
	}
	if (tag == "p") {
		return pEnglishTextFormat;
	}
	return 0;
}
