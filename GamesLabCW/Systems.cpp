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
		//ShowCursor(FALSE);
		SetCursorPos(1920 / 2, 1080 / 2);

		double linear = 18.0 * info.dt;
		double angular = 36.0 * info.dt;
		
		double xpos, ypos;
		float horizontalAngle = 3.14f;
		// vertical angle : 0, look at the horizon
		float verticalAngle = 0.0f;
		float speed = 3.0f; // 3 units / second
		float mouseSpeed = 0.4f;

		xpos = input::cursor_pos.x;
		ypos = input::cursor_pos.y;

		c.orientation.x += mouseSpeed * info.dt * float(1920 / 2 - xpos);
		c.orientation.y += mouseSpeed * info.dt * float(1080 / 2 - ypos);

		if (utility::contains(input::held, input::KEY_W))
		{
			c.position += c.orientation.direction_hv() *info.dt * speed;
		}
		if (utility::contains(input::held, input::KEY_S))
		{
			c.position -= c.orientation.direction_hv() * info.dt * speed;
		}
		if (utility::contains(input::held, input::KEY_D))
		{
			c.position += c.orientation.direction_hv_right() *info.dt * speed;
		}
		if (utility::contains(input::held, input::KEY_A))
		{
			c.position -= c.orientation.direction_hv_right() * info.dt * speed;
		}

		input::cursor_pos = { 1920 / 2, 1080 / 2 };
	};
	SYSTEM(MoveCameraSystem, CameraComponent);
}
