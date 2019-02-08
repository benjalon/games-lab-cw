/**
 * Components.h
 * Defines the basic component data structures.
 */

#pragma once

#include <string>

#include "Vector3.h"

namespace game
{
	struct NameComponent
	{
		std::string name;
	};

	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
	};

	struct KinematicComponent
	{
		Vector3 velocity;
		Vector3 acceleration;
	};
}
