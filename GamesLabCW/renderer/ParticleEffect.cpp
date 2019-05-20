#include "ParticleEffect.h"

namespace game
{
	ParticleEffect::ParticleEffect(Texture texture, GLuint amount, float scale, float speed)
		: texture(texture), amount(amount), scale(scale), speed(speed)
	{
		this->init();
	}

	void ParticleEffect::Update(GLfloat dt, GLuint newParticles, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset)
	{
		// Add new particles 
		for (GLuint i = 0; i < newParticles; ++i)
		{
			int unusedParticle = this->firstUnusedParticle();
			this->respawnParticle(this->particles[unusedParticle], positionVariation, velocityVariation, colorVariation, offset);
		}

		float scaledDT = dt * speed;

		// Update all particles
		for (GLuint i = 0; i < this->amount; ++i)
		{
			Particle &p = this->particles[i];
			p.Life -= scaledDT; // reduce life
			if (p.Life > 0.0f)
			{	// particle is alive, thus update
				p.Position -= p.Velocity * scaledDT;
				p.Color.a -= scaledDT * 2.5;
			}
		}
	}

	// Render all particles
	void ParticleEffect::Draw(GLuint shaderProgram)
	{
		glBindVertexArray(VAO);

		// Use additive blending to give it a 'glow' effect
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		for (Particle particle : this->particles)
		{
			if (particle.Life > 0.0f)
			{
				glUniform1f(
					glGetUniformLocation(shaderProgram, "scale"),
					(GLfloat)particle.Scale
				);

				glUniform3f(
					glGetUniformLocation(shaderProgram, "offset"),
					(GLfloat)particle.Position.x, (GLfloat)particle.Position.y, (GLfloat)particle.Position.z
				);

				glUniform4f(
					glGetUniformLocation(shaderProgram, "color"),
					(GLfloat)particle.Color.x, (GLfloat)particle.Color.y, (GLfloat)particle.Color.z,
					(GLfloat)particle.Color.w
				);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture.handle);
				glUniform1i(glGetUniformLocation(shaderProgram, "texSampler"), 0);
				glBindVertexArray(this->VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}
		}
		// Don't forget to reset to default blending mode
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(0);
	}

	void ParticleEffect::init()
	{
		// Set up mesh and attribute properties
		GLuint VBO;
		GLfloat particle_quad[] = {
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,

			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
		};
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(this->VAO);
		// Fill mesh buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
		// Set mesh attributes
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);

		// Create this->amount default particle instances
		for (GLuint i = 0; i < this->amount; ++i)
			this->particles.push_back(Particle());
	}

	// Stores the index of the last particle used (for quick access to next dead particle)
	GLuint lastUsedParticle = 0;
	GLuint ParticleEffect::firstUnusedParticle()
	{
		// First search from last used particle, this will usually return almost instantly
		for (GLuint i = lastUsedParticle; i < this->amount; ++i) {
			if (this->particles[i].Life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		// Otherwise, do a linear search
		for (GLuint i = 0; i < lastUsedParticle; ++i) {
			if (i >= particles.size()) {
				i = particles.size() - 1;
				return i;
			}
			if (this->particles[i].Life <= 0.0f) {

				lastUsedParticle = i;
				return i;
			}
		}
		// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
		lastUsedParticle = 0;
		return 0;
	}

	void ParticleEffect::respawnParticle(Particle &particle, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset)
	{
		particle.Position = glm::vec3(positionVariation.ToGLM()) + offset;
		particle.Color = glm::vec4(colorVariation.x, colorVariation.y, colorVariation.z, 1.0f);
		particle.Life = 1.0f;
		particle.Velocity = glm::vec3(velocityVariation.ToGLM()) * 0.1f;
		particle.Scale = scale;
	}
}