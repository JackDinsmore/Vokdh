#include "text.h"
#include <fstream>
#include <stack>



TextCounter TextCounter::operator+(int amount) const {
	TextNode* returnNode = node;
	int linesLeft = node->text.size() - line - 1;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next node.
		returnNode = getNextNode(returnNode);
		if (!returnNode) {
			throw std::exception("Index out of bounds.");
		}
		linesLeft = returnNode->text.size();
	}
	return TextCounter(returnNode, amount);
}
void TextCounter::operator+=(int amount) {
	int linesLeft = node->text.size() - line - 1;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next node.
		node = getNextNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		linesLeft = node->text.size();
	}
	line = amount;
}
TextCounter& TextCounter::operator++() {
	if (line + 1 < node->text.size()) {
		line++;
	}
	else {
		node = getNextNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		line = 0;
	}
	return *this;
}
TextCounter TextCounter::operator++(int) {
	if (line + 1 < node->text.size()) {
		line++;
	}
	else {
		node = getNextNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		line = 0;
	}
	return *this;
}

TextCounter TextCounter::operator-(int amount) const {
	TextNode* returnNode = node;
	int linesLeft = line;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next node.
		returnNode = getLastNode(returnNode);
		if (!returnNode) {
			throw std::exception("Index out of bounds.");
		}
		linesLeft = returnNode->text.size();
	}
	return TextCounter(returnNode, returnNode->text.size() - amount);
}
void TextCounter::operator-=(int amount) {
	int linesLeft = line;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next node.
		node = getLastNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		linesLeft = node->text.size();
	}
	line = node->text.size() - amount - 1;
}
TextCounter& TextCounter::operator--() {
	if (line > 0) {
		line--;
	}
	else {
		node = getLastNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		line = node->text.size() - 1;
	}
	return *this;
}
TextCounter TextCounter::operator--(int) {
	if (line > 0) {
		line--;
	}
	else {
		node = getLastNode(node);
		if (!node) {
			throw std::exception("Index out of bounds.");
		}
		line = node->text.size() - 1;
	}
	return *this;
}

TextNode* TextCounter::getNextNode(TextNode* node) {
	if (!node) { return nullptr; }
	if (!node->children.empty()) {
		return node->children[0];
	}

	if (!node->parent) { return nullptr; }
	TextNode* now = node->parent;
	TextNode* last = node;
	// Search upwards until you reach a right branch
	while (std::find(now->children.begin(), now->children.end(), last) == now->children.end() - 1) {
		// While the last node is always the last of now's children:
		if (now->parent == nullptr) {
			return nullptr;
		}
		last = now;
		now = now->parent;
	}

	// Select the next branch
	now = *(std::find(now->children.begin(), now->children.end(), last) + 1);

	// Search downwards, taking all the left branches.
	while (!now->children.empty()) {
		now = now->children[0];
	}
	return now;
}

TextNode* TextCounter::getLastNode(TextNode* node) {
	if (!node) { return nullptr; }
	if (!node->parent) { return nullptr; }
	TextNode* parent = node->parent;
	if(std::find(parent->children.begin(), parent->children.end(), node) == parent->children.begin()) {
		// I am the leftmost node
		return parent;
	}

	// Take the left path, then take all right paths;
	TextNode* now = *(std::find(parent->children.begin(), parent->children.end(), node) - 1);
	while (!now->children.empty()) {
		now = now->children.back();
	}
	return now;
}

TextCounter TextTree::first() const {
	if (!root) {
		return TextCounter(nullptr, 0);
	}
	TextNode* node = TextCounter::getNextNode(root);
	return TextCounter(node, 0);
}

TextCounter TextTree::last() const {
	if (!root) {
		return TextCounter(nullptr, 0);
	}
	TextNode* right = root;
	while (!right->children.empty()) {
		right = right->children.back();
	}
	return TextCounter(right, right->text.size() - 1);
}



TextTree::TextTree(TextNode* root) : root(root) {
	
}

TextCounter TextTree::operator[](int line) const {
	if (line == 0) { return first(); }
	return first() + line;
}

TextTree::TextTree(TextTree&& t) {
	root = std::move(t.root);
	t.root = nullptr;
}

TextTree& TextTree::operator=(TextTree&& t) {
	root = std::move(t.root);
	t.root = nullptr;
	return *this;
}

void TextTree::deleteNode(TextNode* n) {\
	if (!n) { return; }
	for (TextNode* child : n->children) {
		deleteNode(child);
	}
	delete n;
}

bool FileLoader::loadFile(const std::filesystem::path filePath) {
	std::ifstream sourceFile;
	sourceFile.open(filePath);

	TextNode* root = nullptr;
	TextNode* currentNode = nullptr;
	std::stack<std::string> commands;
	std::stack<std::vector<std::string>> lines;

	if (sourceFile.is_open()) {
		std::string line;
		while (std::getline(sourceFile, line)) {
			if (line[0] == '#') {
				// Start command
				std::string command = line.substr(1);

				// Process command
				if (command == "h1" || command == "h2" || command == "h3" || command == "p" || command == "document") {
					TextNode* node = new TextNode();
					node->type = command;
					node->parent = currentNode;
					if (!currentNode) {// No parent exists
						root = node;
					}
					else {
						currentNode->children.push_back(node);
					}
					currentNode = node;
				}
				else {
					postMessage(MESSAGE_TYPE::M_ERROR, "Command " + command + " was not recognized.");
					return false;
				}
				if (commands.empty() && command != "document") {
					postMessage(MESSAGE_TYPE::M_ERROR, "The document must start with the #document command.");
					return false;
				}

				commands.push(command);
				lines.push({});
			}
			else if (line[0] == '$') {
				// End command
				if (commands.empty()) {
					postMessage(MESSAGE_TYPE::M_ERROR, "Encountered too many '$'.");
					return false;
				}
				std::string command = commands.top();
				if (command == "document") {
					textTree = std::move(TextTree(root));
				}
				else if (lines.top().empty()) {
					postMessage(MESSAGE_TYPE::M_ERROR, "All headings must be filled except #document.");
					return false;
				}
				currentNode->text = lines.top();
				currentNode = currentNode->parent;
				commands.pop();
				lines.pop();
			}
			else {
				if (line.empty()) {
					continue;
				}
				if (line[0] == '\\') {
					if (line[1] == '#') {
						line = '#' + line.substr(2);
					}
					if (line[1] == '$') {
						line = '$' + line.substr(2);
					}
				}
				if (lines.empty()) {
					postMessage(MESSAGE_TYPE::M_ERROR, "Line found without heading.");
					return false;
				}
				if (commands.top() == "document") {
					postMessage(MESSAGE_TYPE::M_ERROR, "Line found without heading.");
					return false;
				}
				lines.top().push_back(line);
			}
		}
	}

	else {
		postMessage(MESSAGE_TYPE::M_ERROR, "File " + filePath.filename().generic_string() + " could not be found.");
		return false;
	}

	if (!commands.empty()) {
		postMessage(MESSAGE_TYPE::M_ERROR, "Unclosed headers exist.");
		return false;
	}
	return true;
}