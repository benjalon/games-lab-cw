/**
 * VBO.cpp
 * Implements the VBO class, representing
 * a vertex buffer object in OpenGL.
 */

#include "VBO.h"

namespace game
{
	VBO::VBO() : id_(0), data_uploaded_(false), target(GL_ARRAY_BUFFER) {}

	VBO::VBO(GLenum target, bool update) : id_(0), data_uploaded_(false) { this->target = target; this->update = update; }

	void VBO::create(GLuint size)
	{
		if (created()) {
			return;
		}

		glGenBuffers(1, &id_);
		data_.reserve(size);
		this->size_ = size;
		current_size_ = 0;
	}

	void VBO::remove()
	{
		glDeleteBuffers(1, &id_);
		data_uploaded_ = false;
		
		if (!update)
		{
			data_.clear();
		}
	}

	GLuint VBO::id() const { return id_; }

	bool VBO::created() const { return id_ != 0; }

	size_t VBO::size() const { return current_size_; }

	void VBO::add_data(void *pData, size_t size)
	{
		data_.insert(data_.end(), (unsigned char*)pData, (unsigned char*)pData + size);
		current_size_ += size;
	}

	void VBO::bind() const
	{
		glBindBuffer(target, id_);
	}

	void VBO::upload(GLenum usage)
	{
		if (data_.empty())
		{
			return;
		}

		if (data_uploaded_ && update)
		{
			glBufferSubData(target, 0, data_.size(), &data_[0]);
		}
		else
		{
			glBufferData(target, data_.size(), &data_[0], usage);
		}

		data_.clear();
		data_uploaded_ = true;
	}
}