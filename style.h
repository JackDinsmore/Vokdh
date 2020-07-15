#pragma once
#include <string>
#include <map>
#include <d2d1.h>

#include "message.h"
#include "constants.h"

#define STYLE_FILE "style.txt"

class StyleNode : Poster {
public:
	StyleNode() : text("") {}
	StyleNode(std::string text) : text(text) {}

public:
	explicit operator std::string() const {
		return text;
	}
	explicit operator std::wstring() const {
		try {
			return std::wstring(text.begin(), text.end());
		}
		catch (...) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text " + text + " could not be converted to wstring.");
			return L"";
		}
	}
	explicit operator int() const {
		try {
			return std::stoi(text);
		}
		catch (...) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text " + text + " could not be converted to int.");
			return 0;
		}
	}
	explicit operator float() const {
		try {
			return std::stof(text);
		}
		catch (...) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text " + text + " could not be converted to int.");
			return 0;
		}
	}
	explicit operator double() const {
		try {
			return std::stod(text);
		}
		catch (...) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text " + text + " could not be converted to int.");
			return 0;
		}
	}
	explicit operator D2D1::ColorF() const;

private:
	std::string text;
};

class MapWrapper : Poster {
	friend class StyleMap;
public:
	MapWrapper() : initialized(false) {}
	StyleNode operator[](std::string key) const;

protected:
	MapWrapper(std::map<std::string, StyleNode>&& map) : map(map), initialized(true) {}
	void set(std::string key, std::string value) { map[key] = value; initialized = true; }

private:
	bool initialized;
	std::map<std::string, StyleNode> map;
};

class StyleMap : Poster {
public:
	static StyleMap& summon() {
		static StyleMap instance;
		return instance;
	}
	MapWrapper operator[](std::string key) const;
	void save();

private:
	StyleMap();

private:
	std::map<std::string, MapWrapper> map;
};