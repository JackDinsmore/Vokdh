#include "style.h"
#include <fstream>


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
	styleFile.open(exePath / STYLE_FILE);
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
			bool quoted = false;
			while (startValue < line.size() && (line[startValue] == ' ' || line[startValue] == '\t' || line[startValue] == '\n' || line[startValue] == '"')) {
				if (line[startValue] == '"') {
					startValue++;
					quoted = true;
					break;
				}
				startValue++;
			}
			std::string value = line.substr(startValue);
			if (quoted) {
				int i;
				for (i = value.size(); i >= 0; i--) {
					if (value[i] == '"') {
						break;
					}
				}
				if (value[i] != '"') {
					// EOL encountered while scanning for endquote
					postMessage(MESSAGE_TYPE::M_ERROR, "EOL encountered while scanning for endquote in the style file");
				}
				value = value.substr(0, i);
			}
			if (value == "") {
				header = key;
			}
			else {
				map[header].set(key, value);
			}
		}
	}
	else {
		// Style file not found.
		postMessage(MESSAGE_TYPE::M_WARNING, "Style file not found at " + (exePath / STYLE_FILE).generic_string());
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