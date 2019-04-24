/**
 * Renderer.h
 * Defines the functions for loading and rendering models.
 */

#pragma once

#include "../Components.h"
#include "Texture.h"

namespace game::renderer
{
	//Initialises the render system
	void init();

	GLuint get_shader(
		bool textured, bool normal_mapped, size_t n_ambient, size_t n_directional, size_t n_point, std::string vertex_shader, std::string fragment_shader);

	glm::mat4 proj_matrix(CameraComponent camera);
	glm::mat4 view_matrix(CameraComponent camera);

	//Loads the given model
	void load_model(std::string file);

	// external textures for a given model
	void load_external_map(std::string path, std::string model_path, TextureType type);
	void load_external_map(std::string paths[6], std::string model_path, TextureType type, bool skybox);

	//Renders an individual model using the given camera
	void render_model(CameraComponent camera, ModelComponent &model, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients,
		size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points);
};
