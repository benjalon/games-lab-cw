/**
 * Components.h
 * Defines the basic component data structures.
 */

#pragma once

#include <string>

#include "GameEngine.h"
#include "Vector2.h"
#include "Vector3.h"

namespace game
{
	struct NameComponent
	{
		std::string name = "unnamed";
	};

	struct CameraComponent
	{
		Vector3 position;
		Vector2 orientation{ 180.0, 0.0 };
		double fov = 45.0;
	};

	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale{ 1.0, 1.0, 1.0 };
	};

	struct KinematicComponent
	{
		Vector3 velocity;
		Vector3 acceleration;
	};


	struct ModelComponent
	{
		GLuint vbo;
		GLuint ibo;

		ModelComponent()
		{
			//Create vertex buffer
			glm::vec3 vertices[4];
			vertices[0] = glm::vec3(-1.0f, -1.0f, 0.5773f);
			vertices[1] = glm::vec3(0.0f, -1.0f, -1.15475f);
			vertices[2] = glm::vec3(1.0f, -1.0f, 0.5773f);
			vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);
			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			//Create index buffer
			unsigned int indices[] {
				0, 3, 1,
				1, 3, 2,
				2, 3, 0,
				0, 1, 2
			};
			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		}
	};
}
