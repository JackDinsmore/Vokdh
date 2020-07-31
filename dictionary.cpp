#pragma warning(disable:4804)
#include "dictionary.h"
#include <fstream>

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))


enum class PARSE_MODE {
	NONE,
	PREPOSITIONS,
	PRONOUNS,
	QUESTION_WORDS,
	SHORT_WORDS,
	WORDS,
};

Dictionary::Dictionary() {
	std::ifstream file;
	file.open(dictPath);
	std::string line;
	PARSE_MODE mode;

	if (file.is_open()) {
		while (std::getline(file, line)) {
			if (line[0] == '#') {
				if (line.substr(1) == "prepositions") {
					mode = PARSE_MODE::PREPOSITIONS;
				}
				else if (line.substr(1) == "pronouns") {
					mode = PARSE_MODE::PRONOUNS;
				}
				else if (line.substr(1) == "questionWords") {
					mode = PARSE_MODE::QUESTION_WORDS;
				}
				else if (line.substr(1) == "shortWords") {
					mode = PARSE_MODE::SHORT_WORDS;
				}
				else if (line.substr(1) == "words") {
					mode = PARSE_MODE::WORDS;
				}
				continue;
			}
			int tabPos = std::find(line.begin(), line.end(), '\t') - line.begin();
			std::string tobair = line.substr(0, tabPos);
			std::string english = line.substr(tabPos+1);
			switch (mode) {
			case PARSE_MODE::PREPOSITIONS:
				addPreposition({ tobair, english });
				break;
			case PARSE_MODE::PRONOUNS:
				pronouns.push_back(tobair);
				addWord({ tobair, english });
				break;
			case PARSE_MODE::QUESTION_WORDS:
				questionWords.push_back(tobair);
				addWord({ tobair, english });
				break;
			case PARSE_MODE::SHORT_WORDS:
				shortWords.push_back(tobair);
				addWord({ tobair, english });
				break;
			case PARSE_MODE::WORDS:
				addWord({ tobair, english });
				break;
			}
		}
		file.close();
	}
	else {
		postMessage(MESSAGE_TYPE::M_TERMINATE, "Could not find dictionary file.");
	}

	std::sort(pronouns.begin(), pronouns.end());
	std::sort(questionWords.begin(), questionWords.end());
	std::sort(shortWords.begin(), shortWords.end());
}

void Dictionary::addWord(WordPair word) {
	// Must be tobair, english
	auto c = getConsonants(word.first);
	if (c[0]!= "") {
		tobairEnglishMapConsonants.insert({ c, word.second});
		englishTobairMapConsonants.insert({ word.second, c });
	}
	tobairEnglishMap.insert(word);
	englishTobairMap.insert({ word.second, word.first });
}

void Dictionary::addPreposition(WordPair word) {
	// Must be tobair, english
	shortestPrep = min(shortestPrep, word.first.size());
	longestPrep = max(longestPrep, word.first.size());
	tobairEnglishMapPrepositions.insert(word);
	englishTobairMapPrepositions.insert({ word.second, word.first });

	std::string adv = word.first;
	if (adv.substr(adv.size() - 1) == "a") {
		adv += "i";
		tobairEnglishMapShortenedPrepositions.insert({ word.first.substr(0, word.first.size() - 1), word.second });
	}
	else if (adv.substr(adv.size() - 1) == "e") {
		adv += "e";
		tobairEnglishMapShortenedPrepositions.insert({ word.first.substr(0, word.first.size() - 1), word.second });
	}
	else if (adv.substr(adv.size() - 1) == "o") {
		adv += "i";
		tobairEnglishMapShortenedPrepositions.insert({ word.first.substr(0, word.first.size() - 1), word.second });
	}
	else if (adv.substr(adv.size() - 1) == "u") {
		adv = adv.substr(0, adv.size() - 1) + "oui";
		tobairEnglishMapShortenedPrepositions.insert({ word.first.substr(0, word.first.size() - 1), word.second });
	}
	else {
		adv += "s";
		tobairEnglishMapShortenedPrepositions.insert({ word.first, word.second });
	}
	adverbialPrepositionTobairToEnglishMap.insert({ adv, word.second });
}

