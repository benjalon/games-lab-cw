/**
 * Systems.cpp
 * Defines the systems in the game engine.
 */

#include "Systems.h"

#include <glm/gtc/type_ptr.hpp>

#include "Input.h"
#include "Utility.h"
#include "renderer/Renderer.h"
#include <iostream>
using namespace std;


std::vector<game::systems::SystemInvoker> game::systems::system_invokers;

namespace game::systems
{
	//General game state system
	auto GameStateSystem = [](auto info, auto entity, auto &g)
	{
		//ESC quits the game
		if (input::is_pressed(input::KEY_ESCAPE))
			events::dispatcher.trigger<events::QuitGame>();
	};
	SYSTEM(GameStateSystem, GameStateComponent);


	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, CollisionComponent &c, TransformComponent &t, KinematicComponent &k, BulletComponent &bc)
	{
		double mouse_sensitivity = 5.0;
		double move_speed = 11.0;

		//Rotate using cursor offsetW
		t.rotation.x += mouse_sensitivity * input::cursor_pos.x * info.dt;
		t.rotation.y += mouse_sensitivity * input::cursor_pos.y * info.dt;

		//Clamp y rotation to avoid flipping
		t.rotation.y = std::clamp(t.rotation.y, -60.0, 60.0);

		//Move forward/backward
		k.velocity = Vector2(t.rotation.x, t.rotation.y).direction_hv() * move_speed *
			(input::is_held(input::KEY_W) - input::is_held(input::KEY_S));

		//Strafe
		k.velocity += Vector2(t.rotation.x, t.rotation.y).direction_hv_right() * move_speed *
			(input::is_held(input::KEY_D) - input::is_held(input::KEY_A));

		if (input::is_released(input::KEY_LEFT_CONTROL))
			events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, t.rotation);


		/* JUMPING SIMULATION IN ABSENCE OF COLLISIONS */
		////Acceleration due to gravity
		//k.acceleration.y = -9.8;
		////Clamp to non-neg y pos
		//t.position.y = std::max(t.position.y, 0.0);
		////Simulate solidness
		//if (t.position.y == 0.0)
		//	k.velocity.y = std::max(k.velocity.y, 0.0);
		////Jump!
		//if (input::is_pressed(input::KEY_SPACE))
		//	k.velocity.y += 4.0;
	};
	SYSTEM(FirstPersonControllerSystem, FirstPersonControllerComponent, CollisionComponent, TransformComponent, KinematicComponent, BulletComponent);


	//EXAMPLE Moveable sphere to demo collisions
	auto MoveSphereSystem = [](auto info, auto entity, auto &, TransformComponent &t)
	{
		t.position.z += 2.0 * info.dt * (input::is_held(input::KEY_X) - input::is_held(input::KEY_Z));
	};
	SYSTEM(MoveSphereSystem, MoveSphere, TransformComponent);


	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](auto info, auto entity, auto &t, auto &k)
	{
		k.velocity += k.acceleration * info.dt;
		t.position += k.velocity * info.dt;
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);


	//Updates the spatial partitioning grid
	auto SpatialGridSystem = [](SceneInfo info, auto entity, TransformComponent &t)
	{
		auto i = info.scene.spatial_grid.update(t.position, entity, t.last_index);
		t.last_index = i;
	};
	SYSTEM(SpatialGridSystem, TransformComponent);


	//Detects collisions, updating pools and logging events
	auto CollisionSystem = [](SceneInfo info, auto entity, CollisionComponent &c1, TransformComponent &t1)
	{
		//Get set of all nearby entities
		auto [begin, end] = info.scene.spatial_grid.get_cells_near(t1.position);

		//Test against potentially colliding entities
		for (auto it = begin; it != end; ++it)
		{
			Entity other = *it;

			//Ignore self
			if (other == entity) continue;

			//Ignore if not collidable
			if (!info.registry.has<CollisionComponent>(other)) continue;

			auto &[c2, t2] = info.registry.get<CollisionComponent, TransformComponent>(other);

			//Test if currently colliding (distance between centres less than sum of radii)
			double d2 = std::pow(t2.position.x - t1.position.x, 2) +
				std::pow(t2.position.y - t1.position.y, 2) +
				std::pow(t2.position.z - t1.position.z, 2);

			bool currently_colliding = d2 < std::pow(c1.radius + c2.radius, 2);
			bool was_colliding = utility::contains(c1.colliding, other);

			//Log entering of collision
			if (currently_colliding && !was_colliding)
			{
				c1.colliding.insert(other);
				c2.colliding.insert(entity);

				events::dispatcher.enqueue<events::EnterCollision>(info.registry, entity, other);
			}

			//Log leaving of collision
			if (!currently_colliding && was_colliding)
			{
				c1.colliding.erase(other);
				c2.colliding.erase(entity);

				events::dispatcher.enqueue<events::LeaveCollision>(info.registry, entity, other);
			}
		}
	};
	SYSTEM(CollisionSystem, CollisionComponent, TransformComponent);


	//Makes a camera follow its target
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent &c)
	{
		TransformComponent &t = info.scene.get<TransformComponent>(c.follow);
		c.position = t.position;
		c.orientation = { t.rotation.x, t.rotation.y };
	};
	SYSTEM(MoveCameraSystem, CameraComponent);

	//Animation system
	auto AnimationSystem = [](auto info, auto entity, auto &m)
	{
		// Currently animation isn't finished
		//if (!m.isAnimated)
		{
			return;
		}

		static double timeSinceLastUpdate;
		static double t = 0;
		t += info.dt;

		if (timeSinceLastUpdate < 0.2)
		{
			timeSinceLastUpdate += info.dt;
			return;
		}

		timeSinceLastUpdate = 0;

		renderer::animate_model(t, m.model_file);
	};
	SYSTEM(AnimationSystem, ModelComponent);

	auto AISystem = [](SceneInfo info, Entity entity, ModelComponent &m, ColourComponent &colour, TransformComponent &t, KinematicComponent &k, AIComponent &a, CameraComponent &c, CollisionComponent &col)
	{
		//goal: rotate t on the z axis.

		/*if (input::is_held(input::KEY_LEFT_CONTROL))
		{
			t.rotation.z += 1;
		}*/

		// Get the positions of both Entities
		Vector2 cameraPos = Vector2(c.position.x, c.position.z);
		Vector2 enemyPos = Vector2(t.position.x, t.position.z);
		Vector2 fromPlayerToEnemy = cameraPos - enemyPos;
		fromPlayerToEnemy = Vector2(glm::normalize(fromPlayerToEnemy.ToGLM()));


		// Get the current heading of the player (this should already be Normalized)
		glm::vec2 playerHeading = glm::radians(glm::vec2(t.rotation.x, t.rotation.z));

		// Now calculate the Dot product between the two (Normalized) vectors, playerHeading and fromPlayerToEnemy
		// Note: this process is different between 2-dimensional and 3-dimensional vectors, 
		//  so I'll just represent this process by a function
		float radiansToRotate = glm::dot(playerHeading, fromPlayerToEnemy.ToGLM());


		// Apply this rotation to the player
		//player.Rotate(radiansToRotate);
		float rotate = glm::degrees(radiansToRotate);
		float radius = 180.0;
		t.rotation.z = rotate;
		cout << t.rotation.z << ":" << radiansToRotate << ":" << glm::degrees(radiansToRotate) << endl;
		
	};
	SYSTEM(AISystem, ModelComponent, ColourComponent, TransformComponent, KinematicComponent, AIComponent, CameraComponent,CollisionComponent);


	//Key collision system
	auto KeySystem = [](auto info, auto entity, ModelComponent &m, ColourComponent &c, CollisionComponent &collision, TransformComponent &t, KeyComponent &k, PointLightComponent &pl)
	{
		if (!k.pickedUp && collision.colliding.size() > 0) {
			k.pickedUp = true;
			t.position = k.destination;

			pl.on = false;

			//k.keysHeld++;
		}
	};
	SYSTEM(KeySystem, ModelComponent, ColourComponent, CollisionComponent, TransformComponent, KeyComponent, PointLightComponent);

}
