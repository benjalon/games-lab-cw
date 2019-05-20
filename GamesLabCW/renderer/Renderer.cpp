/**
 * Renderer.cpp
 * Implements the functions for loading and rendering models.
 */

#define GLM_ENABLE_EXPERIMENTAL

#include "Renderer.h"

#include <map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "VBO.h"
#include "Model.h"
#include "ParticleEffect.h"

//Quick conversion to radians
#define R(x) glm::radians((float)x)

namespace game::renderer
{
	std::unordered_map<std::string, Model> models;
	std::unordered_map<std::string, ParticleEffect> particleEffects;
	std::map<std::string, Texture> externalTextures;

	void init()
	{
		//Configure OpenGL
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
	}

	//Returns the (potentially cached) shader using the given paramaters
	GLuint get_shader(
		bool textured, bool normal_mapped, size_t n_ambient, size_t n_directional, size_t n_point, std::string vertex_shader, std::string fragment_shader)
	{
		using Args = std::tuple<bool, bool, size_t, size_t, size_t, std::string, std::string>;

		//Cache of parametrised shaders
		static std::map<Args, Shader> shaders;

		Args args = std::make_tuple(textured, normal_mapped, n_ambient, n_directional, n_point,
			vertex_shader, fragment_shader);

		//If the requested shader already exists, return it
		auto it = shaders.find(args);
		if (it != shaders.end())
			return it->second.handle();

		//Else, compile and return the new shader
		else
		{
			//Shader parameters to prepend to source
			std::string v, f;
			auto define = [](std::string &s, std::string def) { s.append("#define " + def + "\n"); };

			if (textured) define(f, "TEXTURED");
			if (normal_mapped) define(f, "NORMAL_MAPPED");
			define(f, "N_AMBIENT " + std::to_string(n_ambient));
			define(f, "N_DIRECTIONAL " + std::to_string(n_directional));
			define(f, "N_POINT " + std::to_string(n_point));

			//Create new shader
			auto &s = shaders[args];
			s.load("",
				vertex_shader.empty() ? "shaders/Passthrough.vert" : vertex_shader.c_str(),
				fragment_shader.empty() ? "shaders/ParametrisedFragment.frag" : fragment_shader.c_str(),
				v, f);
			return s.handle();
		}
	}

	//Calculates the projection matrix for a camera
	glm::mat4 proj_matrix(CameraComponent camera)
	{
		return glm::infinitePerspective(R(camera.fov), (float)ASPECT_RATIO_VAL, 0.1f);
	}

	//Calculates the view matrix for a camera
	glm::mat4 view_matrix(CameraComponent camera)
	{
		//Calculate direction of view from angles
		glm::vec3 dir = camera.orientation.direction_hv();

		//Get a vector perpendicularly upwards
		glm::vec3 right = camera.orientation.direction_hv_right();
		glm::vec3 up = glm::cross(right, dir);

		return glm::lookAt(glm::vec3(camera.position), glm::vec3(camera.position) + dir, up);
	}

	void load_model(std::string file) {
		models.emplace(file, Model(file)).first->second;
	}

	void load_particle_effect(std::string texture, int count, float scale, float speed) {
		particleEffects.emplace(texture, ParticleEffect(texture, count, scale, speed)).first->second;
	}

	void load_external_map(std::string path, std::string model_path, TextureType type)
	{
		auto texture = Texture(path);
		texture.type = type;
		externalTextures.emplace(model_path, texture); // Need to map this texture with a model, since it was loaded externally
	}

	void load_external_map(std::string paths[6], std::string model_path, TextureType type, bool skybox)
	{
		auto cubemap = Texture(paths, skybox);
		cubemap.type = type;
		cubemap.isSkybox = skybox;
		externalTextures.emplace(model_path, cubemap); // Need to map this texture with a model, since it was loaded externally
	}

	void render_model(CameraComponent camera, ModelComponent &m, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients, size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points)
	{

		//Get the model, aborting if not found
		auto it = models.find(m.model_file);
		if (it == models.end()) return;
		Model &model = it->second;

		m.isAnimated = model.IsAnimated();
		
		//Determine and use appropriate shader
		GLuint shader = get_shader(model.IsTextured(), model.IsNormalMapped(), n_ambient, n_directional, n_point, m.vertex_shader, m.fragment_shader);
		glUseProgram(shader);

		//Calculate MVP matrices
		glm::mat4 matProj = proj_matrix(camera);

		glm::mat4 matView;

		auto tx_it = externalTextures.find(m.model_file);
		if (tx_it != externalTextures.end() && tx_it->second.isSkybox)
		{
			// Skyboxes must be rendered behind everything else so disregard camera transform 
			// and change depth setting
			glDepthFunc(GL_LEQUAL);
			matView = glm::mat3(view_matrix(camera));
		}
		else
		{
			// Regular render settings
			glDepthFunc(GL_LESS);
			matView = view_matrix(camera);
		}

		if (tx_it != externalTextures.end())
		{
			const Texture &texture = tx_it->second;

			glActiveTexture(GL_TEXTURE0);
			switch (texture.type)
			{
			case TextureType::DIFFUSE:
				glBindTexture(GL_TEXTURE_2D, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);
				break;
			case TextureType::NORMAL:
				glBindTexture(GL_TEXTURE_2D, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "normalSampler"), 0);
				break;
			case TextureType::SPECULAR:
				// Not yet implemented
				break;
			case TextureType::CUBE:
				glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "cubeSampler"), 0);
				break;

