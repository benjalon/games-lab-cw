/**
 * Texture.cpp
 * Implements the Texture class, representing a 2D
 * texture.
 */

#include "Texture.h"

#include <glad/glad.h>
#include <SOIL/SOIL.h>

#include <iostream>

game::Texture::Texture(std::string filename, bool genMipMaps) : Texture(filename.c_str(), genMipMaps) {}

game::Texture::Texture(const char *filename, bool genMipMaps)
{
	//Load texture from file
	handle = SOIL_load_OGL_texture
	(
		filename,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_POWER_OF_TWO
	);

	//Check for loading error
	if (handle != 0)
		std::cout << "Loaded texture " << filename << std::endl;
	else
	{
		std::cout << "Error: couldn't load " << filename << std::endl << SOIL_last_result() << std::endl; 
		return;
	}

	glBindTexture(GL_TEXTURE_2D, handle);

	//Generate MipMaps if specified
	if (genMipMaps)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
		this->genMipMaps = true;
	}

	//Generate sampler
	glGenSamplers(1, &sampler);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	path = filename;
}
