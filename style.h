#pragma once
#include <string>
#include <map>

#include "message.h"
#include "constants.h"

#define STYLE_FILE "style.txt"

class StyleNode {
public:
	StyleNode() : text("") {}
	StyleNode(std::string text) : text(text) {}

public:
	explicit operator std::string() const {
		return text;
	}
	explicit operator std::wstring() const {
		return std::wstring(text.begin(), text.end());
	}
	explicit operator int() const {
		return std::stoi(text);
	}
	explicit operator float() const {
		return std::stof(text);
	}
	explicit operator double() const {
		return std::stod(text);
	}

private:
	std::string text;
};

class StyleMap : Poster {
public:
	static StyleMap& summon() {
		static StyleMap instance;
		return instance;
	}
	StyleNode operator[](std::string key);
	void save();

private:
	StyleMap();

private:
	std::map<std::string, StyleNode> map;
};