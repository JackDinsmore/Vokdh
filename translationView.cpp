#include "translationView.h"
#include <sstream>


std::vector<std::string> headers = { "h1", "h2", "h3", "p" };


bool TranslationView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush((D2D1::ColorF)styleMap["colors-translation"]["selection"], &selectBrush);
	return true;
}

void TranslationView::extraDiscardDeviceDependentResources() {
	SafeRelease(&selectBrush);
}

bool TranslationView::extraCreateDeviceIndependentResources() {
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

void TranslationView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->Clear(bgColor);

	drawOutline(renderTarget);

	// Draw Text
	TextCounter line = textTree[int(scrollAmount)];
	int parity = int(scrollAmount) % 2;

	int linesTraversed = 0;
	int ypos = -int((scrollAmount - int(scrollAmount)) * getLineHeight(line.type())) + TEXT_BUFFER;
	while (ypos < screenHeight) {
		// English
		std::string text = line.text();
		std::wstring wtext = std::wstring(text.begin(), text.end());
		int lineHeight = getLineHeight(line.type());

		// Draw selection
		if (selection && (selectionCursorY % 2 == parity || selectBoth) && min(cursorPosY, selectionCursorY) <= linesTraversed + int(scrollAmount) &&
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
			ypos, screenWidth, ypos + lineHeight), (parity ? tobairBrush : englishBrush));
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
			ypos, screenWidth, ypos + lineHeight), (parity ? englishBrush : tobairBrush));
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

	// Draw hover
	drawHover(renderTarget);

	// Draw scrollbar
	FLOAT scrollTop = scrollAmount / textTree.size() * screenHeight;
	FLOAT scrollWidth = float(linesTraversed) / textTree.size() * screenHeight;
	renderTarget->FillRectangle({ FLOAT(screenWidth - 10), scrollTop, FLOAT(screenWidth), scrollTop + scrollWidth }, darkBGBrush);
}

bool TranslationView::handleControlShiftKeyPress(int key) {
	int tempCX, tempCY;
	switch (key) {
	case 'A':
		// Select all text
		tempCX = cursorPosX;
		tempCY = cursorPosY;
		cursorPosX = 0;
		cursorPosY = 0;
		selectionCursorY = textTree.size() - 1;
		selectionCursorX = textTree[selectionCursorY].text().size();
		copySelectBoth();
		cursorPosX = tempCX;
		cursorPosY = tempCY;
		return true;
	case 'C':
		/// Check grammar for all words.
		return true;
	}
	return false;
}

bool TranslationView::handleControlKeyPress(int key) {
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
		return true;
	case VK_UP:
		if (cursorPosY > 0) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY--;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return true;
	case VK_DOWN:
		if (cursorPosY < textTree.size() - 1) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY++;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return true;
	case VK_RETURN:
		// Cut the selected text into a new node.
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
	}
	return false;
}

bool TranslationView::handleKeyPress(int key) {
	std::string remainder;
	TextCounter line, backLine;
	int index;
	if (GetKeyState(VK_SHIFT) & 0x8000) {
		if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) {
			if (!selection) {
				selection = true;
				selectionCursorX = cursorPosX;
				selectionCursorY = cursorPosY;
			}
		}
		if (key == VK_TAB) {
			// Decrement header
			line = textTree[cursorPosY];
			index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
			line.type() = index == headers.size() - 1 ? headers[0] : headers[index + 1];
			return true;
		}
	}
	else if (key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN) {
		selection = false;
	}
	switch (key) {
	case VK_TAB:
		// Increment header
		line = textTree[cursorPosY];
		index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
		line.type() = index == 0 ? headers[headers.size() - 1] : headers[index - 1];
		return true;
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
		return true;
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
		return true;
	case VK_UP:
		if (cursorPosY / 2 > 0) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY -= 2;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return true;
	case VK_DOWN:
		if (cursorPosY / 2 < textTree.size() / 2 - 1) {
			int screenX;
			indexToScreen(cursorPosX, cursorPosY, &screenX, nullptr);
			cursorPosY += 2;
			cursorPosX = getIndexFromLine(cursorPosY, screenX);
		}
		return true;
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
		return true;
	case VK_BACK:
		if (!selection) {
			if (cursorPosX != 0) {
				TextCounter line = textTree[cursorPosY];
				if (cursorPosX != line.text().size()) {
					line.text() = line.text().substr(0, cursorPosX - 1) + line.text().substr(cursorPosX);
				}
				else {
					line.text() = line.text().substr(0, cursorPosX - 1);
				}
				cursorPosX--;
			}
			else if (cursorPosY / 2 > 0) {
				textTree[cursorPosY - 2].text() += textTree[cursorPosY].text();
				int otherIndexAddition = (cursorPosY % 2 == 0) ? +1 : -1;
				textTree[cursorPosY - 2 + otherIndexAddition].text() += textTree[cursorPosY + otherIndexAddition].text();
				textTree.remove(cursorPosY);
				cursorPosY -= 2;
				cursorPosX = textTree[cursorPosY].text().size();
			}
		}
		else {
			deleteSelection();
		}
		return true;
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
			return true;
		}
		else {
			deleteSelection();
		}
		return true;
	}
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
		textTree[cursorPosY].text() = textTree[cursorPosY].text().substr(0, cursorPosX) + letter + textTree[cursorPosY].text().substr(cursorPosX);
		cursorPosX++;
		return true;
	}
	return false;
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
			wtext = std::wstring(text.begin(), text.end()).substr(0, indexX - 1) + L"t";
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

