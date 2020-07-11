#include "style.h"
#include <fstream>


Style::Style() {
	std::ifstream styleFile;
	std::string line;
	styleFile.open(STYLE_PATH);

	if (styleFile.is_open()) {
		while (getline(styleFile, line)) {
			// "key": value
		}
	}
}

StyleNode Style::operator[](std::string key) {
	return map[key];
}