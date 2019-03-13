/**
 * Events.h
 * Defines the interface for event signalling.
 */

#pragma once

#include <entt/entt.hpp>

namespace game::events
{
	//Global event dispatcher
	extern entt::dispatcher dispatcher;

	//Events
	struct QuitGame{};
}
