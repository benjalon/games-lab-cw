/**
 * Systems.cpp
 * Defines the systems in the game engine.
 */

#include "Systems.h"

#include <glm/gtc/type_ptr.hpp>

#include "Input.h"
#include "Utility.h"
#include "renderer/Renderer.h"


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
		ShowCursor(FALSE);
		double screenWidth = 1920;
		double screenHeight = 1080;
		if (!utility::contains(input::held, input::KEY_ESCAPE))
		{
			//This has been hacked because of no fullscreen meaning it needs an offset.
			SetCursorPos(1935 / 2, 1145 / 2);
		}
		
		double xpos, ypos;
		float speed = 11.0f; // 11 units / second
		float mouseSpeedx = 2.0f;
		float mouseSpeedy = 2.5f;

		xpos = input::cursor_pos.x;
		ypos = input::cursor_pos.y;

		c.orientation.x += mouseSpeedx * info.dt * float(screenWidth / 2 - xpos);
		c.orientation.y += mouseSpeedy * info.dt * float(screenHeight / 2 - ypos);

		auto direction = c.orientation.direction_hv();
		direction = { direction.x, 0, direction.z };
		if (utility::contains(input::held, input::KEY_W))
		{
			c.position += direction * info.dt * speed;		}
		if (utility::contains(input::held, input::KEY_S))
		{
			c.position -= direction * info.dt * speed;
		}
		if (utility::contains(input::held, input::KEY_D))
		{
			c.position += c.orientation.direction_hv_right() *info.dt * speed;
		}
		if (utility::contains(input::held, input::KEY_A))
		{
			c.position -= c.orientation.direction_hv_right() * info.dt * speed;
		}


		//auto gravity = -9.81f;
		auto gravity = -10;

		k.velocity.y += -0.8;
		c.position.y += k.velocity.y;

		if (k.velocity.y < 0.0f) {
			k.velocity.y = 0.0f;
		}

		if (c.position.y < 2.0f) {
			c.position.y = 2.0f;
		}
			
		

		////When you want to jump, just set the velocity to a positive value :
		if (utility::contains(input::pressed, input::KEY_SPACE))
		{
			if (true)
			{
				k.velocity.y += 4.0f;
				//canjump = false;
			}
		}

		input::cursor_pos = { screenWidth / 2, screenHeight / 2 };
	};
	SYSTEM(MoveCameraSystem, CameraComponent, KinematicComponent);

	//Animation system
	auto AnimationSystem = [](auto info, auto entity, auto &m)
	{
		if (!m.hasBones)
		{
			return;
		}

		static double timeSinceLastUpdate;
		static double t = 0;
		t += info.dt;

		if (timeSinceLastUpdate < 1)
		{
			timeSinceLastUpdate += info.dt;
			return;
		}

		timeSinceLastUpdate = 0;

		std::vector<glm::mat4> Transforms;
		renderer::BoneTransform(t, Transforms, m.model_file);
	};
	SYSTEM(AnimationSystem, ModelComponent);
}
