#include "textView.h"
#include "resource.h"

#pragma warning(disable:4018)

#define SCROLL_THRESHOLD 0.5


TextView::TextView(TextTree& textTree) : View(textTree) {
	loadLetterFile();
	loadLetters();
	pagePoses = { { 1, 0 } };
	pageNum = 0;
}

void TextView::loadLetterFile() {
	unsigned char* allData;
	HRSRC resourceVal = FindResource(NULL, MAKEINTRESOURCE(IDB_BITMAP1), RT_BITMAP);
	if (resourceVal != NULL) {
		HGLOBAL hData = LoadResource(0, resourceVal);
		if (NULL != hData) {
			DWORD dataSize = SizeofResource(0, resourceVal);
			allData = (unsigned char*)LockResource(hData);

			imageSize.width = allData[0x12 - 14] + 0x100 * allData[0x13 - 14] + 0x10000 * allData[0x14 - 14] + 0x1000000 * allData[0x15 - 14];
			imageSize.height = allData[0x16 - 14] + 256 * allData[0x17 - 14] + 0x10000 * allData[0x18 - 14] + 0x1000000 * allData[0x19 - 14];
			int stripWidth = imageSize.width / 32;// Each strip is 32 pixels
			if (imageSize.width % 32 != 0) {
				stripWidth++;// Round up
			}
			stripWidth *= 4;// There are 4 chars per strip
			imageData = new bool[imageSize.width * imageSize.height];

			int counter = 0;
			for (int y = imageSize.height - 1; y >= 0; y--) {
				for (int x = 0; 8 * x < imageSize.width; x++) {// Read in one 32-pixel line; 8 pixels per char.
					char thisChar = allData[x + stripWidth * y + 48];
					for (int i = 7; i >= 0; i--) {
						if (8 * x + (7 - i) >= imageSize.width) { continue; }
						imageData[counter] = (thisChar >> i) & 1;
						counter++;
					}
				}
			}
		}
	}
}

void TextView::loadLetters() {
	std::string special[10] = { "0", "1", "2", "3", "4", "5", "6", "7", ".", ERROR_CHAR };

	letterSize = { imageSize.width / 20, imageSize.height / 10 };
	float scale = (float)styleMap["text"]["font-size"] / 100;
	scaledSize = { (unsigned int)(letterSize.width * scale), (unsigned int)(letterSize.height * scale) };

	int i = 0;
	for (int x = 0; x < 20; x++) {
		std::string vowel = "";
		if (x / 2 > 0) {
			vowel = vowels[x / 2 - 1];
		}
		for (int y = 0; y < 10; y++) {
			std::string consonant = consonants[y];
			if (x % 2 == 1 && y == 9) {
				consonant = special[x / 2];
				vowel = "";
			}
			else if (x % 2 == 1) {
				consonant = consonants[y + 10];
			}
			indexMap[consonant + vowel] = i;
			loadLetter(letterSize.width * x, letterSize.height * y, &letterArrays[i]);
			i++;
		}
	}

	delete[] imageData;
}

void TextView::loadLetter(int x, int y, float** letterData) {
	D2D1::ColorF onColor = (D2D1::ColorF)styleMap["colors-translation"]["tobair"];

	int counter = 0;
	*letterData = new float[letterSize.width * letterSize.height * 4];
	
	for (int iy = y; iy < y + letterSize.height; iy++) {
		for (int ix = x; ix < x + letterSize.width; ix++) {
			if (imageData[ix + imageSize.width * iy]) {
				(*letterData)[4 * counter] = 0;
				(*letterData)[4 * counter + 1] = 0;
				(*letterData)[4 * counter + 2] = 0;
				(*letterData)[4 * counter + 3] = 0;

			}
			else {
				(*letterData)[4 * counter] = onColor.r;
				(*letterData)[4 * counter + 1] = onColor.g;
				(*letterData)[4 * counter + 2] = onColor.b;
				(*letterData)[4 * counter + 3] = 1;
			}
			counter++;
		}
	}
}

