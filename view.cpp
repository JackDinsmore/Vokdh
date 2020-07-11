#include "view.h"

void View::drawOutline(ID2D1HwndRenderTarget* renderTarget) const {

}

bool View::createDeviceIndependentResources() {
	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
	if (FAILED(hr)) {
		postMessage(MESSAGE_TYPE::M_TERMINATE, "Direct write factory creation failed.");
		return false;
	}

	return extraCreateDeviceIndependentResources();
}

bool View::createDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	return extraCreateDeviceDependentResources(renderTarget);
}

void View::discardDeviceDependentResources() {
	extraDiscardDeviceDependentResources();
}



bool TranslationView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);
	return true;
}
void TranslationView::extraDiscardDeviceDependentResources() {
	SafeRelease(&blackBrush);
}

bool TranslationView::extraCreateDeviceIndependentResources() {
	StyleMap styleMap = styleMap.summon();

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

	HRESULT hr = writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
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

	return true;
}

int TranslationView::getLineHeight(std::string tag) const {
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

IDWriteTextFormat* TranslationView::getTextFormat(std::string tag) const {
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

void TranslationView::draw(ID2D1HwndRenderTarget* renderTarget) const {
	drawOutline(renderTarget);

	int width = renderTarget->GetSize().width;
	int height = renderTarget->GetSize().height;

	// Draw English
	TextCounter line = textTree[int(scrollAmount)];

	int ypos = -int((scrollAmount - int(scrollAmount)) * getLineHeight(line.type()));
	while (ypos < height) {
		// English
		std::string text = line.text();
		std::wstring wtext = std::wstring(text.begin(), text.end());
		int lineHeight = getLineHeight(line.type());
		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos, ypos, width, ypos + lineHeight), blackBrush);
		ypos += lineHeight;

		// Tobair
		if (line != textTree.last()) {
			line++;
			std::string text = line.text();
			std::wstring wtext = std::wstring(text.begin(), text.end());
			int lineHeight = getLineHeight(line.type());
			renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos, ypos, width, ypos + lineHeight), blackBrush);
			ypos += lineHeight;
		}
		else {
			break;
		}

		// Space
		if (line != textTree.last()) {
			ypos += lineHeight;
			line++;
		}
		else {
			break;
		}
	}

	// Draw translation
}