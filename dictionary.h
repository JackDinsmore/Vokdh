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

public:
	std::vector<std::string> pronouns;// Tobair
	std::vector<std::string> questionWords;// Tobair
	std::vector<std::string> shortWords;// Tobair

	std::map<std::string, std::string> englishTobairMapPrepositions;
	std::map<std::string, std::string> tobairEnglishMapPrepositions;

private:
	int getTobairScore(std::array<std::string, 3> a, std::array<std::string, 3> b) const;
	int getEnglishScore(std::string a, std::string b) const;
	std::array<std::string, 3> getConsonants(std::string t) const;

private:
	Dictionary();

	int maxPrepLength = 0;

	std::map<std::string, std::string> englishTobairMap;
	std::map<std::string, std::string> tobairEnglishMap;
	std::map<std::string, std::array<std::string, 3>> englishTobairMapConsonants;
	std::map<std::array<std::string, 3>, std::string> tobairEnglishMapConsonants;

	const std::filesystem::path dictPath = exePath / "dictionary.txt";
};