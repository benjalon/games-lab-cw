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

	// --- Bug: This breaks other ambient lighting, i think because it doesn't increment the N_Ambient count ---
	////EXAMPLE System that uses input to turn lights on if mouse is held
	//auto LightMouseSystem = [](double dt, auto entity, auto &al)
	//{
	//	al.intensity = utility::contains(input::held, input::MOUSE_BUTTON_LEFT) ? 1.0 : 0.0;
	//};
	//SYSTEM(LightMouseSystem, AmbientLightComponent);

	auto MoveCameraSystem = [](double dt, auto entity, CameraComponent &c)
	{
		auto speed = 0.3;
		c.position.x += utility::contains(input::held, input::KEY_A) ? -speed : 0.0;
		c.position.x += utility::contains(input::held, input::KEY_D) ? speed: 0.0;
		c.position.z += utility::contains(input::held, input::KEY_S) ? speed : 0.0;
		c.position.z += utility::contains(input::held, input::KEY_W) ? -speed : 0.0;
		c.orientation.x += utility::contains(input::held, input::KEY_Q) ? speed*2 : 0.0;
		c.orientation.x += utility::contains(input::held, input::KEY_E) ? -speed*2 : 0.0;
	};
	SYSTEM(MoveCameraSystem, CameraComponent);
}
