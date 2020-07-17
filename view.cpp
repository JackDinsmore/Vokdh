#include "view.h"
#include <sstream>

std::vector<std::string> headers = { "h1", "h2", "h3", "p" };

void View::drawOutline(ID2D1HwndRenderTarget* renderTarget) const {
	int width = renderTarget->GetSize().width;
	int height = renderTarget->GetSize().height;
	renderTarget->DrawLine({ (FLOAT)outlinePos, 0 }, { (FLOAT)outlinePos, (FLOAT)height }, darkBGBrush, 2);

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




bool TranslationView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["selection"], &selectBrush);
	return true;
}

void TranslationView::extraDiscardDeviceDependentResources() {
	SafeRelease(&selectBrush);
}

bool TranslationView::extraCreateDeviceIndependentResources() {
	return true;
}

void TranslationView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->Clear(bgColor);

	drawOutline(renderTarget);

	int width = renderTarget->GetSize().width;
	int height = renderTarget->GetSize().height;

	// Draw Text
	TextCounter line = textTree[int(scrollAmount)];
	int parity = int(scrollAmount) % 2;

	int linesTraversed = 0;
	int ypos = -int((scrollAmount - int(scrollAmount)) * getLineHeight(line.type())) + TEXT_BUFFER;
	while (ypos < height) {
		// English
		std::string text = line.text();
		std::wstring wtext = std::wstring(text.begin(), text.end());
		int lineHeight = getLineHeight(line.type());

		// Draw selection
		if (selection && (selectionCursorY % 2 == parity || selectBoth) && min(cursorPosY, selectionCursorY) <= linesTraversed + int(scrollAmount) &&
			linesTraversed + int(scrollAmount) <= max(cursorPosY, selectionCursorY) ) {
			// Select this text
			int right;
			int indexRight = line.text().size();
			if (linesTraversed + int(scrollAmount) == max(cursorPosY, selectionCursorY)) {
				indexRight = cursorPosY > selectionCursorY ? cursorPosX : selectionCursorX;
				if (cursorPosY == selectionCursorY) {
					indexRight = max(cursorPosX, selectionCursorX);
				}
			}
			indexToScreen(indexRight, linesTraversed + int(scrollAmount), &right, nullptr);

			int left = 0;
			if (linesTraversed + int(scrollAmount) == min(cursorPosY, selectionCursorY)) {
				int indexLeft = cursorPosY < selectionCursorY ? cursorPosX : selectionCursorX;
				if (cursorPosY == selectionCursorY) {
					indexLeft = min(cursorPosX, selectionCursorX);
				}
				indexToScreen(indexLeft,
					linesTraversed + int(scrollAmount), &left, nullptr);
			}
			renderTarget->FillRectangle({(FLOAT)outlinePos + TEXT_BUFFER + (FLOAT) left, (FLOAT)ypos, (FLOAT)right + TEXT_BUFFER + (FLOAT)outlinePos,
				(FLOAT)ypos+ (FLOAT)lineHeight}, selectBrush);
		}

		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos + TEXT_BUFFER,
			ypos, width, ypos + lineHeight), (parity ? tobairBrush : englishBrush));
		ypos += lineHeight;
		linesTraversed++;
		if (!line.isLast()) {
			line++;
		}
		else {
			break;
		}

		// Space
		if (parity) {
			if (!line.isLast()) {
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

		// Draw selection
		if (selection && (selectionCursorY % 2 != parity || selectBoth) && min(cursorPosY, selectionCursorY) <= linesTraversed + int(scrollAmount) &&
			linesTraversed + int(scrollAmount) <= max(cursorPosY, selectionCursorY)) {
			// Select this text
			int right;
			int indexRight = line.text().size();
			if (linesTraversed + int(scrollAmount) == max(cursorPosY, selectionCursorY)) {
				indexRight = cursorPosY > selectionCursorY ? cursorPosX : selectionCursorX;
				if (cursorPosY == selectionCursorY) {
					indexRight = max(cursorPosX, selectionCursorX);
				}
			}
			indexToScreen(indexRight, linesTraversed + int(scrollAmount), &right, nullptr);

			int left = 0;
			if (linesTraversed + int(scrollAmount) == min(cursorPosY, selectionCursorY)) {
				int indexLeft = cursorPosY < selectionCursorY ? cursorPosX : selectionCursorX;
				if (cursorPosY == selectionCursorY) {
					indexLeft = min(cursorPosX, selectionCursorX);
				}
				indexToScreen(indexLeft,
					linesTraversed + int(scrollAmount), &left, nullptr);
			}
			renderTarget->FillRectangle({ (FLOAT)outlinePos + TEXT_BUFFER + (FLOAT)left, (FLOAT)ypos, (FLOAT)right + TEXT_BUFFER + (FLOAT)outlinePos,
				(FLOAT)ypos + (FLOAT)lineHeight }, selectBrush);
		}

		renderTarget->DrawText(wtext.c_str(), wtext.size(), getTextFormat(line.type()), D2D1::RectF(outlinePos + TEXT_BUFFER,
			ypos, width, ypos + lineHeight), (parity ? englishBrush : tobairBrush));
		ypos += lineHeight;
		linesTraversed++;
		if (!line.isLast()) {
			line++;
		}
		else {
			break;
		}

		// Space
		if (!parity) {
			if (!line.isLast()) {
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
		int tempCX = cursorPosX;
		int tempCY = cursorPosY;
		cursorPosX = 0;
		cursorPosY = 0;
		selectionCursorY = textTree.size() - 1;
		selectionCursorX = textTree[selectionCursorY].text().size();
		copySelectBoth();
		cursorPosX = tempCX;
		cursorPosY = tempCY;
		return;
	}
}

void TranslationView::handleControlKeyPress(int key) {
	TextCounter line;
	int index;
	switch (key) {
	case 'A':
		// Select all of type
		if (cursorPosY % 2 == 0) {
			selectionCursorX = 0;
			selectionCursorY = 0;
			cursorPosY = textTree.size() - 2;
			cursorPosX = textTree[cursorPosY].text().size();
		}
		else {
			selectionCursorX = 0;
			selectionCursorY = 1;
			cursorPosY = textTree.size() - 1;
			cursorPosX = textTree[cursorPosY].text().size();
		}
		selection = true;
		return;
	case 'Q':
		// Decrement header
		line = textTree[cursorPosY];
		index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
		line.type() = index == 0 ? headers[headers.size() - 1] : headers[index - 1];
		return;
	case 'E':
		// Increment header
		line = textTree[cursorPosY];
		index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
		line.type() = index == headers.size() - 1 ? headers[0] : headers[index + 1];
		return;
	case VK_UP:
		if (cursorPosY > 0) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY--;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return;
	case VK_DOWN:
		if (cursorPosY < textTree.size() - 1) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY++;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return;
	case VK_RETURN:
		// Cut the selected text into a new node.
		return;
	case 'C':
		if (!selection) { return; }
		copySelected();
		// Copy
		return;
	case 'X':
		if (!selection) { return; }
		copySelected();
		deleteSelection();
		// Cut
		return;
	case 'V':
		if (selection) { deleteSelection(); }
		paste();
		// Paste
		return;
	}
}

void TranslationView::handleKeyPress(int key) {
	std::string remainder;
	TextCounter line, backLine;
	if (GetKeyState(VK_SHIFT) & 0x8000) {
		if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) {
			if (!selection) {
				selection = true;
				selectionCursorX = cursorPosX;
				selectionCursorY = cursorPosY;
			}
		}
	}
	else if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) {
		selection = false;
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
	case VK_RETURN:
		if (selection) { deleteSelection(); }
		line = textTree[cursorPosY];
		textTree.insertLine(cursorPosY);
		remainder = line.text().substr(cursorPosX);
		line.text() = line.text().substr(0, cursorPosX);
		line++; line++;

		line.text() = remainder;
		cursorPosY += 2;
		cursorPosX = 0;
		return;
	case VK_BACK:
		if (!selection) {
			if (cursorPosX != 0) {
				TextCounter line = textTree[cursorPosY];
				line.text() = line.text().substr(0, cursorPosX - 1) + line.text().substr(cursorPosX + 1);
				cursorPosX--;
			}
			else if (cursorPosY / 2 > 0) {
				int lowerCursor = (cursorPosY / 2 - 1) * 2;
				int thisCursor = (cursorPosY / 2) * 2;
				textTree[lowerCursor].text() += textTree[thisCursor].text();
				cursorPosX = textTree[thisCursor].text().size() - 2;
				textTree[lowerCursor + 1].text() += textTree[thisCursor + 1].text();
				textTree.remove(cursorPosY);
				cursorPosY -= 2;
			}
		}
		else {
			deleteSelection();
		}
		return;
	case VK_DELETE:
		if (!selection) {
			if (cursorPosX != textTree[cursorPosY].text().size()) {
				TextCounter line = textTree[cursorPosY];
				line.text() = line.text().substr(0, cursorPosX) + line.text().substr(cursorPosX + 1);
			}
			else if (cursorPosY / 2 < textTree.size() / 2) {
				int higherCursor = (cursorPosY / 2 + 1) * 2;
				int thisCursor = (cursorPosY / 2) * 2;
				textTree[thisCursor].text() += textTree[higherCursor].text();
				textTree[thisCursor + 1].text() += textTree[higherCursor + 1].text();
				textTree.remove(cursorPosY + 2);
			}
			return;
		}
		else {
			deleteSelection();
		}
	}
	if (key == VK_SPACE || (48 <= key && key <= 57) || (65 <= key && key <= 90)) {
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
		textTree[cursorPosY].text() = textTree[cursorPosY].text().substr(0, cursorPosX) + letter + textTree[cursorPosY].text().substr(cursorPosX);
		cursorPosX++;
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
			if (!count.isLast()) {
				ypos += lineHeight;
				count++;
			}
			else {
				break;
			}

			// Space
			if (parity) {
				if (!count.isLast()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}

			// Tobair
			lineHeight = getLineHeight(count.type());
			if (count != line && !count.isLast()) {
				ypos += lineHeight;
				count++;
			}
			else {
				break;
			}

			// Space
			if (!parity) {
				if (!count.isLast()) {
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
			if (!line.isLast()) {
				ypos += lineHeight;
				(*indexY)++;
				line++;
			}
			else {
				break;
			}

			// Space
			if (parity) {
				if (ypos < screenY && !line.isLast()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}

			// Tobair
			diffY = ypos - screenY;
			lineHeight = getLineHeight(line.type());

			if (ypos < screenY && !line.isLast()) {
				ypos += lineHeight;
				(*indexY)++;
				line++;
			}
			else {
				break;
			}

			// Space
			if (!parity) {
				if (ypos < screenY && !line.isLast()) {
					ypos += lineHeight;
				}
				else {
					break;
				}
			}
		}

		if (*indexY > 0) {
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
	if (GetKeyState(VK_SHIFT) & 0x8000) {
		selectedWhileClicking = true;

		if (posx > TEXT_BUFFER + outlinePos) {
			screenToIndex(posx - TEXT_BUFFER - outlinePos, posy - TEXT_BUFFER, &cursorPosX, &cursorPosY);
			selection = true;
		}
	}
	else {
		selectedWhileClicking = false;
		if (posx > TEXT_BUFFER + outlinePos) {
			screenToIndex(posx - TEXT_BUFFER - outlinePos, posy - TEXT_BUFFER, &cursorPosX, &cursorPosY);
			selectionCursorX = cursorPosX;
			selectionCursorY = cursorPosY;
		}
		else {
			// Move to this line.
		}
	}
}

void TranslationView::handleLeftUnclick(int posx, int posy) {
	if (!selectedWhileClicking) {
		selection = false;
	}

	if (selectBoth) {
		copySelectBoth();
		selection = false;
	}
}

void TranslationView::handleDrag(int posx, int posy) {
	if (posx < 0 || posy < 0) { return; }
	screenToIndex(posx - TEXT_BUFFER - outlinePos, posy - TEXT_BUFFER, &cursorPosX, &cursorPosY);
	if (selectionCursorX != cursorPosX || selectionCursorY != cursorPosY) {
		selection = true;
		selectedWhileClicking = true;

		if (GetKeyState(VK_CONTROL) & 0x8000) {
			selectBoth = true;
		}
		else {
			selectBoth = false;
		}
	}
}

void TranslationView::deleteSelection() {
	selection = false;
	TextCounter tcBegin = textTree[min(selectionCursorY, cursorPosY)];
	
	if (selectionCursorY != cursorPosY) {
		TextCounter tcBeginOther = textTree[min(selectionCursorY, cursorPosY) + (cursorPosY % 2 == 0 ? +1 : -1)];
		// Handle the first line
		tcBegin.text() = tcBegin.text().substr(0, selectionCursorY > cursorPosY ? cursorPosX : selectionCursorX);
		// Delete every middle line
		int i = min(selectionCursorY, cursorPosY) + 1;
		if (i % 2 == 1) { i++; }
		for (; i < (max(selectionCursorY, cursorPosY) / 2) * 2; i += 2) {
			textTree.remove(i);
		}

		// Handle last line
		TextCounter tcEnd = textTree[min(selectionCursorY, cursorPosY) + 2];
		TextCounter tcEndOther = textTree[min(selectionCursorY, cursorPosY) + 2 + (cursorPosY % 2 == 0 ? +1 : -1)];
		tcBegin.text() += tcEnd.text().substr(selectionCursorY < cursorPosY ? cursorPosX : selectionCursorX);
		tcBeginOther.text() += tcEndOther.text();
		textTree.remove(min(selectionCursorY, cursorPosY) + 2);
	}
	else {
		tcBegin.text() = tcBegin.text().substr(0, min(cursorPosX, selectionCursorX)) + tcBegin.text().substr(max(cursorPosX, selectionCursorX));
	}
	cursorPosX = selectionCursorY < cursorPosY ? selectionCursorX : cursorPosX;
	if (selectionCursorY == cursorPosY) {
		cursorPosX = min(selectionCursorX, cursorPosX);
	}
	cursorPosY = min(selectionCursorY, cursorPosY);
}

void TranslationView::copySelected() {
	std::string s;

	// Write selected text to string
	// Handle the first line
	TextCounter tc = textTree[min(selectionCursorY, cursorPosY)];
	if (selectionCursorY != cursorPosY) {
		s = tc.text().substr(selectionCursorY > cursorPosY ? cursorPosX : selectionCursorX) + "\n";

		// Handle every middle line
		for (int i = min(selectionCursorY, cursorPosY) + 2; i < max(selectionCursorY, cursorPosY); i += 2) {
			tc += 2;
			s += tc.text() + "\n";
		}

		// Handle last line
		tc += 2;
		s += tc.text().substr(0, 1 + selectionCursorY < cursorPosY ? cursorPosX : selectionCursorX);
	}
	else {
		s = tc.text().substr(min(selectionCursorX, cursorPosX), abs(selectionCursorX - cursorPosX));
	}


	OpenClipboard(0);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size()+1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size()+1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

void TranslationView::copySelectBoth() {
}

void TranslationView::paste() {
	std::stringstream ss;

	OpenClipboard(0);
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr) { return; }

	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr) { return; }
	ss << pszText;
	GlobalUnlock(hData);
	CloseClipboard();

	std::string line;
	std::getline(ss, line);
	TextCounter cursor = textTree[cursorPosY];
	std::string leftover = cursor.text().substr(cursorPosX);
	cursor.text() = cursor.text().substr(0, cursorPosX) + line;
	int i;
	for (i = 0; std::getline(ss, line); i+= 2) {
		// Add line
		textTree.insertLine(cursorPosY + i);
		cursor+=2;
		cursor.text() = line;
	}
	cursorPosX = cursor.text().size();
	cursorPosY += i;

	cursor.text() += leftover;
}