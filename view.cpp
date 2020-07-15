#include "view.h"

void View::drawOutline(ID2D1HwndRenderTarget* renderTarget) const {
	int width = renderTarget->GetSize().width;
	int height = renderTarget->GetSize().height;
	renderTarget->DrawLine({ (FLOAT)outlinePos, 0 }, { (FLOAT)outlinePos, (FLOAT)height }, darkBGBrush, 2);
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
	styleMap = styleMap.summon();
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["dark-background"], &darkBGBrush);
	return extraCreateDeviceDependentResources(renderTarget);
}

void View::discardDeviceDependentResources() {
	SafeRelease(&darkBGBrush);
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
	scrollAmount = 0;
}




bool TranslationView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	styleMap = styleMap.summon();
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["foreground"], &foreBrush);
	bgColor = (D2D1::ColorF)styleMap["colors-translation"]["background"];
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["english"], &englishBrush);
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["tobair"], &tobairBrush);
	return true;
}

void TranslationView::extraDiscardDeviceDependentResources() {
	SafeRelease(&foreBrush);
	SafeRelease(&englishBrush);
	SafeRelease(&tobairBrush);
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
	TextCounter line = textTree[max(int(scrollAmount), 0)];
	int parity = max(int(scrollAmount), 0) % 2;

	int linesTraversed = 0;
	int ypos = -int((scrollAmount - max(int(scrollAmount), 0)) * getLineHeight(line.type())) + TEXT_BUFFER;
	while (ypos < height) {
		linesTraversed++;
		// English
		std::string text = line.text();
		std::wstring wtext = std::wstring(text.begin(), text.end());
		int lineHeight = getLineHeight(line.type());
		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos + TEXT_BUFFER, ypos, width, ypos + lineHeight), (parity ? tobairBrush : englishBrush));
		ypos += lineHeight;
		if (line != textTree.last()) {
			line++;
		}
		else {
			break;
		}

		// Space
		if (parity) {
			if (line != textTree.last()) {
				ypos += lineHeight;
			}
			else {
				break;
			}
		}

		// Tobair
		text = line.text();
		wtext = std::wstring(text.begin(), text.end());
		lineHeight = getLineHeight(line.type());
		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos + TEXT_BUFFER, ypos, width, ypos + lineHeight), (parity ? englishBrush : tobairBrush));
		ypos += lineHeight;
		if (line != textTree.last()) {
			line++;
		}
		else {
			break;
		}

		// Space
		if (!parity) {
			if (line != textTree.last()) {
				ypos += lineHeight;
			}
			else {
				break;
			}
		}
	}

	// Draw cursor
	int screenX = -100, screenY = -100;
	int lineHeight = getLineHeight(textTree[cursorPosY].type());
	indexToScreen(cursorPosX, cursorPosY, &screenX, &screenY);
	screenX += outlinePos;
	renderTarget->DrawLine({ (FLOAT)screenX + TEXT_BUFFER, (FLOAT)screenY }, { (FLOAT)screenX + TEXT_BUFFER, (FLOAT)(screenY + lineHeight) }, foreBrush, 2);
	
	// Draw scrollbar
	FLOAT scrollTop = scrollAmount / textTree.size() * height;
	FLOAT scrollWidth = float(linesTraversed) / textTree.size() * height;
	renderTarget->FillRectangle({FLOAT(width - 10), scrollTop, FLOAT(width), scrollTop + scrollWidth }, darkBGBrush);
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
	int parity = max(int(scrollAmount), 0) % 2;

	if (screenY && indexY >= scrollAmount) {
		TextCounter count = textTree[max(0, int(scrollAmount))];
		int ypos = -int((scrollAmount - max(0, int(scrollAmount))) * getLineHeight(line.type())) + TEXT_BUFFER;
		while (count != line && ypos < MAX_HEIGHT) {
			// English
			int lineHeight = getLineHeight(count.type());
			ypos += lineHeight;
			if (count != textTree.last()) {
				count++;
			}
			else {
				break;
			}

			// Space
			if (parity) {
				if (count != textTree.last()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}

			// Tobair
			lineHeight = getLineHeight(count.type());
			ypos += lineHeight;
			if (count != textTree.last()) {
				count++;
			}
			else {
				break;
			}

			// Space
			if (!parity) {
				if (count != textTree.last()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}
		}
		
		if (ypos < MAX_HEIGHT) {
			*screenY = ypos;
		}
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
	if (indexY && indexX) {
		TextCounter line = textTree[max(int(scrollAmount), 0)];
		int parity = max(int(scrollAmount), 0) % 2;
		int ypos = -int((scrollAmount - max(int(scrollAmount), 0)) * getLineHeight(line.type())) + TEXT_BUFFER;
		*indexY = max(int(scrollAmount), 0);
		int diffY = 0;

		while (ypos < screenHeight && ypos < screenY) {

			diffY = ypos - screenY;
			int lineHeight = getLineHeight(line.type());
			if (line != textTree.last()) {
				ypos += lineHeight;
				(*indexY)++;
				line++;
			}
			else {
				break;
			}

			// Space
			if (parity) {
				if (line != textTree.last()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}

			// Tobair
			diffY = ypos - screenY;
			lineHeight = getLineHeight(line.type());

			if (line != textTree.last()) {
				ypos += lineHeight;
				(*indexY)++;
				line++;
			}
			else {
				break;
			}

			// Space
			if (!parity) {
				if (line != textTree.last()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}
		}

		int diffYNow = ypos - screenY;
		if (abs(diffY) < abs(diffYNow)) {
			ypos = diffY + screenY;
			line--;
			(*indexY)--;
		}

		// Do some stuff
		*indexX = getIndexFromLine(*indexY, screenX);
	}
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

void TranslationView::extraHandleLeftClick(int posx, int posy) {
	screenToIndex(posx - TEXT_BUFFER - outlinePos, posy - TEXT_BUFFER, &cursorPosX, &cursorPosY);
}