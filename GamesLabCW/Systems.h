/**
 * Systems.h
 * Defines the framework and utilities for representing
 * systems as automatically-registered functions.
 */

#pragma once

#include <functional>
#include <entt/entt.hpp>

#include "Components.h"

namespace game
{
	//Function representing a system, receiving dt, entity and arbitrary components
	template <typename... Ts>
	using SystemFunction = std::function<void(double, entt::registry<>::entity_type, Ts&...)>;

	//Wrapped system function to be invoked by registry owner
	using SystemInvoker = std::function<void(double, entt::registry<>&)>;

	//Wraps a system function
	template <typename... Ts>
	SystemInvoker system_invoker(const SystemFunction<Ts...> &f)
	{
		return [f](double dt, entt::registry<> &reg)
		{
			reg.view<Ts...>().each(
				[f, dt](auto entity, auto&... params) { f(dt, entity, params...); }
			);
		};
	}

	//Base class containing collection of wrapped system function
	struct SystemRegistry
	{
		static std::vector<SystemInvoker> system_invokers;
	};

	//CRTP derived class registering system function definitions
	template <typename T, typename... Ts>
	struct System : SystemRegistry
	{
		System(const SystemFunction<Ts...> &f)
		{
			system_invokers.emplace_back(system_invoker(f));
		}
	};
}

//Registers a function as a system expecting an arbitrary number of components
#define SYSTEM(name, ...) \
	struct name##_SYS { static System<name##_SYS, __VA_ARGS__> s; }; \
	System<name##_SYS, __VA_ARGS__> name##_SYS::s ( \
	name ) \
