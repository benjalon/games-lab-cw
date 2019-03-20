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

		//Path to governing model
		std::string path;

		// Is this cubemap going to be used as a skybox
		bool skybox;

		//Constructs a Cubemap from the given file
		Cubemap(std::string modelPath, std::string texturePaths[6], bool skybox);
	};
}