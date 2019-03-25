/**
 * BO.cpp
 * Implements the BO class, representing
 * a buffer object in OpenGL.
 */

#include "BO.h"

game::BO::BO() : id_(0), data_uploaded_(false), target(GL_ARRAY_BUFFER){}

game::BO::BO(GLenum target) : id_(0), data_uploaded_(false) { BO::target = target; }

void game::BO::create(GLuint size)
{
	glGenBuffers(1, &id_);
	data_.reserve(size);
	this->size_ = size;
	current_size_ = 0;
}

void game::BO::remove()
{
	glDeleteBuffers(1, &id_);
	data_uploaded_ = false;
	data_.clear();
}

GLuint game::BO::id() const { return id_; }

bool game::BO::created() const { return id_ != 0; }

size_t game::BO::size() const { return current_size_; }

void game::BO::add_data(void *pData, size_t size)
{
	data_.insert(data_.end(), (unsigned char*)pData, (unsigned char*)pData + size);
	current_size_ += size;
}

void game::BO::bind() const
{
	glBindBuffer(target, id_);
}

void game::BO::upload(GLenum usage)
{
	if (!data_.empty())
	{
		glBufferData(target, data_.size(), &data_[0], usage);
		data_uploaded_ = true;
		data_.clear();
		glBindBuffer(target, 0);
	}
}
