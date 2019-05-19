/**
 * Events.h
 * Defines the interface for event signalling.
 */
#pragma once

#include <entt/entt.hpp>

#include "Scene.h"

namespace game::events
{
	/* CUSTOM-DEFINED EVENTS */

	//Instructs the game engine to quit
	struct QuitGame {};

	//Represents two entities entering a collision
	struct EnterCollision
	{
		entt::registry<> &registry;
		Entity a;
		Entity b;
	};

	//Represents two entities leaving a collision
	struct LeaveCollision
	{
		entt::registry<> &registry;
		Entity a;
		Entity b;
	};


	struct FireBullet
	{
		Scene &scene;
		std::string bullet_file;
		Vector3 position;
		Vector3 rotation;
	};


	/* EVENTS AND RESPONSES IMPLEMENTATION */

	//Global event dispatcher
	extern entt::dispatcher dispatcher;

	//Base class for responses
	struct ResponseBase
	{
		virtual void log() = 0;
	};

	//Collection of responses ready for logging
	extern std::vector<std::unique_ptr<ResponseBase>> responses;

	//Derived class registering themselves with the event dispatcher
	template <typename T, auto Function>
	struct Response : ResponseBase
	{
		void log() override
		{
			dispatcher.sink<T>().connect<Function>();
		}
	};

	//Auxiliary class to create responses ready for logging
	template <typename T, auto Function>
	struct ResponseLogger
	{
		ResponseLogger()
		{
			responses.push_back(std::make_unique<Response<T, Function>>());
		}
	};
		
	void HandleKeyCollision(const EnterCollision &e);

	void HandleBulletCollision(const EnterCollision &e);
}

//Registers a function as a response to an event
#define RESPONSE(name, ev) \
	ResponseLogger<ev, &name> RES_##name
