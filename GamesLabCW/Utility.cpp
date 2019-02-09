/**
 * Utility.cpp
 * Implements engine-wide, general-purpose utilities.
 */

#include "Utility.h"

#include <fstream>

std::string game::utility::read_file(std::string file_path)
{
	std::string content;
	std::ifstream file_stream(file_path, std::ios::in);

	if (!file_stream.is_open())
		return "";

	std::string line = "";
	while (!file_stream.eof())
	{
		std::getline(file_stream, line);
		content.append(line + "\n");
	}

	file_stream.close();
	return content;
}