/**
 * VBO.cpp
 * Implements the VBO class, representing
 * a vertex buffer object in OpenGL.
 */

#include "VBO.h"

game::VBO::VBO() : id_(0), data_uploaded_(false) {}

void game::VBO::create(GLuint size)
{
	glGenBuffers(1, &id_);
	data_.reserve(size);
	this->size_ = size;
	current_size_ = 0;
}

void game::VBO::remove()
{
	glDeleteBuffers(1, &id_);
	data_uploaded_ = false;
	data_.clear();
}

GLuint game::VBO::id() const { return id_; }

bool game::VBO::created() const { return id_ != 0; }

size_t game::VBO::size() const { return current_size_; }

void game::VBO::add_data(void *pData, size_t size)
{
	data_.insert(data_.end(), (unsigned char*)pData, (unsigned char*)pData + size);
	current_size_ += size;
}

void game::VBO::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, id_);
}

void game::VBO::upload(GLenum usage)
{
	if (!data_.empty())
	{
		glBufferData(GL_ARRAY_BUFFER, data_.size(), &data_[0], usage);
		data_uploaded_ = true;
		data_.clear();
	}
}
