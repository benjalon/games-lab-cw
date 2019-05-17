/**
 * Utility.h
 * Defines engine-wide, general-purpose utilities.
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <string>

namespace game::utility
{
	//Reads an entire file as a string
	std::string read_file(std::string file_path);

	//Checks if an element is in a collection
	template <typename T>
	bool contains(const T &collection, const typename T::value_type &elt)
	{
		return collection.find(elt) != collection.end();
	}
}

#endif