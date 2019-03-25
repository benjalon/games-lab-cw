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
	class BO
	{
	private:
		GLuint id_;
		GLuint size_;
		size_t current_size_;
		bool data_uploaded_;
		GLenum target;

		std::vector<unsigned char> data_;

	public:
		//Default constructor
		BO();

		//Constructor for alternate types of buffer
		BO(GLenum target);

		//Initialises vertex buffer object
		void create(GLuint size = 0);

		//Destroys vertex buffer object, freeing memory
		void remove();

		//Returns the ID of the BO
		GLuint id() const;

		//Queries if the BO has been created
		bool created() const;

		//Returns the total size of data in the BO
		size_t size() const;

		//Adds any amount of arbitrary byte data to BO
		void add_data(void *pData, size_t size);

		//Binds the BO
		void bind() const;

		//Uploads BO data to GPU
		void upload(GLenum usage);
	};
}