std::string Dictionary::findEnglish(std::string tobair) const {
	auto p = tobairEnglishMap.find(tobair);
	if (p == tobairEnglishMap.end()) {
		return "";
	}
	return p->second;
}

std::string Dictionary::findEnglishPreposition(std::string tobair) const {
	auto p = tobairEnglishMapPrepositions.find(tobair);
	if (p == tobairEnglishMapPrepositions.end()) {
		return "";
	}
	return p->second;
}

std::string Dictionary::findTobair(std::string english) const {
	auto p = englishTobairMap.find(english);
	if (p == englishTobairMap.end()) {
		return "";
	}
	return p->second;
}

void Dictionary::deleteEnglish(std::string english) {
	std::string tobair = englishTobairMap.find(english)->second;
	englishTobairMap.erase(english);
	tobairEnglishMap.erase(tobair);

	auto p = englishTobairMapConsonants.find(english);
	if (p != englishTobairMapConsonants.end()) {
		auto c = p->second;
		englishTobairMapConsonants.erase(english);
		tobairEnglishMapConsonants.erase(c);
	}
}

int Dictionary::searchEnglish(std::string english, WordPair* pairs, int numResults) const {
	int* scores = new int[numResults];
	int numWordsScored = 0;

	for (auto p = englishTobairMap.begin(); p != englishTobairMap.end(); p++) {
		int score = getEnglishScore(p->first, english);
		bool addedScore = false;
		for (int i = 0; i < min(numWordsScored, numResults - 1); i++) {
			if (score > scores[i]) {
				// Add the word
				for (int j = min(numWordsScored, numResults-1); j > i; j--) {
					// Move j-1 to here
					scores[j] = scores[j - 1];
					pairs[j] = pairs[j - 1];
				}
				scores[i] = score;
				pairs[i] = *p;
				addedScore = true;
				numWordsScored++;
				break;
			}
		}
		if (!addedScore && numWordsScored < numResults) {
			scores[numWordsScored] = score;
			pairs[numWordsScored] = *p;
			numWordsScored++;
		}
	}

	delete[] scores;
	return min(numWordsScored, numResults);
}

int Dictionary::searchTobair(std::string tobair, WordPair* pairs, int numResults) const {
	auto c = getConsonants(tobair);
	if (c[0] == "") {
		int* scores = new int[numResults];
		int numWordsScored = 0;

		for (auto p = tobairEnglishMap.begin(); p != tobairEnglishMap.end(); p++) {
			int score = getEnglishScore(p->first, tobair);
			bool addedScore = false;
			for (int i = 0; i < min(numWordsScored, numResults - 1); i++) {
				if (score > scores[i]) {
					// Add the word
					for (int j = min(numWordsScored, numResults - 1); j > i; j--) {
						// Move j-1 to here
						scores[j] = scores[j - 1];
						pairs[j] = pairs[j - 1];
					}
					scores[i] = score;
					pairs[i] = *p;
					addedScore = true;
					numWordsScored++;
					break;
				}
			}
			if (!addedScore && numWordsScored < numResults) {
				scores[numWordsScored] = score;
				pairs[numWordsScored] = *p;
				numWordsScored++;
			}
		}

		delete[] scores;
		return min(numWordsScored, numResults);
	}
	else {
		int* scores = new int[numResults];
		int numWordsScored = 0;

		for (auto p = tobairEnglishMapConsonants.begin(); p != tobairEnglishMapConsonants.end(); p++) {
			int score = getTobairScore(p->first, c);
			bool addedScore = false;
			for (int i = 0; i < min(numWordsScored, numResults - 1); i++) {
				if (score > scores[i]) {
					// Add the word
					for (int j = min(numWordsScored, numResults - 1); j > i; j--) {
						// Move j-1 to here
						scores[j] = scores[j - 1];
						pairs[j] = pairs[j - 1];
					}
					scores[i] = score;
					pairs[i] = {p->first[0] + " " + p->first[1] + " " + p->first[2], p->second };
					addedScore = true;
					numWordsScored++;
					break;
				}
			}
			if (!addedScore && numWordsScored < numResults) {
				scores[numWordsScored] = score;
				pairs[numWordsScored] = { p->first[0] + " " + p->first[1] + " " + p->first[2], p->second };
				numWordsScored++;
			}
		}

		delete[] scores;
		return min(numWordsScored, numResults);
	}
}

