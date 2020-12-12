#include "text.h"
#include <fstream>
#include <stack>



TextCounter TextCounter::operator+(int amount) const {
	TextNode* returnNode = n;
	int linesLeft = n->text.size() - line - 1;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next n.
		TextNode* newNode = NodeCounter::getNextNode(returnNode);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return TextCounter(returnNode, amount);
		}
		returnNode = newNode;
		linesLeft = returnNode->text.size() - 1;
	}
	return TextCounter(returnNode, amount);
}
void TextCounter::operator+=(int amount) {
	int linesLeft = n->text.size() - line - 1;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next n.
		n = NodeCounter::getNextNode(n);
		if (!n) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return;
		}
		linesLeft = n->text.size();
		line = 0;
	}
	line += amount;
}
TextCounter& TextCounter::operator++() {
	if (line + 1 < n->text.size()) {
		line++;
	}
	else {
		TextNode* newNode = NodeCounter::getNextNode(n);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return *this;
		}
		n = newNode;
		line = 0;
	}
	return *this;
}
TextCounter TextCounter::operator++(int) {
	if (line + 1 < n->text.size()) {
		line++;
	}
	else {
		TextNode* newNode = NodeCounter::getNextNode(n);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return *this;
		}
		n = newNode;
		line = 0;
	}
	return *this;
}

TextCounter TextCounter::operator-(int amount) const {
	TextNode* returnNode = n;
	int linesLeft = line;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next n.
		TextNode* newNode = NodeCounter::getLastNode(returnNode);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return TextCounter(returnNode, returnNode->text.size() - amount);
		}
		returnNode = newNode;
		linesLeft = returnNode->text.size();
	}
	return TextCounter(returnNode, returnNode->text.size() - amount);
}
void TextCounter::operator-=(int amount) {
	int linesLeft = line;
	while (amount > linesLeft) {
		amount -= linesLeft + 1;// One is added because one is required to get the next n.
		n = NodeCounter::getLastNode(n);
		if (!n) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
		}
		linesLeft = n->text.size();
	}
	line = n->text.size() - amount - 1;
}
TextCounter& TextCounter::operator--() {
	if (line > 0) {
		line--;
	}
	else {
		TextNode* newNode = NodeCounter::getLastNode(n);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return *this;
		}
		n = newNode;
		line = n->text.size() - 1;
	}
	return *this;
}
TextCounter TextCounter::operator--(int) {
	if (line > 0) {
		line--;
	}
	else {
		TextNode* newNode = NodeCounter::getLastNode(n);
		if (!newNode) {
			postMessage(MESSAGE_TYPE::M_WARNING, "Text counter index out of bounds.");
			return *this;
		}
		n = newNode;
		line = n->text.size() - 1;
	}
	return *this;
}
bool TextCounter::isLast() const {
	if (line == n->text.size() - 1) {
		if (!NodeCounter::getNextNode(n)) {
			return true;
		}
	}
	return false;
}

TextCounter TextTree::first() const {
	if (!root) {
		return TextCounter(nullptr, 0);
	}
	TextNode* node = NodeCounter::getNextNode(root);
	return TextCounter(node, 0);
}


TextNode* NodeCounter::getNextNode(TextNode* node) {
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

	/*
	// Search downwards, taking all the left branches.
	while (!now->children.empty()) {
		now = now->children[0];
	}*/
	return now;
}

TextNode* NodeCounter::getLastNode(TextNode* node) {
	if (!node) { return nullptr; }
	if (!node->parent) { return nullptr; }
	TextNode* parent = node->parent;
	if (std::find(parent->children.begin(), parent->children.end(), node) == parent->children.begin()) {
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



TextTree::TextTree(TextNode* root) : root(root) {
	changed = false;
}

TextCounter TextTree::operator[](int line) const {
	if (line == 0) { return first(); }
	return first() + line;
}

TextTree::TextTree(TextTree&& t) {
	root = std::move(t.root);
	changed = t.changed;
	t.root = nullptr;
}

TextTree& TextTree::operator=(TextTree&& t) {
	root = std::move(t.root);
	changed = t.changed;
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

void TextTree::cutNode(int begin, int end) {
	// Make a new node between begin and end.
}

void TextTree::insertLine(int index) {
	TextCounter line = operator[](index);
	line.n->text.insert(line.line + line.n->text.begin() + 1, "");
	line.n->text.insert(line.line + line.n->text.begin() + 1, "");
}

void TextTree::remove(int index) {
	TextCounter line = operator[](index);
	line.n->text.erase(line.line + line.n->text.begin());
	line = operator[](index);
	line.n->text.erase(line.line + line.n->text.begin());
	if (line.n->text.size() == 0) {
		TextNode* parent = line.n->parent;
		if (parent) {
			parent->children.erase(std::find(parent->children.begin(), parent->children.end(), line.n));
			for (TextNode* child : line.n->children) {
				parent->children.push_back(child);
				child->parent = parent;
			}
			delete line.n;
		}
		else {
			int debug = 0;
		}
	}
}

int TextTree::size() const {
	int sum = 0;
	for (TextNode* n = root; n != nullptr; n = NodeCounter::getNextNode(n)) {
		sum += n->text.size();
	}
	return sum;
}



bool FileLoader::loadFile(const std::filesystem::path filePath) {
	thisPath = filePath;
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

	textTree.changed = false;
	return true;
}

void FileLoader::saveFile(const std::filesystem::path filePath) {
	std::ofstream file;
	file.open(filePath);
	writeNode(textTree.root, &file);
	file.close();

	textTree.changed = false;
}

void FileLoader::writeNode(const TextNode* n, std::ofstream* file) {
	// Write header 
	(*file) << '#' + n->type << std::endl;

	// Write lines
	for (int i = 0; i < n->text.size(); i++) {
		if (n->text[i][0] == '#' || n->text[i][0] == '$') {
			(*file) << '\\';
		}
		(*file) << n->text[i] << std::endl;
	}

	// Write children
	for (TextNode* child : n->children) {
		writeNode(child, file);
	}

	// Write footer 
	(*file) << '$' << std::endl;
}

void FileLoader::newFile() {
	thisPath = "";
	textTree = std::move(TextTree());
}

void FileLoader::unload() {
	thisPath = "";
	textTree.~TextTree();
}


TextTree::TextTree() {
	root = new TextNode;
	root->type = "document";
	root->children.push_back(new TextNode);
	root->children[0]->type = "p";
	root->children[0]->text.push_back("\n");
	changed = false;
}