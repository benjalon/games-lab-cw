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

		//Base case variadic template for instantiation
		template <typename T>
		Entity instantiate_comps(std::string p, T c)
		{
			auto e = instantiate(p);
			registry_.replace<T>(e, c);
			return e;
		}

		//Recursive case variadic template for instantiation
		template <typename T, typename... Ts>
		Entity instantiate_comps(std::string p, T c, Ts... cs)
		{
			auto e = instantiate_comps(p, cs...);
			registry_.replace<T>(e, c);
			return e;
		}

	public:
		//Ticks all logic systems in this Scene
		void tick(double dt);

		//Invokes the render systems in this Scene
		void draw();

		//Default-instantiates an entity of the given prototype
		Entity instantiate(std::string p);

		//Instantiates an entity of the given prototype with the given component values
		template <typename... Ts>
		Entity instantiate(std::string p, Ts... components)
		{
			return instantiate_comps(p, components...);
		}
	};
}