std::array<std::string, 3> Dictionary::getConsonants(std::string t) const {
	auto firstSpacePos = std::find(t.begin(), t.end(), ' ');
	if (firstSpacePos != t.end()) {
		auto secondSpacePos = std::find(firstSpacePos + 1, t.end(), ' ');
		if (secondSpacePos != t.end()) {
			auto thirdSpacePos = std::find(secondSpacePos + 1, t.end(), ' ');
			if (thirdSpacePos == t.end()) {
				std::string c1 = t.substr(0, firstSpacePos - t.begin());
				std::string c2 = t.substr(firstSpacePos - t.begin() + 1, secondSpacePos - firstSpacePos - 1);
				std::string c3 = t.substr(secondSpacePos - t.begin() + 1);
				if (std::find(consonants.begin(), consonants.end(), c1) != consonants.end() &&
					std::find(consonants.begin(), consonants.end(), c2) != consonants.end() &&
					std::find(consonants.begin(), consonants.end(), c3) != consonants.end()
					) {
					return { c1, c2, c3 };
				}
			}
		}
	}
	return{ "", "", "" };
}

int Dictionary::getEnglishScore(std::string a, std::string b) const {
	// Leftshift one word and count up letter matchups.
	std::string x = a.size() > b.size() ? a : b;
	std::string n = a.size() > b.size() ? b : a;

	int score = 0;
	for (int i = 0; i <= x.size() - n.size(); i++) {
		// Leftshifts
		int thisScore = 0;
		for (int j = 0; j < n.size(); j++) {
			if (n[j] == x[j + i]) { thisScore += 2; }
			else if (j + i + 1 < x.size() && n[j] == x[j + i + 1]) { thisScore++; }
			else if (j + i - 1 >= 0 && n[j] == x[j + i - 1]) { thisScore++; }
			else { thisScore--; }
			if (n[j] == x[j + i] || (j + i + 1 < x.size() && n[j] == x[j + i + 1]) || (j + i - 1 >= 0 && n[j] == x[j + i - 1])) {
				if (i == 0) {
					score += 3;
				}
			}
		}
		score = max(score, thisScore);
	}
	return score;
}

int Dictionary::getTobairScore(std::array<std::string, 3> a, std::array<std::string, 3> b) const {
	int score = 0;
	if (a[0] == b[0]) { score += 2; }
	if (a[1] == b[1]) { score += 2; }
	if (a[2] == b[2]) { score += 2; }
	if (a[1] == b[0]) { score++; }
	if (a[2] == b[0]) { score++; }
	if (a[0] == b[1]) { score++; }
	if (a[2] == b[1]) { score++; }
	if (a[0] == b[2]) { score++; }
	if (a[1] == b[2]) { score++; }
	return score;
}

