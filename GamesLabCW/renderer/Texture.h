/**
 * Texture.h
 * Declares the Texture structure, representing a 2D
 * texture.
 */

#pragma once

#include <string>

namespace game
{
	enum TextureType
	{
		DIFFUSE,
		NORMAL,
		SPECULAR,
		CUBE
	};

	struct Texture
	{
		//Handle for the texture
		unsigned int handle;
		//Handle for the sampler
		unsigned int sampler;

		//Texture path
		std::string path;

		int width, height;
		bool genMipMaps = false;

		TextureType type;
		bool isSkybox = false; // switch for disabling depth settings

		//Constructs a Texture from the given file
		Texture(const char *filename, bool genMipMaps = true);
		Texture(std::string filename, bool genMipMaps = true);

		// Cubemaps
		Texture(std::string filenames[6], bool genMipMaps = true);
	};
}
