#include "ParticleEffect.h"

namespace game
{
	ParticleEffect::ParticleEffect(Texture texture, int amount, float scale, float speed) : texture(texture), amount(amount), scale(scale), speed(speed)
	{
		// Set up mesh and attribute properties
		GLuint vbo;

		// xys used for positions, zws used for texcoords
		GLfloat particleCoords[] = {
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			1.0f, 0.0f, 1.0f, 0.0f
		};

		glGenVertexArrays(1, &this->vao);
		glGenBuffers(1, &vbo);

		// Build up vertex/tex coord info for the vbo
		glBindVertexArray(this->vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particleCoords), particleCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);

		// Create initial particles
		for (size_t i = 0; i < this->amount; ++i)
		{
			this->particles.push_back(Particle());
		}
	}

	void ParticleEffect::Update(float dt, int newParticles, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset)
	{
		// Respawn particles at given speed, assuming there are some dead
		for (size_t i = 0; i < newParticles && i < deadParticles; ++i)
		{
			int unusedParticle = this->firstUnusedParticle();
			this->respawnParticle(this->particles[unusedParticle], positionVariation, velocityVariation, colorVariation, offset);
			deadParticles--;
		}

		float scaledDT = dt * speed;

		for (size_t i = 0; i < this->amount; ++i)
		{
			Particle &p = this->particles[i];
			p.Life -= scaledDT;

			// If still alive, perform the update
			if (p.Life > 0.0f)
			{
				p.Position -= p.Velocity * scaledDT;
				p.Color.a = p.Life > 1.0 ? 1.0 : p.Life; // Slowly turn the particle invisible towards the end of its life
			}
			else
			{
				deadParticles++;
			}
		}
	}

	void ParticleEffect::Render(GLuint shaderProgram)
	{
		// Bind array object and blend
		glBindVertexArray(vao);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		// Set uniforms for shader
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
				glBindVertexArray(this->vao);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindVertexArray(0);
			}
		}

		// Reset array object and blend setting
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(0);
	}

	// Stores ID of last used particle for fast access
	GLuint lastUsedParticle = 0;
	GLuint ParticleEffect::firstUnusedParticle()
	{
		// Find last used particle
		for (GLuint i = lastUsedParticle; i < this->amount; ++i) {
			if (this->particles[i].Life <= 0.0f) {
				lastUsedParticle = i;
				return i;
			}
		}
		
		// Last used not found, do simple search
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

		lastUsedParticle = 0;
		return 0;
	}

	void ParticleEffect::respawnParticle(Particle &particle, Vector3 positionVariation, Vector3 velocityVariation, Vector3 colorVariation, glm::vec3 offset)
	{
		particle.Position = glm::vec3(positionVariation.ToGLM()) + offset;
		particle.Color = glm::vec4(colorVariation.x, colorVariation.y, colorVariation.z, 1.0f);
		particle.Life = rand() % 5 + 1;
		particle.Velocity = glm::vec3(velocityVariation.ToGLM()) * 0.1f;
		particle.Scale = scale;
	}
}