void Dictionary::save() const {
	std::ofstream file;
	file.open(dictPath);

	if (file.is_open()) {
		// prepositions:
		file << "#prepositions" << std::endl;
		for (auto p = tobairEnglishMapPrepositions.begin(); p != tobairEnglishMapPrepositions.end(); p++) {
			file << p->first << '\t' << p->second << std::endl;
		}

		// pronouns
		file << "#pronouns" << std::endl;
		for (auto p = pronouns.begin(); p != pronouns.end(); p++) {
			std::string english = tobairEnglishMap.at(*p);
			file << *p << '\t' << english << std::endl;
		}

		// question words
		file << "#questionWords" << std::endl;
		for (auto p = questionWords.begin(); p != questionWords.end(); p++) {
			std::string english = tobairEnglishMap.at(*p);
			file << *p << '\t' << english << std::endl;
		}

		// short words
		file << "#shortWords" << std::endl;
		for (auto p = shortWords.begin(); p != shortWords.end(); p++) {
			std::string english = tobairEnglishMap.at(*p);
			file << *p << '\t' << english << std::endl;
		}

		// words
		file << "#words" << std::endl;
		for (auto p = tobairEnglishMap.begin(); p != tobairEnglishMap.end(); p++) {
			if (std::find(pronouns.begin(), pronouns.end(), p->first) == pronouns.end() &&
				std::find(questionWords.begin(), questionWords.end(), p->first) == questionWords.end() &&
				std::find(shortWords.begin(), shortWords.end(), p->first) == shortWords.end()) {
				file << p->first << '\t' << p->second << std::endl;
			}
		}
		file.close();
	}
	else {
		postMessage(MESSAGE_TYPE::M_TERMINATE, "Could not find dictionary file.");
	}
}

Grammar Dictionary::translate(std::string tobair) const {
	std::for_each(tobair.begin(), tobair.end(), [](char& c) {
		c = std::tolower(c);
	});

	Grammar noPro;
	if (translateShort(tobair, noPro)) { return noPro; }
	if (translateNounAdj(tobair, noPro)) { return noPro; }
	if (translateVerb(tobair, noPro)) { return noPro; }
	if (translatePlain(tobair, noPro)) { return noPro; }
	if (translateBe(tobair, noPro)) { return noPro; }

	Grammar pro;
	std::string withoutPrep = stripFullPrep(tobair, &pro);
	if (withoutPrep != tobair) {
		if (translateShort(withoutPrep, pro)) { return pro; }
		if (translateNounAdj(withoutPrep, pro)) { return pro; }
		if (translatePlain(withoutPrep, pro)) { return pro; }
		if (translateBe(withoutPrep, pro)) { return pro; }
	}
	withoutPrep = stripShortPrep(tobair, &pro);
	if (withoutPrep != tobair) {
		if (translateShort(withoutPrep, pro)) { return pro; }
		if (translateVerb(withoutPrep, pro)) { return pro; }
		if (translatePlain(withoutPrep, pro)) { return pro; }
	}
	// Error
	pro.pos = PART_OF_SPEECH::NONE;
	pro.rootTobair = tobair;
	pro.rootEnglish = "?";
	return pro;
}

bool Dictionary::translateShort(std::string tobair, Grammar& g) const {
	auto pIndex = std::find(pronouns.begin(), pronouns.end(), tobair);
	auto qIndex = std::find(questionWords.begin(), questionWords.end(), tobair);
	auto sIndex = std::find(shortWords.begin(), shortWords.end(), tobair);
	auto aIndex = adverbialPrepositionTobairToEnglishMap.find(tobair);
	if (pIndex != pronouns.end()) {
		g.pos = PART_OF_SPEECH::PRONOUN;
		g.rootTobair = tobair;
		g.rootEnglish = findEnglish(*pIndex);
		return true;
	}
	else if (qIndex != questionWords.end()) {
		g.pos = PART_OF_SPEECH::PRONOUN;
		g.rootTobair = tobair;
		g.rootEnglish = findEnglish(*qIndex);
		return true;
	}
	else if (sIndex != shortWords.end()) {
		g.pos = PART_OF_SPEECH::PRONOUN;
		g.rootTobair = tobair;
		g.rootEnglish = findEnglish(*sIndex);
		return true;
	}
	else if (aIndex != adverbialPrepositionTobairToEnglishMap.end()) {
		g.pos = PART_OF_SPEECH::ADVERB;
		g.rootTobair = aIndex->first;
		g.rootEnglish = aIndex->second;
		return true;
	}
	return false;
}

