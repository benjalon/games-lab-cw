/**
 * Scene.h
 * Declares the Scene class, representing the current
 * world of entities.
 */

#pragma once

#include <entt/entt.hpp>
#include <cstdint>

namespace game
{
	class Scene
	{
	private:
		//Registry of entities
		entt::registry<> registry_;

	public:
		//Ticks all logic systems in this Scene
		void tick(double dt);

		//Invokes the render systems in this Scene
		void draw();
	};
}
