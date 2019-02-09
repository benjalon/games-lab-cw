/**
 * Renderer.h
 */

#pragma once

#include "Components.h"

namespace game::renderer
{
	//Initialises the render system
	void init();

	//Renders an individual model using the given camera
	void render_model(CameraComponent camera, ModelComponent model, TransformComponent t);
}
