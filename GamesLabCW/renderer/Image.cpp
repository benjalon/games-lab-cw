#include "Image.h"

namespace game
{
	Image::Image(Texture texture, Vector2 position) : texture(texture), position(position)
	{
		// Set up mesh and attribute properties
		GLuint vbo;

		// xys used for positions, zws used for texcoords
		GLfloat squareCoords[] = {
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &this->vao);
		glGenBuffers(1, &vbo);

		// Build up vertex/tex coord info for the vbo
		glBindVertexArray(this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(squareCoords), squareCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);
	}

	void Image::Render(GLuint shaderProgram)
	{
		// Bind array object and blend
		glBindVertexArray(vao);

		// Set uniforms for shader.0f)
		glUniform2f(
			glGetUniformLocation(shaderProgram, "position"),
			(GLfloat)position.x, (GLfloat)position.y
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture.handle);
		glUniform1i(glGetUniformLocation(shaderProgram, "texSampler"), 0);
		glBindVertexArray(this->vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}