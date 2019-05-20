#pragma once

#include "../Components.h"
#include "Texture.h"

namespace game
{
	class Image
	{
	public:
		Image(Texture texture, Vector2 position);
		void Render(GLuint shaderProgram);
	private:
		GLuint vao;
		Texture texture;
		Vector2 position;
	};
}