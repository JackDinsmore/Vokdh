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

void View::draw(ID2D1HwndRenderTarget* renderTarget) {
	if (stageResize) {
		resize(renderTarget->GetSize().width, renderTarget->GetSize().height);
	}
	extraDraw(renderTarget);
}





bool TranslationView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &blackBrush);
	bgColor = (D2D1::ColorF)styleMap["colors-translation"]["background"];
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["english"], &englishBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["tobair"], &tobairBrush);
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

void TranslationView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->Clear(bgColor);

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
		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos, ypos, width, ypos + lineHeight), englishBrush);
		ypos += lineHeight;

		// Tobair
		if (line != textTree.last()) {
			line++;
			std::string text = line.text();
			std::wstring wtext = std::wstring(text.begin(), text.end());
			int lineHeight = getLineHeight(line.type());
			renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos, ypos, width, ypos + lineHeight), tobairBrush);
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

	// Draw cursor
	int screenX, screenY;
	int lineHeight = getLineHeight(line.type());
	indexToScreen(cursorPosX, cursorPosY, &screenX, &screenY);
	screenX += outlinePos;
	renderTarget->DrawLine({(FLOAT)screenX, (FLOAT)screenY}, { (FLOAT)screenX, (FLOAT)(screenY + lineHeight)}, blackBrush, 3);
}

void TranslationView::handleControlShiftKeyPress(int key) {
	switch (key) {
	case 'A':
		// Select all text
		return;
	}
}

void TranslationView::handleControlKeyPress(int key) {
	switch (key) {
	case 'A':
		// Select all of type
		return;
	}
}

void TranslationView::handleKeyPress(int key) {
	if (GetKeyState(VK_SHIFT) & 0x8000) {
	}
	switch (key) {
	case VK_LEFT:
		if (cursorPosX > 0) {
			cursorPosX--;
		}
		else {
			if (cursorPosY / 2 > 0) {
				cursorPosY -= 2;
				cursorPosX = textTree[cursorPosY].text().size();
			}
		}
		return;
	case VK_RIGHT:
		if (cursorPosX < textTree[cursorPosY].text().size()) {
			cursorPosX++;
		}
		else {
			if (cursorPosY / 2 < textTree.size() / 2 - 1) {
				cursorPosY += 2;
				cursorPosX = 0;
			}
		}
		return;
	case VK_UP:
		if (cursorPosY / 2 > 0) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY -= 2;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return;
	case VK_DOWN:
		if (cursorPosY / 2 < textTree.size() / 2 - 1) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY += 2;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return;
	}
	if (key == VK_SPACE || (48 <= key && key <= 57) || (65 <= key && key <= 90)) {
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
		textTree[cursorPosY].text() = textTree[cursorPosY].text().substr(0, cursorPosX) + letter + textTree[cursorPosY].text().substr(cursorPosX);
		cursorPosX++;
		return;
	}
}

void TranslationView::indexToScreen(int indexX, int indexY, int* screenX, int* screenY) const {
	TextCounter line = textTree[indexY];
	std::string text = line.text();

	if (screenY) {
		TextCounter count = textTree[int(scrollAmount)];
		int ypos = -int((scrollAmount - int(scrollAmount)) * getLineHeight(line.type()));
		while (count != line) {
			// English
			int lineHeight = getLineHeight(count.type());
			ypos += lineHeight;
			count++;

			if (count == line) break;

			// Tobair
			lineHeight = getLineHeight(count.type());
			ypos += lineHeight;
			count++;

			// Space
			ypos += lineHeight;
		}

		*screenY = ypos;
	}


	if (screenX) {
		std::wstring wtext;
		if (indexX != 0 && text[indexX - 1] == ' ') {
			wtext = std::wstring(text.begin(), text.end()).substr(0, indexX - 1) + L"_";
		}
		else {
			wtext = std::wstring(text.begin(), text.end()).substr(0, indexX);
		}
		IDWriteTextLayout* layout;
		DWRITE_TEXT_METRICS metrics;
		writeFactory->CreateTextLayout(wtext.c_str(), wtext.size(), getTextFormat(line.type()), screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);

		*screenX = metrics.width;

		SafeRelease(&layout);
	}

}
void TranslationView::screenToIndex(int screenX, int screenY, int* indexX, int* indexY) const {

}

int TranslationView::getIndexFromLine(int cursorPosY, int screenX) const {
	TextCounter line = textTree[cursorPosY];
	std::wstring wtext = std::wstring(line.text().begin(), line.text().end());
	IDWriteTextFormat* textFormat = getTextFormat(line.type());
	IDWriteTextLayout* layout;
	DWRITE_TEXT_METRICS metrics;

	// Check if cursor is even over line
	writeFactory->CreateTextLayout(wtext.c_str(), wtext.size(), textFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	if (metrics.width < screenX) {
		return wtext.size();
	}
	SafeRelease(&layout);

	// Binary search.
	int left = 0;
	int right = wtext.size();
	while (right - left > 1) {
		int middle = (right + left) / 2;
		writeFactory->CreateTextLayout(wtext.c_str(), middle, textFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		if (metrics.width < screenX) {
			left = middle;
		}
		else {
			right = middle;
		}
		SafeRelease(&layout);
	}

	// Make final choice
	if (right - left == 1) {
		writeFactory->CreateTextLayout(wtext.c_str(), right, textFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		int width1 = metrics.width;
		SafeRelease(&layout);

		writeFactory->CreateTextLayout(wtext.c_str(), left, textFormat, screenWidth, screenHeight, &layout);
		layout->GetMetrics(&metrics);
		int width2 = metrics.width;
		SafeRelease(&layout);

		if (abs(width1 - screenX) < abs(width2 - screenX)) {
			return right;
		}
		else {
			return left;
		}
	}
	return left;
}