bool Dictionary::translateNounAdj(std::string tobair, Grammar& g) const {
	std::string nounStrip, verbStrip, consonant, vowel;
	nounStrip = stripNoun(tobair, &g);
	consonant = takeOffConsonant(&nounStrip);
	if (!consonant.empty()) {
		std::string wordConsonants[3];
		wordConsonants[0] = consonant;
		vowel = takeOffVowel(&nounStrip);
		if (vowel == "e" || vowel == "a" || vowel == "o" || vowel == "i") {
			g.pos = PART_OF_SPEECH::NOUN;
		}
		else if (vowel == "ee" || vowel == "ai" || vowel == "oi" || vowel == "oui") {
			g.pos = PART_OF_SPEECH::ADJ;
		}
		else {
			return false;
		}
		if (vowel == "e" || vowel == "ee") { g.info[0] = "agent"; }
		if (vowel == "a" || vowel == "ai") { g.info[0] = "result"; }
		if (vowel == "o" || vowel == "oi") { g.info[0] = "tool"; }
		if (vowel == "i" || vowel == "oui") { g.info[0] = "DO/IO"; }

		consonant = takeOffConsonant(&nounStrip);
		if (!consonant.empty()) {
			wordConsonants[1] = consonant;
			vowel = takeOffVowel(&nounStrip);

			if (vowel == "i") { g.info[1] = "people"; }
			else if (vowel == "u") { g.info[1] = "place"; }
			else if (vowel == "e" || vowel == "") { g.info[1] = "thing"; }
			else if (vowel == "ai") { g.info[1] = "idea"; }
			else if (vowel == "oi") { g.info[1] = "time"; }
			else {
				return false;
			}

			consonant = takeOffConsonant(&nounStrip);
			if (!consonant.empty()) {
				wordConsonants[2] = consonant;
				vowel = takeOffVowel(&nounStrip);
				if (vowel == "i" || vowel == " ") { g.info[2] = "sing"; }
				else if (vowel == "ee") { g.info[2] = "pau"; }
				else if (vowel == "ai" || vowel == "") { g.info[2] = "pl"; }
				else {
					return false;
				}

				g.rootTobair = wordConsonants[0] + " " + wordConsonants[1] + " " + wordConsonants[2];
				g.rootEnglish = findEnglish(g.rootTobair);
				if (g.rootEnglish == "") { g.rootEnglish = "?"; }

				if (!nounStrip.empty()) {
					// Suffixes
					bool stripped = true;
					while (stripped) {
						stripped = false;
						if (nounStrip.substr(0, 3) == "sil") {
							g.suffixesTobair.push_back("sil");
							g.suffixesEnglish.push_back("compar");
							nounStrip = nounStrip.substr(3);
							stripped = true;
						}
						if (nounStrip.substr(0, 3) == "lir") {
							g.suffixesTobair.push_back("lir");
							g.suffixesEnglish.push_back("past");
							nounStrip = nounStrip.substr(3);
							stripped = true;
						}
						if (nounStrip.substr(0, 6) == "sithel") {
							g.suffixesTobair.push_back("sithel");
							g.suffixesEnglish.push_back("superl");
							nounStrip = nounStrip.substr(6);
							stripped = true;
						}
						if (nounStrip.substr(0, 5) == "loira") {
							g.suffixesTobair.push_back("loira");
							g.suffixesEnglish.push_back("future");
							nounStrip = nounStrip.substr(5);
							stripped = true;
						}
						if (nounStrip.substr(0, 5) == "skire") {
							g.suffixesTobair.push_back("skire");
							g.suffixesEnglish.push_back("dimin");
							nounStrip = nounStrip.substr(5);
							stripped = true;
						}
					}
					if (!nounStrip.empty()) {
						return false;
					}
				}

				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	return false;
}

bool Dictionary::translateVerb(std::string tobair, Grammar& g) const {
	// Must be followed by no prep or a short one

	std::string verbStrip = stripVerb(tobair, &g);
	std::string vowel = takeOffVowel(&verbStrip);
	if (!vowel.empty()) {
		g.pos = PART_OF_SPEECH::VERB;
		std::string wordConsonants[3];

		std::string consonant = takeOffConsonant(&verbStrip);
		if (consonant.empty()) { return false; }
		wordConsonants[0] = consonant;

		if (vowel == "a") { g.info[0] = "past"; }
		else if (vowel == "e") { g.info[0] = "present"; }
		else if (vowel == "oi") { g.info[0] = "future"; }
		else {
			return false;
		}


		vowel = takeOffVowel(&verbStrip);
		consonant = takeOffConsonant(&verbStrip);
		if (!consonant.empty()) {
			wordConsonants[1] = consonant;
			if (vowel == "e" || vowel == "") { g.info[1] = "ind"; }
			else if (vowel == "ai") { g.info[1] = "imper"; }
			else if (vowel == "u") { g.info[1] = "intro"; }
			else if (vowel == "a") { g.info[1] = "rel"; }
			else if (vowel == "oui") { g.info[1] = "purp"; }
			else if (vowel == "i") { g.info[1] = "res"; }
			else if (vowel == "o") { g.info[1] = "indir"; }
			else {
				return false;
			}

			vowel = takeOffVowel(&verbStrip);
			consonant = takeOffConsonant(&verbStrip);
			if (!consonant.empty()) {
				wordConsonants[2] = consonant;
				if (vowel == "a" || vowel == "") { g.info[2] = "act"; }
				else if (vowel == "o") { g.info[2] = "pass"; }
				else if (vowel == "ai") { g.info[2] = "ref"; }
				else {
					return false;
				}

				g.rootTobair = wordConsonants[0] + " " + wordConsonants[1] + " " + wordConsonants[2];
				g.rootEnglish = findEnglish(g.rootTobair);
				if (g.rootEnglish == "") { g.rootEnglish = "?"; }

				if (!verbStrip.empty()) {
					// Suffixes
					if (verbStrip == "e") {
						g.suffixesTobair.push_back("e");
						g.suffixesTobair.push_back("imp");
					}
					else if (verbStrip == "oi") {
						g.suffixesTobair.push_back("oi");
						g.suffixesTobair.push_back("+ time");
					}
					else if (verbStrip == "a") {
						g.suffixesTobair.push_back("a");
						g.suffixesTobair.push_back("- time");
					}
					else {
						return false;
					}
				}

				return true;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	return false;
}

bool Dictionary::translatePlain(std::string tobair, Grammar& g) const {
	std::string eng = findEnglish(tobair);
	if (!eng.empty()) {
		g.pos = PART_OF_SPEECH::PROPER;
		g.rootTobair = tobair;
		g.rootEnglish = eng;
		g.info[0] = ""; g.info[1] = ""; g.info[2] = "";
		return true;
	}
	return false;
}

bool Dictionary::translateBe(std::string tobair, Grammar& g) const {
	std::string verbStrip = stripVerb(tobair, &g);
	std::string vowel = takeOffVowel(&verbStrip);
	if (!vowel.empty()) {
		g.pos = PART_OF_SPEECH::VERB;
		if (vowel == "a") { g.info[0] = "past"; }
		else if (vowel == "e") { g.info[0] = "present"; }
		else if (vowel == "oi") { g.info[0] = "future"; }
		else {
			return false;
		}
		if (takeOffConsonant(&verbStrip) == "f") {
			vowel = takeOffVowel(&verbStrip);
			if (vowel == "e" || vowel == "") { g.info[1] = "ind"; }
			else if (vowel == "ai") { g.info[1] = "imper"; }
			else if (vowel == "u") { g.info[1] = "intro"; }
			else if (vowel == "a") { g.info[1] = "rel"; }
			else if (vowel == "oui") { g.info[1] = "purp"; }
			else if (vowel == "i") { g.info[1] = "res"; }
			else if (vowel == "o") { g.info[1] = "indir"; }
			else {
				return false;
			}

			g.rootTobair = "f (b r)";
			g.rootEnglish = "be";

			if (!verbStrip.empty()) {
				// Suffixes
				if (verbStrip == "e") {
					g.suffixesTobair.push_back("e");
					g.suffixesTobair.push_back("imp");
				}
				else if (verbStrip == "oi") {
					g.suffixesTobair.push_back("oi");
					g.suffixesTobair.push_back("+ time");
				}
				else if (verbStrip == "a") {
					g.suffixesTobair.push_back("a");
					g.suffixesTobair.push_back("- time");
				}
				else {
					return false;
				}
			}

			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	return false;
}

std::string Dictionary::stripFullPrep(std::string tobair, Grammar* g) const {
	for (int i = shortestPrep; i <= longestPrep; i++) {
		auto p = tobairEnglishMapPrepositions.find(tobair.substr(0, i));
		if (p != tobairEnglishMapPrepositions.end()) {
			g->prepositionTobair = tobair.substr(0, i);
			g->prepositionEnglish = p->second;
			return tobair.substr(i);
		}
	}
	return tobair;
}

std::string Dictionary::stripShortPrep(std::string tobair, Grammar* g) const {
	for (int i = shortestPrep - 1; i <= longestPrep - 1; i++) {
		auto p = tobairEnglishMapShortenedPrepositions.find(tobair.substr(0, i));
		if (p != tobairEnglishMapShortenedPrepositions.end()) {
			g->prepositionTobair = tobair.substr(0, i);
			g->prepositionEnglish = p->second;
			return tobair.substr(i);
		}
	}
	return tobair;
}

std::string Dictionary::stripNoun(std::string tobair, Grammar* g) const {
	if (tobair.substr(0, 3) == "shi") {
		g->prefixesTobair.push_back("shi");
		g->prefixesEnglish.push_back("not");
		return tobair.substr(3);
	}
	return tobair;
}

std::string Dictionary::stripVerb(std::string tobair, Grammar* g) const {
	if (tobair.substr(0, 2) == "sh") {
		g->prefixesTobair.push_back("sh");
		g->prefixesEnglish.push_back("not");
		return tobair.substr(2);
	}
	return tobair;
}

std::string Dictionary::takeOffConsonant(std::string* tobair) const {
	auto p = std::find(consonants.begin(), consonants.end(), tobair->substr(0, 2));
	if (tobair->size() >= 2 && p != consonants.end()) {
		*tobair = tobair->substr(2);
		return *p;
	}
	p = std::find(consonants.begin(), consonants.end(), tobair->substr(0, 1));
	if (tobair->size() >= 1 && p != consonants.end()) {
		*tobair = tobair->substr(1);
		return *p;
	}
	return "";
}

std::string Dictionary::takeOffVowel(std::string* tobair) const {
	auto p = std::find(vowels.begin(), vowels.end(), tobair->substr(0, 3));
	if (tobair->size() >= 3 && p != vowels.end()) {
		*tobair = tobair->substr(3);
		return *p;
	}
	p = std::find(vowels.begin(), vowels.end(), tobair->substr(0, 2));
	if (tobair->size() >= 2 && p != vowels.end()) {
		*tobair = tobair->substr(2);
		return *p;
	}
	p = std::find(vowels.begin(), vowels.end(), tobair->substr(0, 1));
	if (tobair->size() >= 1 && p != vowels.end()) {
		*tobair = tobair->substr(1);
		return *p;
	}
	return "";
}