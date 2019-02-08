/**
 * Prototypes.h
 * Defines the framework and utilities for representing
 * entity prototypes.
 */

#pragma once

#include "GameEngine.h"

namespace game::prototypes
{
	//Dictionary of entity prototypes
	extern std::unordered_map<std::string, entt::prototype<Entity>> prototypes;

	//Invokes registration of all prototypes
	void register_prototypes();

	//Registers a single prototype of the given name with the component types
	template <typename... Ts>
	void register_prototype(std::string name)
	{
		static entt::registry<> dummy_reg;
		auto &p = prototypes.emplace(name, dummy_reg).first->second;
		(p.set<Ts>(), ...);
	}
}

//Registers a prototype of the given name with component types
#define PROTOTYPE(name, ...) \
	register_prototype<__VA_ARGS__>(#name)
