#pragma once
#pragma warning(disable:4244)
#include <map>
#include <array>
#include "constants.h"
#include "message.h"

#define NUM_CONSONANTS 21
#define NUM_VOWELS 9

typedef std::pair<std::string, std::string> WordPair;

inline extern const std::array<std::string, NUM_CONSONANTS> consonants = { "b", "d", "f", "g", "j", "k", "l", "m", "n", "p", "r", "s", "t", "v", "w", "z", "ch", "sh", "th", "dh", "'" };
inline extern const std::array<std::string, NUM_VOWELS> vowels = {"a", "ai", "e", "ee", "i", "o", "oi", "u", "oui"};


enum class PART_OF_SPEECH {
	NONE,
	NOUN,
	VERB,
	ADJ,
	PROPER,
	PRONOUN,
	SHORT,
	ADVERB,
};

struct Grammar {
	PART_OF_SPEECH pos;
	std::string rootTobair;
	std::string rootEnglish;
	std::string prepositionTobair;
	std::string prepositionEnglish;
	std::vector<std::string> prefixesTobair;
	std::vector<std::string> prefixesEnglish;
	std::vector<std::string> suffixesTobair;
	std::vector<std::string> suffixesEnglish;
	std::string info[3];
};

class Dictionary : private Poster {
public:
	Dictionary& summon() {
		static Dictionary instance;
		return instance;
	}

	void addWord(WordPair word);// Must be tobair, english
	void addPreposition(WordPair word);// Must be tobair, english
	std::string findEnglish(std::string tobair) const;
	std::string findTobair(std::string english) const;
	void deleteEnglish(std::string english);
	void save() const;
	std::string findEnglishPreposition(std::string tobair) const;
	int searchEnglish(std::string english, WordPair* pairs, int numResults) const;
	int searchTobair(std::string tobair, WordPair* pairs, int numResults) const;

	Grammar translate(std::string tobair) const;

public:
	std::vector<std::string> pronouns;// Tobair
	std::vector<std::string> questionWords;// Tobair
	std::vector<std::string> shortWords;// Tobair

	std::map<std::string, std::string> englishTobairMapPrepositions;
	std::map<std::string, std::string> tobairEnglishMapPrepositions; 
	std::map<std::string, std::string> tobairEnglishMapShortenedPrepositions;

private:
	int getTobairScore(std::array<std::string, 3> a, std::array<std::string, 3> b) const;
	int getEnglishScore(std::string a, std::string b) const;
	std::array<std::string, 3> getConsonants(std::string t) const;

	std::string stripFullPrep(std::string tobair, Grammar* g) const;
	std::string stripShortPrep(std::string tobair, Grammar* g) const;
	std::string stripNoun(std::string tobair, Grammar* g) const;
	std::string stripVerb(std::string tobair, Grammar* g) const;
	std::string takeOffConsonant(std::string* tobair) const;
	std::string takeOffVowel(std::string* tobair) const;

	bool translateShort(std::string tobair, Grammar& g) const;
	bool translateVerb(std::string tobair, Grammar& g) const;
	bool translateNounAdj(std::string tobair, Grammar& g) const;
	bool translatePlain(std::string tobair, Grammar& g) const;
	bool translateBe(std::string tobair, Grammar& g) const;

private:
	Dictionary();

	int shortestPrep = 3;
	int longestPrep = 0;

	std::map<std::string, std::string> englishTobairMap;
	std::map<std::string, std::string> tobairEnglishMap;
	std::map<std::string, std::array<std::string, 3>> englishTobairMapConsonants;
	std::map<std::array<std::string, 3>, std::string> tobairEnglishMapConsonants;
	std::map<std::string, std::string> adverbialPrepositionTobairToEnglishMap;

	const std::filesystem::path dictPath = exePath / "dictionary.txt";
};