void TextView::resize(int x, int y) {
	screenWidth = x;
	screenHeight = y;

	float aspect = (float)styleMap["text"]["page-width"] / (float)styleMap["text"]["page-height"];
	if (aspect < screenWidth / (float)screenHeight) {
		page.top = 20;
		page.bottom = screenHeight - 20;
		float width = aspect * (screenHeight - 40);
		page.left = (screenWidth - width) / 2;
		page.right = (screenWidth + width) / 2;
	}
	else {
		page.left = 20;
		page.right = screenWidth - 20;
		float width = aspect * (screenWidth - 40);
		page.top = (screenWidth - width) / 2;
		page.bottom = (screenWidth + width) / 2;
	}

	recalculatePagePoses();
}

void TextView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ 0, 0, screenWidth, screenHeight }, backBrush);
	bool lastAddedSpace;

	TextCounter textCounter = textTree[pagePoses[pageNum].first];
	int textIndex = pagePoses[pageNum].second;

	std::string line = lowerString(textCounter.text());
	int nColumns = (int)styleMap["text"]["num-columns"];

	for (int column = 0; column < nColumns; column++) {
		int x = page.left + (page.right - page.left) * column / nColumns;
		int y = 20;
		lastAddedSpace = false;
		while (y <= page.bottom) {
			int nextSpace = find(line.begin() + textIndex, line.end(), ' ') - line.begin();
			std::string word = line.substr(textIndex, nextSpace - textIndex);
			int length = getWordLength(word);
			if (y + length > page.bottom) {
				if (page.top + length > page.bottom) {
					// Word is too long to be pushed to the next line.
					int breakIndex;
					for (breakIndex = nextSpace; y + length > page.bottom; breakIndex--) {
						word = line.substr(textIndex, breakIndex - textIndex);
						length = getWordLength(word);
					}
					textIndex = breakIndex;
				}
				break;
			}

			std::string letter;
			while (letter == "") {
				letter = takeLetter(line, &textIndex);
				if (letter == "") {
					if (textCounter.isLast()) { break; }
					textCounter++;
					if (textCounter.isLast()) { break; }
					textCounter++;
					textIndex = 0;
					line = lowerString(textCounter.text());
					if(!lastAddedSpace) y += scaledSize.height;
					if (y > page.bottom) {
						break;
					}
				}
			}
			if (letter == "") { break; }

			if (letter == ")") {
				/// TO DO: periods and paragraphs
			}
			else if (letter == " ") {}
			else {
				if (indexMap.find(letter) == indexMap.end()) {
					/// Render error character
					renderTarget->DrawBitmap(letterBitmaps[indexMap.at(ERROR_CHAR)], { (FLOAT)x, (FLOAT)y, (FLOAT)x + scaledSize.width, (FLOAT)y + scaledSize.height });
				}
				else {
					renderTarget->DrawBitmap(letterBitmaps[indexMap.at(letter)], { (FLOAT)x, (FLOAT)y, (FLOAT)x + scaledSize.width, (FLOAT)y + scaledSize.height });
				}
			}
			y += scaledSize.height;
			lastAddedSpace = false;
		}
	}

	// Draw hover
	drawHover(renderTarget);
}

bool TextView::extraCreateDeviceIndependentResources(HINSTANCE hInst) {
	std::string englishFontName = (std::string)styleMap["fonts"]["english"];
	if (englishFontName.empty()) { englishFontName = "Calibri"; }

	int h3Size = (int)styleMap["fonts"]["h3-size"];
	if (h3Size == 0) { h3Size = 18; }

	writeFactory->CreateTextFormat(std::wstring(englishFontName.begin(), englishFontName.end()).c_str(), NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, h3Size, L"", &h3EnglishTextFormat);
	return true;
}