			default:
				break;
			}
			//Provide cubemap component (reflections, skyboxes etc.)
		}

		glm::mat4 matModel = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		//Provide MVP matrices
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "projectionMatrix"),
			1, GL_FALSE, glm::value_ptr(matProj)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "viewMatrix"),
			1, GL_FALSE, glm::value_ptr(matView)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "modelMatrix"),
			1, GL_FALSE, glm::value_ptr(matModel)
		);

		//Provide flat colour component
		glUniform4f(
			glGetUniformLocation(shader, "flatColour"),
			(GLfloat)c.colour.x, (GLfloat)c.colour.y, (GLfloat)c.colour.z,
			(GLfloat)c.alpha
		);

		//Provide shininess value (used to determine how much specular highlighting the model will have)
		glUniform1f(
			glGetUniformLocation(shader, "shininess"),
			(GLfloat)m.shininess
		);

		// Provide camera position for eye calculations
		glUniform3f(
			glGetUniformLocation(shader, "cameraPosition"),
			(GLfloat)camera.position.x, (GLfloat)camera.position.y, (GLfloat)camera.position.z
		);

		//Provide ambient lights information
		for (size_t i = 0; i < n_ambient; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("ambientLights[" + j + "].colour").c_str()),
				(GLfloat)ambients[i].colour.x,
				(GLfloat)ambients[i].colour.y,
				(GLfloat)ambients[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("ambientLights[" + j + "].intensity").c_str()),
				(GLfloat)ambients[i].intensity);
			glUniform1f(glGetUniformLocation(shader,
				("ambientLights[" + j + "].on").c_str()),
				(GLfloat)ambients[i].on);
		}

		//Provide directional lights information
		for (size_t i = 0; i < n_directional; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].colour").c_str()),
				(GLfloat)directionals[i].colour.x,
				(GLfloat)directionals[i].colour.y,
				(GLfloat)directionals[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].intensity").c_str()),
				(GLfloat)directionals[i].intensity);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].direction").c_str()),
				(GLfloat)directionals[i].direction.x,
				(GLfloat)directionals[i].direction.y,
				(GLfloat)directionals[i].direction.z);
			glUniform1f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].on").c_str()),
				(GLfloat)directionals[i].on);
		}

		//Provide point lights information
		for (size_t i = 0; i < n_point; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("pointLights[" + j + "].colour").c_str()),
				(GLfloat)points[i].colour.x,
				(GLfloat)points[i].colour.y,
				(GLfloat)points[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].intensity").c_str()),
				(GLfloat)points[i].intensity);
			glUniform3f(glGetUniformLocation(shader,
				("pointLights[" + j + "].position").c_str()),
				(GLfloat)points[i].position.x,
				(GLfloat)points[i].position.y,
				(GLfloat)points[i].position.z);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].constant").c_str()),
				(GLfloat)points[i].constant);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].linear").c_str()),
				(GLfloat)points[i].linear);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].exponent").c_str()),
				(GLfloat)points[i].exponent);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].on").c_str()),
				(GLfloat)points[i].on);
		}

		model.Render(shader);
	}

	void render_particle(CameraComponent camera, ParticleComponent &p, ColourComponent c, TransformComponent t)
	{
		//Get the particle, aborting if not found
		auto it = particleEffects.find(p.texture_file);
		if (it == particleEffects.end()) return;
		ParticleEffect &particle = it->second;

		GLuint shader = get_shader(false, false, 0, 0, 0, "shaders/Particle.vert", "shaders/Particle.frag");
		glUseProgram(shader);

		//Calculate MVP matrices
		glm::mat4 matProj = proj_matrix(camera);

		glm::mat4 matView = view_matrix(camera);

		glm::mat4 matModel = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		//Provide MVP matrices
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "projectionMatrix"),
			1, GL_FALSE, glm::value_ptr(matProj)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "viewMatrix"),
			1, GL_FALSE, glm::value_ptr(matView)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "modelMatrix"),
			1, GL_FALSE, glm::value_ptr(matModel)
		);

		particle.Draw(shader);
	}

	void animate_model(double time, std::string model_file) 
	{
		//Get the model, aborting if not found
		auto it = models.find(model_file);
		if (it == models.end()) return;
		Model &model = it->second;

		model.Animate(time);
	}

	void update_particle(double time, std::string texture_file, int respawn_count, Vector3 position_variation, Vector3 velocity_variation, Vector3 color_variation)
	{
		//Get the model, aborting if not found
		auto it = particleEffects.find(texture_file);
		if (it == particleEffects.end()) return;
		ParticleEffect &particleEffect = it->second;

		particleEffect.Update(time, respawn_count, position_variation, velocity_variation, color_variation);
	}
}
