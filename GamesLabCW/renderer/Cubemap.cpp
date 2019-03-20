/**
 * Cubemap.cpp
 * Implements the Cubemap class, representing a group of textures.
 */

#include "Cubemap.h"

#include <glad/glad.h>
#include <SOIL/SOIL.h>

#include <iostream>

game::Cubemap::Cubemap(std::string filenames[6])
{
	//Load cubemap from file
	handle = SOIL_load_OGL_cubemap
	(
		filenames[0].c_str(),
		filenames[1].c_str(),
		filenames[2].c_str(),
		filenames[3].c_str(),
		filenames[4].c_str(),
		filenames[5].c_str(),
		SOIL_LOAD_RGB,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS
	);

	//Check for loading error
	if (handle != 0)
		std::cout << "Loaded cubemap " << std::endl;
	else
	{
		std::cout << "Error: couldn't load cubemap " << std::endl << SOIL_last_result() << std::endl;
		return;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

	//Generate sampler
	glGenSamplers(1, &sampler);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}