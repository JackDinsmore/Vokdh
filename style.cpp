#include "style.h"
#include <fstream>

#pragma warning(disable:4244)


StyleNode::operator D2D1::ColorF() const {
	int begin = std::find(text.begin(), text.end(), '{') - text.begin();
	int end = std::find(text.begin(), text.end(), '}') - text.begin();
	if (begin >= end || begin == text.size() || end == text.size()) {
		postMessage(MESSAGE_TYPE::M_ERROR, "Color string improperly formatted.");
		return D2D1::ColorF({ 0, 0, 0 });
	}
	int comma1 = std::find(text.begin() + begin, text.end(), ',') - text.begin();
	int comma2 = std::find(text.begin() + comma1+1, text.end(), ',') - text.begin();
	std::string r = text.substr(begin + 1, comma1 - begin - 1);
	std::string g = text.substr(comma1 + 1, comma2 - comma1 - 1);
	std::string b = text.substr(comma2 + 1, end - comma2 - 1);
	float R, G, B;
	try {
		R = std::stof(r);
		G = std::stof(g);
		B = std::stof(b);
	}
	catch (...) {
		postMessage(MESSAGE_TYPE::M_ERROR, "Color string improperly formatted.");
		return D2D1::ColorF({ 0, 0, 0 });
	}
	return D2D1::ColorF({ R, G , B });
}


StyleMap::StyleMap() {
	std::ifstream styleFile;
	std::string line;
	styleFile.open(exePath.parent_path().parent_path() / STYLE_FILE);
	std::string header = "";

	if (styleFile.is_open()) {
		while (getline(styleFile, line)) {
			// key: value
			if (line == "") { continue; }
			unsigned colonPos = std::find(line.begin(), line.end(), ':') - line.begin();
			unsigned startKey;
			for (startKey = 0; line[startKey] == ' ' || line[startKey] == '\t'; startKey++);

			std::string key;
			if (colonPos == line.size()-1) {
				key = line.substr(startKey, colonPos);
			}
			else {
				key = line.substr(startKey, colonPos - 1);
			}
			unsigned startValue = colonPos + 1;
			while (startValue < line.size() && (line[startValue] == ' ' || line[startValue] == '\t' || line[startValue] == '\n')) {
				startValue++;
			}
			std::string value = line.substr(startValue);
			if (value == "") {
				header = key;
			}
			else {
				map[header].setAsIs(key, value);
			}
		}
	}
	else {
		// Style file not found.
		postMessage(MESSAGE_TYPE::M_ERROR, "Style file not found at " + (exePath / STYLE_FILE).generic_string());
	}
}

MapWrapper StyleMap::operator[](std::string key) const {
	if (map.find(key) != map.end()) {
		return map.at(key);
	}
	else {
		postMessage(MESSAGE_TYPE::M_WARNING, "Header \"" + key + "\" was not found in the style map.");
		return MapWrapper();
	}
}

void StyleMap::save() {
	std::ofstream styleFile;
	styleFile.open(exePath.parent_path().parent_path() / STYLE_FILE);

	if (styleFile.is_open()) {
		for (auto wrapper : map) {
			styleFile << wrapper.first + ":\n";
			for (auto node : wrapper.second.map) {
				styleFile << "\t" + node.first + ": " + node.second.text + "\n";
			}
		}
	}
	else {
		postMessage(MESSAGE_TYPE::M_ERROR, "Style file not found at " + (exePath / STYLE_FILE).generic_string());
	}
}



StyleNode MapWrapper::operator[](std::string key) const {
	if (map.find(key) != map.end()) {
		return map.at(key);
	}
	else if (initialized){
		postMessage(MESSAGE_TYPE::M_WARNING, "Key \"" + key + "\" was not found in the style map.");
		return StyleNode();
	}
	return StyleNode();
}