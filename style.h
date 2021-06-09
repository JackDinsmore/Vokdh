#pragma once
#include <string>
#include <map>
#include <d2d1.h>

#include "message.h"
#include "constants.h"

#define STYLE_FILE "style.txt"

class StyleNode : Poster {
    friend class StyleMap;
    friend class MapWrapper;
public:
    StyleNode() : text("") {}
    StyleNode(std::string text) : text(text) {}

public:
    explicit operator std::string() const {
        std::string result =  text.substr(1, text.size() - 2);
        return result;
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

protected:
    std::string text;

    void set(std::string s) {
        text = "\"" + s + "\"";
    }
    void set(int i) {
        text = std::to_string(i);
    }
    void set(float f) {
        text = std::to_string(f);
    }
    void set(double d) {
        text = std::to_string(d);
    }
    void set(D2D1::ColorF c) {
        text = "{" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " + std::to_string(c.b) + "}";
    }
};

class MapWrapper : Poster {
    friend class StyleMap;
public:
    MapWrapper() : initialized(false) {}
    StyleNode operator[](std::string key) const;

protected:
    MapWrapper(std::map<std::string, StyleNode>&& map) : map(map), initialized(true) {}
    void setAsIs(std::string key, std::string value) {
        map[key].text = value;
        initialized = true;
    }
    template <class T> void set(std::string key, T value) { 
        map[key].set(value);
        initialized = true;
    }

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
    template <class T> void set(std::string wrapper, std::string key, T value) {
        MapWrapper newWrapper;
        newWrapper.set(key, value);
        map[wrapper] = newWrapper;
    }

private:
    StyleMap();

private:
    std::map<std::string, MapWrapper> map;
};