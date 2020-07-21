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

	// Draw scrollbar
	FLOAT scrollTop = scrollAmount / textTree.size() * screenHeight;
	FLOAT scrollWidth = float(linesTraversed) / textTree.size() * screenHeight;
	renderTarget->FillRectangle({ FLOAT(screenWidth - 10), scrollTop, FLOAT(screenWidth), scrollTop + scrollWidth }, darkBGBrush);
}

bool TranslationView::handleControlShiftKeyPress(int key) {
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
		return true;
	}
	return false;
}

bool TranslationView::handleControlKeyPress(int key) {
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
		return true;
	case 'Q':
		// Decrement header
		line = textTree[cursorPosY];
		index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
		line.type() = index == 0 ? headers[headers.size() - 1] : headers[index - 1];
		return true;
	case 'E':
		// Increment header
		line = textTree[cursorPosY];
		index = std::find(headers.begin(), headers.end(), line.type()) - headers.begin();
		line.type() = index == headers.size() - 1 ? headers[0] : headers[index + 1];
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