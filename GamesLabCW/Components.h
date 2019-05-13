/**
 * Components.h
 * Defines the basic component data structures.
 */

#pragma once

#include <string>
#include <unordered_set>

#include "GameEngine.h"
#include "Vector2.h"
#include "Vector3.h"
#include "SpatialGrid.h"

namespace game
{
	/* GAME AND ENGINE */

	struct GameStateComponent {};
	struct FirstPersonControllerComponent {};

	// Tracks how many keys are picked up, and where in the game they should be rendered
	struct KeyComponent
	{
		bool pickedUp = false;
		Vector3 destination;
	};

	/* CAMERAS AND RENDERING */

	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale{ 1.0, 1.0, 1.0 };

		SpatialGrid<Entity>::index_type last_index;
	};

	struct CameraComponent
	{
		Entity follow;
		Vector3 position;
		Vector2 orientation;
		double fov = 60.0;
	};

	struct ModelComponent
	{
		std::string model_file;
		double shininess = 32.0;
		std::string vertex_shader;
		std::string fragment_shader;
		bool isAnimated = false;
	};

	struct ColourComponent
	{
		Vector3 colour;
		double alpha = 1.0;
	};


	/* LIGHTING */

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
		Vector3 position{ 0.0, 0.0, 0.0 };
		double constant = 0;
		double linear = 0;
		double exponent = 1;
	};


	/* PHYSICS */

	struct KinematicComponent
	{
		Vector3 velocity;
		Vector3 acceleration;
	};

	struct CollisionComponent
	{
		double radius = 3;
		std::unordered_set<Entity> colliding;
	};

	struct MoveSphere {};
}
