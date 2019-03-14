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
	//General game state system
	auto GameStateSystem = [](auto info, auto entity, GameStateComponent &g)
	{
		//ESC quits the game
		if (input::is_pressed(input::KEY_ESCAPE))
			events::dispatcher.trigger<events::QuitGame>();
	};
	SYSTEM(GameStateSystem, GameStateComponent);


	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](auto info, auto entity, auto &t, auto &k)
	{
		k.velocity += k.acceleration * info.dt;
		t.position += k.velocity * info.dt;
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);


	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, TransformComponent &t, KinematicComponent &k)
	{
		double mouse_sensitivity = 5.0;
		double move_speed = 11.0;

		//Rotate using cursor offset
		t.rotation.x += mouse_sensitivity * input::cursor_pos.x * info.dt;
		t.rotation.y += mouse_sensitivity * input::cursor_pos.y * info.dt;

		//Clamp y rotation to avoid flipping
		t.rotation.y = std::clamp(t.rotation.y, -60.0, 60.0);

		//Move forward/backward
		t.position += Vector2(t.rotation.x, t.rotation.y).direction_hv() * move_speed * info.dt *
			(input::is_held(input::KEY_W) - input::is_held(input::KEY_S));

		//Strafe
		t.position += Vector2(t.rotation.x, t.rotation.y).direction_hv_right() * move_speed * info.dt *
			(input::is_held(input::KEY_D) - input::is_held(input::KEY_A));
	};
	SYSTEM(FirstPersonControllerSystem, FirstPersonControllerComponent, TransformComponent, KinematicComponent);


	//Makes a camera follow its target
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent &c)
	{
		TransformComponent &t = info.scene.get<TransformComponent>(c.follow);
		c.position = t.position;
		c.orientation = { t.rotation.x, t.rotation.y };
		std::cout << c.orientation.y << std::endl;
	};
	SYSTEM(MoveCameraSystem, CameraComponent);
}
