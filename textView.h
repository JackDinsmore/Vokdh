#pragma once
#include "d2tools.h"
#include "view.h"
#include "dictionary.h"
#include <map>

#define NUM_IMAGES (NUM_CONSONANTS + 1) * (NUM_VOWELS + 1)
#define ERROR_CHAR "NULL"


class TextView : public View {
public:
	TextView(TextTree& textTree);

public:
	void extraDraw(ID2D1HwndRenderTarget* renderTarget) const override;
	void resize(int width, int height);

private:
	bool extraCreateDeviceIndependentResources(HINSTANCE hInst) override;
	bool extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) override;
	void extraDiscardDeviceDependentResources() override;
	void handleScroll(int scrollTimes) override;
	void handleMouseMotion(int x, int y) override;

	void loadLetterFile();
	void loadLetters();
	void loadLetter(int x, int y, float** letterData);
	std::string takeLetter(std::string, int* index) const;
	void drawHover(ID2D1HwndRenderTarget* rt) const;
	int getWordLength(std::string word) const;
	void recalculatePagePoses();

private:

	Dictionary* dictionary = dictionary->summon();
	Grammar grammar;
	bool hoverBoxOpen = false;
	int hoverIndexX;
	int hoverIndexY;
	int hoverScreenX;
	int hoverScreenY;

	IDWriteTextFormat* textFormat;
	ID2D1SolidColorBrush* whiteBrush;
	IDWriteTextFormat* h3EnglishTextFormat;
	D2D1_RECT_F page;

	const std::filesystem::path lettersPath = exePath.parent_path().parent_path() / "letters.bmp";

	bool* imageData;
	D2D1_SIZE_U imageSize, letterSize, scaledSize;

	ID2D1Bitmap* letterBitmaps[NUM_IMAGES];
	float* letterArrays[NUM_IMAGES];
	std::map<std::string, int> indexMap;

	std::vector<std::pair<int, int>> pagePoses;
	int pageNum;
};