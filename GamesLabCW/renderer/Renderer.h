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
	void render_model(CameraComponent camera, ModelComponent model, CubemapComponent cm, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients,
		size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points);

	//Loads the given model
	void load(std::string file);

	//Finalises the loaded models
	void finalise();
}
