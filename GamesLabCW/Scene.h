/**
 * Scene.h
 * Declares the Scene class, representing the current
 * world of entities.
 */

#pragma once

#include <entt/entt.hpp>

namespace game
{
	//Numerical type representing individual entities
	using Entity = entt::registry<>::entity_type;

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

		//Default-instantiates an entity of the given prototype
		Entity instantiate(std::string p) { return instantiate({ p }); }

		//Default-instantiates an entity of the given prototypes
		Entity instantiate(std::initializer_list<std::string> p);

		//Instantiates an entity of the given prototype with the given component values
		template <typename... Ts>
		Entity instantiate(std::string p, Ts... components) { return instantiate({ p }, components...); }

		//Instantiates an entity of the given prototypes with the given component values
		template <typename... Ts>
		Entity instantiate(std::initializer_list<std::string> p, Ts... components)
		{
			auto e = instantiate(p);
			(registry_.replace<Ts>(e, components), ...);
			return e;
		}
	};
}
