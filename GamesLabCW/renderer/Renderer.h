/**
 * Renderer.h
 * Defines the functions for loading and rendering models.
 */

#pragma once

#include "../Components.h"

namespace game::renderer
{
	//Initialises the render system
	void init();

	//Renders an individual model using the given camera
	void render_model(CameraComponent camera, ModelComponent &model, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients,
		size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points);

	//Loads the given model
	void load_model(std::string file);

	// external textures for a given model
	/*void load_external_texture(std::string path, std::string model_path, TextureType type);
	void load_external_cubemap(std::string paths[6], std::string model_path, TextureType type, bool skybox);*/

	//Finalises the loaded models
	void finalise();

	// Get bone transforms for animation
	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms, std::string file);
}
