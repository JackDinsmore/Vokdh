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
	maxPrepLength = max(maxPrepLength, word.first.size());
	tobairEnglishMapPrepositions.insert(word);
	englishTobairMapPrepositions.insert({ word.second, word.first });
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