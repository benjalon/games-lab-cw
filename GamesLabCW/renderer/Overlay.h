#pragma once

#include "../Components.h"
#include "Texture.h"

namespace game
{
	class Overlay
	{
	public:
		Overlay(Texture texture, Vector2 position);
		void Render(GLuint shaderProgram);
	private:
		GLuint vao;
		Texture texture;
		Vector2 position;
	};
}