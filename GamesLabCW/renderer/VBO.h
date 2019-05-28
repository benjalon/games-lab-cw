/**
 * VBO.h
 * Declares the VBO class, representing
 * a vertex buffer object in OpenGL.
 */

#pragma once

#include <glad\glad.h>
#include <vector>

namespace game
{
	class VBO
	{
	private:
		GLuint id_;
		GLuint size_;
		size_t current_size_;
		bool data_uploaded_;
		GLenum target;
		bool update;

		std::vector<unsigned char> data_;

	public:
		//Default constructor
		VBO();
		
		//Constructor for alternate types of buffer
		VBO(GLenum target, bool update);

		//Initialises vertex buffer object
		void create(GLuint size = 0);

		//Destroys vertex buffer object, freeing memory
		void remove();

		//Returns the ID of the VBO
		GLuint id() const;

		//Queries if the VBO has been created
		bool created() const;

		//Returns the total size of data in the VBO
		size_t size() const;

		//Adds any amount of arbitrary byte data to VBO
		void add_data(void *pData, size_t size);

		//Binds the VBO
		void bind() const;

		//Uploads VBO data to GPU
		void upload(GLenum usage);
	};
}

