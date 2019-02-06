/**
 * Components.h
 * Defines the basic component data structures.
 */

#pragma once

#include "Vector3.h"

namespace game
{
	struct TransformComponent
	{
		Vector3 position;
		Vector3 scale;
	};

	struct KinematicComponent
	{
		Vector3 velocity;
		Vector3 acceleration;
	};
}
