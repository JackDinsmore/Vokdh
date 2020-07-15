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
	TextCounter operator+(int amount) const;
	void operator+=(int amount);
	TextCounter& operator++();
	TextCounter operator++(int);
	TextCounter operator-(int amount) const;
	void operator-=(int amount);
	TextCounter& operator--();
	TextCounter operator--(int);
	bool operator==(const TextCounter tc) const { return node == tc.node && line == tc.line; }
	bool operator!=(const TextCounter tc) const { return node != tc.node || line != tc.line; }

	std::string& text() { if (!node) throw std::exception("Invalid TextCounter"); return node->text[line]; }
	std::string& type() { if (!node) throw std::exception("Invalid TextCounter"); return node->type; }

protected:
	TextCounter(TextNode* node, int line) : node(node), line(line) {}

private:
	TextNode* node;
	int line;

	static TextNode* getNextNode(TextNode* node);
	static TextNode* getLastNode(TextNode* node);
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
	TextCounter last() const;
	int size() const;

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