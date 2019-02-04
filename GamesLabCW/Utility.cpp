#include "Utility.h"

#define _USE_MATH_DEFINES
#include <math.h>

std::string Utility::ReadFile(const char *filePath) {
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

float Utility::ToRadians(float x)
{
	return (x * M_PI) / 180.0f;
}
