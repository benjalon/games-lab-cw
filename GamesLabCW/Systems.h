/**
 * Systems.h
 * Defines the framework and utilities for representing
 * systems as automatically-registered functions.
 */

#pragma once

#include <functional>
#include <entt/entt.hpp>

#include "Components.h"

namespace game::systems
{
	//Function representing a system, receiving dt, entity and arbitrary components
	template <typename... Ts>
	using SystemFunction = std::function<void(SceneInfo, entt::registry<>::entity_type, Ts&...)>;

	//Wrapped system function to be invoked by registry owner
	using SystemInvoker = std::function<void(SceneInfo, entt::registry<>&)>;

	//Wraps a system function
	template <typename... Ts>
	SystemInvoker system_invoker(const SystemFunction<Ts...> &f)
	{
		return [f](SceneInfo info, entt::registry<> &reg)
		{
			reg.view<Ts...>().each(
				[f, info](auto entity, auto&... params) { f(info, entity, params...); }
			);
		};
	}

	//Dictionary of wrapped system functions
	extern std::vector<SystemInvoker> system_invokers;

	//CRTP base class registering system function definitions
	template <typename T, typename... Ts>
	struct System
	{
		System(const SystemFunction<Ts...> &f)
		{
			system_invokers.emplace_back(system_invoker(f));
		}
	};
}

//Registers a function as a system expecting an arbitrary number of components
#define SYSTEM(name, ...) \
	struct SYS_##name { static System<SYS_##name, __VA_ARGS__> s; }; \
	System<SYS_##name, __VA_ARGS__> SYS_##name::s ( \
	name )
