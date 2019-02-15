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
		double fov = 60.0;
	};

	struct ModelComponent
	{
		std::string model_file;
		double shininess = 32.0;
	};

	struct ColourComponent
	{
		Vector3 colour;
		double alpha = 1.0;
	};

	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale{ 1.0, 1.0, 1.0 };
	};

	struct AmbientLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
	};

	struct DirectionalLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
		Vector3 direction{ 0.0, 0.0, 0.0 };
	};

	struct PointLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
		Vector3 position{ 2.0, 0.0, 0.0 };
		double constant = 1.0;
		double linear = 0.1;
		double exponent = 0.0;
	};

	struct KinematicComponent
	{
		Vector3 velocity;
		Vector3 acceleration;
	};
}