bool TextView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	D2D1_BITMAP_PROPERTIES properties;
	renderTarget->GetDpi(&properties.dpiX, &properties.dpiY);
	properties.pixelFormat = { DXGI_FORMAT_R32G32B32A32_FLOAT, D2D1_ALPHA_MODE_PREMULTIPLIED };

	for (int i = 0; i < NUM_IMAGES; i++) {
		HRESULT hr = renderTarget->CreateBitmap(letterSize, letterArrays[i],
			sizeof(float) * 4 * letterSize.width, properties, &letterBitmaps[i]);
		if (FAILED(hr)) { return false; }
	}
	return true;
}

void TextView::extraDiscardDeviceDependentResources() {
	for (int i = 0; i < NUM_IMAGES; i++) {
		SafeRelease(&letterBitmaps[i]);
	}
}

void TextView::handleScroll(int scrollTimes) {
	if (abs(scrollTimes) < SCROLL_THRESHOLD) { return; }
	if (scrollTimes > 0) {
		if (pageNum > 0) {
			pageNum--;
		}
		return;
	}

	bool lastAddedSpace;

	int newIndexY = pagePoses[pageNum].first;
	TextCounter textCounter = textTree[newIndexY];
	int newIndexX = pagePoses[pageNum].second;

	std::string line = lowerString(textCounter.text());

	int nColumns = (int)styleMap["text"]["num-columns"];

	for (int column = 0; column < nColumns; column++) {
		int x = page.left + (page.right - page.left) * column / nColumns;
		int y = 20;
		lastAddedSpace = false;
		while (y <= page.bottom) {
			int oldy = y;
			int nextSpace = find(line.begin() + newIndexX, line.end(), ' ') - line.begin();
			std::string word = line.substr(newIndexX, nextSpace - newIndexX);
			int length = getWordLength(word);
			if (y + length > page.bottom) {
				if (page.top + length > page.bottom) {
					// Word is too long to be pushed to the next line.
					int breakIndex;
					for (breakIndex = nextSpace; y + length > page.bottom; breakIndex--) {
						word = line.substr(newIndexX, breakIndex - newIndexX);
						length = getWordLength(word);
					}
					newIndexX = breakIndex;
				}
				break;
			}

			std::string letter;
			while (letter == "") {
				letter = takeLetter(line, &newIndexX);
				if (letter == "") {
					if (textCounter.isLast()) { return; }
					textCounter++;
					if (textCounter.isLast()) { return; }
					textCounter++;
					newIndexX = 0;
					newIndexY += 2;
					line = lowerString(textCounter.text());
					if (!lastAddedSpace) y += scaledSize.height;
					if (y > page.bottom) {
						break;
					}
				}
			}
			if (letter == "") { break; }

			y += scaledSize.height;
			lastAddedSpace = false;
		}
	}

	pagePoses.push_back({ newIndexY, newIndexX });
	pageNum++;
}

std::string TextView::takeLetter(std::string line, int* index) const {
	if (*index >= line.size()) { return ""; }
	std::string ret = "";
	if ('0' <= line[*index] && line[*index] <= '9') {
		ret = std::string(1, line[*index]);
		(*index)++;
		return ret;
	}
	if (line[*index] == '.' || line[*index] == ' ' || line[*index] == ')') {
		ret = std::string(1, line[*index]);
		(*index)++;
		return ret;
	}

	if (std::find(vowels.begin(), vowels.end(), std::string(1, line[*index]))!=vowels.end()) {
		ret += "'";
		ret += line[*index];
		(*index)++;
	}
	else {
		ret += line[*index];
		(*index)++;
		while (std::find(consonants.begin(), consonants.end(), ret + std::string(1, line[*index])) != consonants.end()) {
			ret += line[*index];
			(*index)++;
		}
	}
	while (std::find(vowels.begin(), vowels.end(), std::string(1, line[*index])) != vowels.end()) {
		ret += line[*index];
		(*index)++;
	}
	return ret;
}

