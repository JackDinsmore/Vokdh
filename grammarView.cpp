#include "grammarView.h"


std::vector<std::wstring> grammarText = {
	L"C - form - C - Animacy - C - Number:\n"
	"Form\t\t\tAnimacy\t\t\tNumber\n"
	"Agent\t\te/ee\tPeople\t\ti\t(Sing)\t\ti\n"
	"Result\t\ta/ai\tPlace\t\tu\tPau\t\tee\n"
	"Tool\t\to/oi\tThing\t\t-(e)\tPl\t\tai\n"
	"DO/IO\t\ti/ou\tIdea\t\tai\n"
	"\t\t\tTime\t\toi\n\n"
	"Suffixes:\n"
	"Compar\t\tsil\tSuperl\t\tsithel\n"
	"Past\t\tlir\tFuture\t\tloira\n"
	"Dimin\t\tsikre\n\n"
	"Prefixes:\n"
	"Negate meaning\t\tshi\n"
	"Prepositions\n\n"
	"Notes:\n"
	"- Do not use number endings for numbers\n"
	"- Use the thing vowel near ' or with double letters\n"
	"- Use the singular ending after double consonants\n"
	"- The negation preposition goes after prepositions",

	L"Tense - C - Mood - C - Voice - C\n"
	"Tense\t\t\tMood\t\t\tVoice\n"
	"Past\t\ta\tInd\t\t-(e)\t(Act)\t\ta\n"
	"Present\t\te\tImp\t\tai\tPass\t\to\n"
	"Future\t\toi\tIntro\t\tu\tRef\t\tai\n"
	"\t\t\tRelative\t\ta\n"
	"\t\t\tPurpose\t\toui\n"
	"\t\t\tResult\t\ti\n"
	"\t\t\tIndirect\t\to\n\n"
	"Suffixes:\n"
	"Imp\t\te\t+time\t\toi\t-time\t\ta\n\n"
	"Prefixes:\n"
	"Negate meaning\t\tsh\n"
	"Prepositions\n\n"
	"Notes:\n"
	"- Only use the active ending for Ind. verbs or when the last consonant is '\n"
	"- The Intro mood is for since or if clauses, etc.\n"
	"- Use the Ind. vowel near ' or with double letters",

	L"Order:\n"
	"- (Introductory words) (Verb adv) (Noun adj adv) (DO)\n"
	"- (Subordinate clauses). IOs go anywhere after verb\n"
	"Punctuation:\n"
	"- Fi Tobair only has periods, punctuation marks, and quotes\n"
	"Prepositions:\n"
	"- Fi takes prepositions after it (e.g. fividu)\n"
	"- To adverbify prepositions, lengthen the last vowel.\n"
	"- If there is no last vowel, add an s after the preposition."
};
std::vector<std::wstring> grammarHeadings = { L"Nouns", L"Verbs", L"General notes"};

void GrammarView::extraDraw(ID2D1HwndRenderTarget* renderTarget) const {
	renderTarget->FillRectangle({ SUB_MENU_LR_BUFFER, SUB_MENU_TB_BUFFER, screenWidth - SUB_MENU_LR_BUFFER, screenHeight - SUB_MENU_TB_BUFFER }, backBrush);
	FLOAT ypos = SUB_MENU_TB_BUFFER + TEXT_BUFFER;

	renderTarget->DrawText(grammarHeadings[textIndex].c_str(), grammarHeadings[textIndex].size(), headerFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER,
		screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 20 }, tobairBrush);
	renderTarget->DrawText(grammarText[textIndex].c_str(), grammarText[textIndex].size(), bodyFormat, { SUB_MENU_LR_BUFFER + TEXT_BUFFER, SUB_MENU_TB_BUFFER + TEXT_BUFFER + 30,
		screenWidth - SUB_MENU_LR_BUFFER - TEXT_BUFFER, screenHeight - SUB_MENU_TB_BUFFER - TEXT_BUFFER + 20 }, englishBrush);
}

bool GrammarView::extraCreateDeviceIndependentResources() {
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 24, L"", &headerFormat);
	writeFactory->CreateTextFormat(L"Consolas", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 14, L"", &bodyFormat);
	return true;
}

bool GrammarView::extraCreateDeviceDependentResources(ID2D1HwndRenderTarget* renderTarget) {
	renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush);
	return true;
}

void GrammarView::extraDiscardDeviceDependentResources() {
	SafeRelease(&whiteBrush);

}

void GrammarView::handleScroll(int scrollTimes) {
	textIndex -= scrollTimes;
	textIndex = max(textIndex, 0);

	textIndex = min(textIndex, grammarText.size() - 1);
}