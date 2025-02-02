/**
 * Scene.h
 * Declares the Scene class, representing the current
 * world of entities.
 */

#pragma once

#include <entt/entt.hpp>

#include "SpatialGrid.h"

namespace game
{
	//Numerical type representing individual entities
	using Entity = entt::registry<>::entity_type;

	class Scene
	{
	private:
		//Registry of entities
		entt::registry<> registry_;

		//Has this scene been drawn yet after the last clear?
		bool drawn_yet = false;

	public:
		//Spatial partitioning grid of entities
		SpatialGrid<Entity> spatial_grid;

		//Default constructor
		Scene();

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

		//Creates a custom new entity using the given components
		template <typename... Ts>
		Entity create(Ts... components)
		{
			auto e = registry_.create();
			(registry_.assign_or_replace<Ts>(e, components), ...);
			return e;
		}

		//Adds the given component to the given entity in this scene
		template <typename T>
		void add(Entity e, T component)
		{
			registry_.assign_or_replace<T>(e, component);
		}

		//Gets the given component of the given entity
		template <typename T>
		T &get(Entity e)
		{
			return registry_.get<T>(e);
		}

		//Destroys the given entity
		void destroy(Entity e);

		//Clears the entire scene, destroying all entities
		void clear();
	};


	//Struct of general-purpose information to be passed to systems, events etc.
	struct SceneInfo
	{
		//Reference to the current scene
		Scene &scene;

		//Delta time (seconds since last update)
		double dt;

		//The registry in use
		entt::registry<> &registry;
	};
}
