/**
 * Systems.cpp
 * Defines the systems in the game engine.
 */

#include "Systems.h"

#include "Input.h"
#include "Utility.h"

std::vector<game::systems::SystemInvoker> game::systems::system_invokers;

namespace game::systems
{
	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](auto info, auto entity, auto &t, auto &k)
	{
		k.velocity += k.acceleration * info.dt;
		t.position += k.velocity * info.dt;

		std::cout << "s=" << t.position << " v=" << k.velocity << " a=" << k.acceleration << std::endl;
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);

	//Allows for noclip camera control by the player
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent &c)
	{
		double linear = 18.0 * info.dt;
		double angular = 36.0 * info.dt;
		c.position.x += (utility::contains(input::held, input::KEY_D) - utility::contains(input::held, input::KEY_A)) * linear;
		c.position.z += (utility::contains(input::held, input::KEY_S) - utility::contains(input::held, input::KEY_W)) * linear;
		c.orientation.x += (utility::contains(input::held, input::KEY_Q) - utility::contains(input::held, input::KEY_E)) * angular;
	};
	SYSTEM(MoveCameraSystem, CameraComponent);
}