void TextView::handleMouseMotion(int screenX, int screenY) {
	hoverBoxOpen = false;
	if (screenX < page.left || screenX > page.right) { return; }
	if (screenY < page.top || screenY > page.bottom) { return; }
	bool lastAddedSpace;

	int newHoverIndexY = pagePoses[pageNum].first;
	TextCounter textCounter = textTree[newHoverIndexY];
	int newHoverIndexX = pagePoses[pageNum].second;

	std::string line = lowerString(textCounter.text());

	int nColumns = (int)styleMap["text"]["num-columns"];
	int columnNum = (screenX - page.left) / float(page.right - page.left) * nColumns;

	for (int column = 0; column < nColumns; column++) {
		int x = page.left + (page.right - page.left) * column / nColumns;
		int y = 20;
		lastAddedSpace = false;
		while (y <= page.bottom) {
			int oldy = y;
			int nextSpace = find(line.begin() + newHoverIndexX, line.end(), ' ') - line.begin();
			std::string word = line.substr(newHoverIndexX, nextSpace - newHoverIndexX);
			int length = getWordLength(word);
			if (y + length > page.bottom) {
				if (page.top + length > page.bottom) {
					// Word is too long to be pushed to the next line.
					int breakIndex;
					for (breakIndex = nextSpace; y + length > page.bottom; breakIndex--) {
						word = line.substr(newHoverIndexX, breakIndex - newHoverIndexX);
						length = getWordLength(word);
					}
					newHoverIndexX = breakIndex;
				}
				break;
			}

			std::string letter;
			while (letter == "") {
				letter = takeLetter(line, &newHoverIndexX);
				if (letter == "") {
					if (textCounter.isLast()) { break; }
					textCounter++;
					if (textCounter.isLast()) { break; }
					textCounter++;
					newHoverIndexX = 0;
					newHoverIndexY += 2;
					line = lowerString(textCounter.text());
					if (!lastAddedSpace) y += scaledSize.height;
					if (y > page.bottom) {
						break;
					}
				}
			}
			if (letter == "") { break; }

			y += scaledSize.height;
			if (column == columnNum && oldy < screenY && y > screenY) {
				
				// Word found
				if (newHoverIndexX != hoverIndexX || newHoverIndexY != hoverIndexY) {
					int recentSpace = line.find_last_of(' ', nextSpace-1);
					if (recentSpace == -1) { recentSpace = 0; }
					grammar = dictionary->translate(line.substr(recentSpace, nextSpace - recentSpace));
					hoverIndexX = newHoverIndexX;
					hoverIndexY = newHoverIndexY;
					hoverScreenX = page.left + float(page.right - page.left) / (nColumns) * (columnNum + 1);
					hoverScreenY = oldy;
				}
				hoverBoxOpen = true;
				return;

			}
			lastAddedSpace = false;
		}
	}
}

void TextView::drawHover(ID2D1HwndRenderTarget* rt) const {
	if (!hoverBoxOpen) { return; }
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
	if (grammar.prepositionTobair != "") {
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
	fourthLine = L"";
	for (int i = 0; i < 3; i++) {
		if (!grammar.info[i].empty()) {
			if (i != 0) {
				fourthLine += L", ";
			}
			fourthLine += std::wstring(grammar.info[i].begin(), grammar.info[i].end());
		}
	}

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


	int nLines = 3;
	if (!fourthLine.empty()) { nLines++; }
	D2D1_RECT_F rect = { (FLOAT)hoverScreenX, (FLOAT)hoverScreenY, (FLOAT)hoverScreenX + width + 2 * TEXT_BUFFER, (FLOAT)hoverScreenY + 20 * nLines + TEXT_BUFFER };
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

int TextView::getWordLength(std::string word) const {
	int index = 0;
	int i = 0;
	while (index < word.size()) {
		takeLetter(word, &index);
		i++;
	}
	return i * scaledSize.height;
}

void TextView::recalculatePagePoses() {
	pagePoses = { { 1, 0 } };
	pageNum = 0;
}