/**
 * Systems.cpp
 * Defines the systems in the game engine.
 */

#include "Systems.h"

namespace game::systems
{
	std::vector<SystemInvoker> system_invokers;

	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](double dt, auto entity, auto &t, auto &k)
	{
		k.velocity += k.acceleration * dt;
		t.position += k.velocity * dt;

		std::cout << "s=" << t.position << " v=" << k.velocity << " a=" << k.acceleration << std::endl;
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);

	//Prints the names of all named entities
	auto NameSystem = [](double dt, auto entity, auto &n)
	{
		std::cout << n.name << std::endl;
	};
	SYSTEM(NameSystem, NameComponent);
}
