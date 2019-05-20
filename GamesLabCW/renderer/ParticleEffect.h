#pragma once

#include "../Components.h"
#include "Texture.h"

namespace game
{
	struct Particle 
	{
		glm::vec3 Position;
		glm::vec3 Velocity;
		glm::vec4 Color;
		float Scale;
		float Life;
		Particle()
			: Position(0.0f), Velocity(0.0f), Color(1.0f), Scale(1.0f), Life(0.0f) { }
	};

	class ParticleEffect
	{
	public:
		ParticleEffect(Texture texture, int amount, float scale, float speed);
		void Update(float dt, int newParticles, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));
		void Render(GLuint shaderProgram);
	private:
		std::vector<Particle> particles;
		int amount;
		int deadParticles = 0;
		float scale;
		float speed;
		Texture texture;
		GLuint vao;
		GLuint firstUnusedParticle();
		void respawnParticle(Particle &particle, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));
	};
}