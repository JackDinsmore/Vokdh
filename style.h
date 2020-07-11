#pragma once
#include <string>
#include <map>

#define STYLE_PATH "style.txt"

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

class Style {
public:
	static Style* summon() {
		if (!instance) {
			instance = new Style();
		}
		return instance;
	}
	StyleNode operator[](std::string key);
	void save();

private:
	Style();

private:
	inline static Style* instance = nullptr;
	std::map<std::string, StyleNode> map;
};