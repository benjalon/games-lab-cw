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
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent &c, KinematicComponent &k)
	{
		//Set to correct cursor mode
		input::cursor_centre = true;

		double mouse_sensitivity = 5.0;
		double move_speed = 11.0;

		//Rotate using cursor offset
		c.orientation.x += mouse_sensitivity * input::cursor_pos.x * info.dt;
		c.orientation.y += mouse_sensitivity * input::cursor_pos.y * info.dt;

		//Move forward/backward
		c.position += c.orientation.direction_hv() * move_speed * info.dt *
			(input::is_held(input::KEY_W) - input::is_held(input::KEY_S));

		//Strafe
		c.position += c.orientation.direction_hv_right() * move_speed * info.dt *
			(input::is_held(input::KEY_D) - input::is_held(input::KEY_A));
	};
	SYSTEM(MoveCameraSystem, CameraComponent, KinematicComponent);
}
