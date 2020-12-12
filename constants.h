#pragma once

#include <filesystem>
#include <string>

#define FRAMERATE 25

inline extern std::filesystem::path exePath = std::filesystem::path();

std::string lowerString(std::string data);