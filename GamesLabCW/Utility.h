/**
 * Utility.h
 * Defines engine-wide, general-purpose utilities.
 */

#pragma once

#include <string>

namespace game::utility
{
	//Reads an entire file as a string
	std::string read_file(std::string file_path);
}