void TranslationView::screenToIndex(int screenX, int screenY, int* indexX, int* indexY, bool keepParity) const {
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

		if (keepParity && *indexY % 2 == selectionCursorY % 2) {
			(*indexY)++;
			line++;
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
			/// Move to this line.
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
	screenToIndex(posx - TEXT_BUFFER - outlinePos, posy - TEXT_BUFFER, &cursorPosX, &cursorPosY, true);
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
		int s;
		if (i % 2 == 1) { i++; }
		for (; i < (max(selectionCursorY, cursorPosY) / 2) * 2; i += 2) {
			textTree.remove(min(selectionCursorY, cursorPosY) + 2);/// Fix this!
			s = textTree.size();
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
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
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
	for (i = 0; std::getline(ss, line); i += 2) {
		// Add line
		textTree.insertLine(cursorPosY + i);
		cursor += 2;
		cursor.text() = line;
	}
	cursorPosX = cursor.text().size();
	cursorPosY += i;

	cursor.text() += leftover;
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

void TranslationView::drawOutline(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->DrawLine({ (FLOAT)outlinePos, 0 }, { (FLOAT)outlinePos, (FLOAT)screenHeight }, darkBGBrush, 2);

	int ypos = TEXT_BUFFER;
	for (NodeCounter c = textTree.firstNode(); !c.isLast(); c++) {
		if (c.type() == "h1") {
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

void TranslationView::handleScroll(int scrollTimes) {
	scrollAmount -= scrollTimes;
	scrollAmount = max(scrollAmount, 0);

	scrollAmount = min(scrollAmount, textTree.size() - 1);
}

void TranslationView::handleMouseMotion(int x, int y) {
	int newHoverIndexX, newHoverIndexY;
	screenToIndex(x - (outlinePos + TEXT_BUFFER), y, &newHoverIndexX, &newHoverIndexY);
	if (newHoverIndexY % 2 == 1) {
		std::string line = textTree[newHoverIndexY].text();
		int startIndex = line.substr(0, newHoverIndexX).find_last_of(' ');
		int endIndex = line.substr(newHoverIndexX).find_first_of(' ');
		if (endIndex == -1) { endIndex = line.size() - newHoverIndexX; }
		int periodIndex = line.substr(newHoverIndexX).find_first_of('.');
		if (periodIndex != -1) {
			endIndex = min(endIndex, periodIndex);
		}
		endIndex += newHoverIndexX;
		std::string hoverWord;
		if (startIndex != -1) {
			hoverWord = line.substr(startIndex + 1, endIndex - startIndex - 1);
		}
		else {
			startIndex = 0;
			hoverWord = line.substr(startIndex, endIndex);
		}
		newHoverIndexX = startIndex;
		if (newHoverIndexX != hoverIndexX || newHoverIndexY != hoverIndexY) {
			grammar = dictionary.translate(hoverWord);
			hoverIndexX = newHoverIndexX;
			hoverIndexY = newHoverIndexY;
			indexToScreen(startIndex, hoverIndexY, &hoverScreenX, &hoverScreenY);
			hoverScreenX += outlinePos + TEXT_BUFFER;
			hoverScreenY += getLineHeight(textTree[newHoverIndexY].type());
		}
	}
}

void TranslationView::drawHover(ID2D1HwndRenderTarget* rt) const {
	std::wstring firstLine, secondLine, thirdLine, fourthLine;

	// First line
	switch (grammar.pos) {
	case PART_OF_SPEECH::NONE:
		firstLine = L"Error"; break;
	case PART_OF_SPEECH::NOUN:
		firstLine = L"n."; break;
	case PART_OF_SPEECH::VERB:
		firstLine = L"v."; break;
	case PART_OF_SPEECH::ADJ:
		firstLine = L"adj."; break;
	case PART_OF_SPEECH::PROPER:
		firstLine = L"proper n."; break;
	case PART_OF_SPEECH::PRONOUN:
		firstLine = L"pron."; break;
	case PART_OF_SPEECH::SHORT:
		firstLine = L"short"; break;
	case PART_OF_SPEECH::ADVERB:
		firstLine = L"adv."; break;
	}

	// Second line
	if (grammar.prepositionEnglish != "") {
		secondLine += std::wstring(grammar.prepositionEnglish.begin(), grammar.prepositionEnglish.end()) + L"+";
	}
	for (std::string s : grammar.prefixesEnglish) {
		secondLine += std::wstring(s.begin(), s.end()) + L"+";
	}
	secondLine += std::wstring(grammar.rootEnglish.begin(), grammar.rootEnglish.end()) + L"+";
	for (std::string s : grammar.suffixesEnglish) {
		secondLine += std::wstring(s.begin(), s.end()) + L"+";
	}
	secondLine = secondLine.substr(0, secondLine.size() - 1);

	// Third line
	if (grammar.prepositionTobair!= "") {
		thirdLine += std::wstring(grammar.prepositionTobair.begin(), grammar.prepositionTobair.end()) + L"+";
	}
	for (std::string s : grammar.prefixesTobair) {
		thirdLine += std::wstring(s.begin(), s.end()) + L"+";
	}
	thirdLine += std::wstring(grammar.rootTobair.begin(), grammar.rootTobair.end()) + L"+";
	for (std::string s : grammar.suffixesTobair) {
		thirdLine += std::wstring(s.begin(), s.end()) + L"+";
	}
	thirdLine = thirdLine.substr(0, thirdLine.size() - 1);

	// Fourth line
	fourthLine = std::wstring(grammar.info[0].begin(), grammar.info[0].end()) + L", " +
		std::wstring(grammar.info[1].begin(), grammar.info[1].end()) + L", " +
		std::wstring(grammar.info[2].begin(), grammar.info[2].end());

	// Get width of box
	FLOAT width = 0;
	IDWriteTextLayout* layout;
	DWRITE_TEXT_METRICS metrics;

	// Check if cursor is even over line
	writeFactory->CreateTextLayout(firstLine.c_str(), firstLine.size(), h3EnglishTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	width = max(width, metrics.width);
	SafeRelease(&layout);

	writeFactory->CreateTextLayout(secondLine.c_str(), secondLine.size(), h3EnglishTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	width = max(width, metrics.width);
	SafeRelease(&layout);

	writeFactory->CreateTextLayout(thirdLine.c_str(), thirdLine.size(), h3EnglishTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	width = max(width, metrics.width);
	SafeRelease(&layout);

	writeFactory->CreateTextLayout(fourthLine.c_str(), fourthLine.size(), h3EnglishTextFormat, screenWidth, screenHeight, &layout);
	layout->GetMetrics(&metrics);
	width = max(width, metrics.width);
	SafeRelease(&layout);


	D2D1_RECT_F rect = { (FLOAT)hoverScreenX, (FLOAT)hoverScreenY, (FLOAT)hoverScreenX + width + 2 * TEXT_BUFFER, (FLOAT)hoverScreenY + 80 + TEXT_BUFFER };
	rt->FillRectangle(rect, darkBGBrush);
	rt->DrawRectangle(rect, tobairBrush);
	rect.left += TEXT_BUFFER;
	rect.right -= TEXT_BUFFER;

	rt->DrawText(firstLine.c_str(), firstLine.size(), h3EnglishTextFormat, rect, tobairBrush);
	rect.top += 20;
	rt->DrawText(secondLine.c_str(), secondLine.size(), h3EnglishTextFormat, rect, englishBrush);
	rect.top += 20;
	rt->DrawText(thirdLine.c_str(), thirdLine.size(), h3EnglishTextFormat, rect, tobairBrush);
	rect.top += 20;
	rt->DrawText(fourthLine.c_str(), fourthLine.size(), h3EnglishTextFormat, rect, englishBrush);
	rect.top += 20;
}