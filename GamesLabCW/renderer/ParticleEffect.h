#pragma once

#include "../Components.h"
#include "Texture.h"

namespace game
{
	struct Particle {
		glm::vec2 Position, Velocity;
		glm::vec4 Color;
		GLfloat Life;

		Particle()
			: Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
	};

	class ParticleEffect
	{
	public:
		// Constructor
		ParticleEffect(Texture texture, GLuint amount);
		// Update all particles
		void Update(GLfloat dt, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
		// Render all particles
		void Draw(GLuint shaderProgram);
	private:
		// State
		std::vector<Particle> particles;
		GLuint amount;
		// Render state
		Texture texture;
		GLuint VAO;
		// Initializes buffer and vertex attributes
		void init();
		// Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
		GLuint firstUnusedParticle();
		// Respawns particle
		void respawnParticle(Particle &particle, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	};
}