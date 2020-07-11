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
	BOOL createDeviceIndependentResources(PCWSTR lpWindowName, DWORD dwStyle, DWORD dwExStyle = 0, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT, int nHeight = CW_USEDEFAULT, HWND hWndParent = 0, HMENU hMenu = 0);
	HWND window() const { return hwnd; }
	PCWSTR className() const { return L"Vokdh class"; }
	LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void update();

public:
	bool quit = false;

private:
	HRESULT createDeviceDependentResources();
	void discardDeviceDependentResources();
	void paint();
	void resize();

private:
	ID2D1Factory* factory;
	ID2D1HwndRenderTarget* renderTarget;

	HWND hwnd;

	Logger logger;
	TranslationView translationView;
	View* view = &translationView;

	std::filesystem::path openFilePath;

	TextTree textTree;
	FileLoader loader;
};
