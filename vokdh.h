#pragma once
#include <windows.h>
#include <d2d1.h>
#include <fstream>
#include <ctime>
#include <filesystem>

#include "d2tools.h"
#include "view.h"
#include "message.h"
#include "style.h"
#include "text.h"
#include "translationView.h"
#include "helpView.h"


class Logger : Listener, Poster {
public:
	Logger() {
		StyleMap styleMap = styleMap.summon();
		std::string loggerStyle = (std::string)styleMap["logger"]["logger-level"];
		if (loggerStyle == "errors") {
			lowerLimit = MESSAGE_TYPE::M_ERROR;
		}
		else if (loggerStyle == "warnings") {
			lowerLimit = MESSAGE_TYPE::M_WARNING;
		}
		else if (loggerStyle == "info") {
			lowerLimit = MESSAGE_TYPE::M_INFO;
		}
		else if (loggerStyle == "debug") {
			lowerLimit = MESSAGE_TYPE::M_DEBUG;
		}
		else {
			lowerLimit = MESSAGE_TYPE::M_INFO;
		}
	}

	~Logger() {
		if (logFile.is_open()) {
			logFile.close();
		}
	}

	void log(Message m) {
		if (!logFile.is_open()) {
			logFile.open("vokdh.log");
			logFile << "Log for Vokdh word processor." << std::endl;
			time_t  timev;
			logFile << std::time(&timev) << std::endl;
		}

		std::string prepend = "";
		switch (m.type) {
		case MESSAGE_TYPE::M_TERMINATE:
		case MESSAGE_TYPE::M_ERROR:
			prepend = "ERROR: ";
			break;
		case MESSAGE_TYPE::M_WARNING:
			prepend = "WARNING: ";
			break;
		case MESSAGE_TYPE::M_INFO:
			prepend = "INFO: ";
			break;
		case MESSAGE_TYPE::M_DEBUG:
			prepend = "DEBUG: ";
			break;
		}

		logFile << prepend << m.m << std::endl;
	}

	void update() {
		Message msg;
		while (popMessage(&msg)) {
			if (msg.type <= lowerLimit) {
				log(msg);
			}
		}
	}

private:
	MESSAGE_TYPE lowerLimit;
	std::ofstream logFile;
};


class Vokdh : Listener, Poster {
public:
	Vokdh(std::string cmdLines);

public:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL createDeviceIndependentResources(HINSTANCE hInstance);
	HWND window() const { return hwnd; }
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void update();

public:
	bool quit = false;

private:
	HRESULT createDeviceDependentResources();
	void discardDeviceDependentResources();
	void paint();
	void resize();
	bool handleKeyPress(int key);
	void handleLeftClick(int keydown, int posx, int posy);
	void handleLeftDoubleClick(int posx, int posy);
	void toggleFullscreen();
	void saveAs();
	void open();
	void newFile();
	bool checkChanged();

private:
	ID2D1Factory* factory = nullptr;
	ID2D1HwndRenderTarget* renderTarget = nullptr;

	HWND hwnd;
	UINT dpi;

	Logger logger;
	ViewHandler viewHandler;

	std::filesystem::path openFilePath;

	TextTree textTree;
	FileLoader loader;
};
