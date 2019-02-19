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
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);

	//Allows for noclip camera control by the player
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent &c)
	{
		double linear = 18.0 * info.dt;
		double angular = 36.0 * info.dt;
		c.position.x += (input::is_held(input::KEY_D) - input::is_held(input::KEY_A)) * linear;
		c.position.z += (input::is_held(input::KEY_S) - input::is_held(input::KEY_W)) * linear;
		c.orientation.x += (input::is_held(input::KEY_Q) - input::is_held(input::KEY_E)) * angular;
	};
	SYSTEM(MoveCameraSystem, CameraComponent);
}
