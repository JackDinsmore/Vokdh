#pragma once
#include <vector>
#include <string>
#include <filesystem>

#include "message.h"

struct TextNode {
	std::vector<TextNode*> children;
	TextNode* parent = nullptr;
	std::vector<std::string> text;
	std::string type;
};

class TextCounter {
	friend class TextTree;
public:
	TextCounter(TextNode* node, int line) : n(node), line(line) {}
	TextCounter() {}

	TextCounter operator+(int amount) const;
	void operator+=(int amount);
	TextCounter& operator++();
	TextCounter operator++(int);
	TextCounter operator-(int amount) const;
	void operator-=(int amount);
	TextCounter& operator--();
	TextCounter operator--(int);
	bool operator==(const TextCounter tc) const { return n == tc.n && line == tc.line; }
	bool operator!=(const TextCounter tc) const { return n != tc.n || line != tc.line; }

public:
	bool isLast() const;

	std::string& text() { if (!n) throw std::exception("Invalid TextCounter"); return n->text[line]; }
	std::string& type() { if (!n) throw std::exception("Invalid TextCounter"); return n->type; }

protected:
	TextNode* n = nullptr;
	int line;
};

class NodeCounter {
	friend class TextCounter;
	friend class TextTree;
public:
	NodeCounter(TextNode* node) : node(node) {}

	NodeCounter& operator++() { node = getNextNode(node); return *this; }
	NodeCounter operator++(int) { node = getNextNode(node); return *this; }
	NodeCounter& operator--() { node = getLastNode(node); return *this; }
	NodeCounter operator--(int) { node = getLastNode(node); return *this; }
	bool operator==(const NodeCounter tc) const { return node == tc.node; }
	bool operator!=(const NodeCounter tc) const { return node != tc.node; }

public:
	bool isLast() const { if (!getNextNode(node)) { return true; } return false; }

	std::vector<std::string>& text() { if (!node) throw std::exception("Invalid TextCounter"); return node->text; }
	std::string& type() { if (!node) throw std::exception("Invalid TextCounter"); return node->type; }

protected:
	static TextNode* getNextNode(TextNode* node);
	static TextNode* getLastNode(TextNode* node);

private:
	TextNode* node;
};

class TextTree {
	friend class FileLoader;
public:
	TextTree();
	TextTree(TextNode* root);
	~TextTree() { deleteNode(root); }
	TextTree(const TextTree&) = delete;
	TextTree& operator=(const TextTree&) = delete;
	TextTree(TextTree&&);
	TextTree& operator=(TextTree&&);

	TextCounter operator[](int line) const;
	TextCounter first() const;
	NodeCounter firstNode() const { return NodeCounter(root); }
	int size() const;

	void cutNode(int begin, int end);
	void insertLine(int index);
	void remove(int index);

private:
	void deleteNode(TextNode* n);

protected:
	TextNode* root = nullptr;
};

class FileLoader : Poster {
public:
	FileLoader(TextTree& textTree) : textTree(textTree) {}
	bool loadFile(std::filesystem::path filePath);
	void save() { saveFile(thisPath); }
	void saveFile(std::filesystem::path filePath);
	void newFile();
	void unload();

private:
	void writeNode(const TextNode* n, std::ofstream* file);

public:
	std::filesystem::path thisPath;

private:
	TextTree& textTree;
};