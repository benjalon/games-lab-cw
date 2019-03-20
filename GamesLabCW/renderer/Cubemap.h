/**
 * Cubemap.h
 * Declares the Cubemap structure, representing a group of textures.
 */

#pragma once

#include <string>

namespace game
{
	struct Cubemap
	{
		//Handle for the cubemap
		unsigned int handle;
		//Handle for the sampler
		unsigned int sampler;

		//Constructs a Cubemap from the given file
		Cubemap(std::string filenames[6]);
	};
}