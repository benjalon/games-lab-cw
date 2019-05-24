/**
 * Systems.cpp
 * Defines the systems in the game engine.
 */

#include "Systems.h"

#define GLM_ENABLE_EXPERIMENTAL


#include "Input.h"
#include "Utility.h"
#include "renderer/Renderer.h"
#include <iostream>
using namespace std;
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<game::systems::SystemInvoker> game::systems::system_invokers;

namespace game::systems
{
	//General game state system
	auto GameStateSystem = [](auto info, auto entity, auto& g)
	{
		//ESC quits the game
		if (input::is_pressed(input::KEY_ESCAPE))
			events::dispatcher.trigger<events::QuitGame>();
	};
	SYSTEM(GameStateSystem, GameStateComponent);

	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, TransformComponent &t, KinematicComponent &k, ProjectileComponent &bc, CollisionComponent &c)
	{
		//if (s.health < 1)
		//{
		//	info.scene.destroy(entity);
		//}
		double mouse_sensitivity = 5.0;
		double move_speed = 11.0;

		//Rotate using cursor offsetW
		t.rotation.x += mouse_sensitivity * input::cursor_pos.x * info.dt;
		t.rotation.y += mouse_sensitivity * input::cursor_pos.y * info.dt;

		//Clamp y rotation to avoid flipping
		t.rotation.y = std::clamp(t.rotation.y, -85.0, 85.0);
		

		//Move forward/backward
		k.velocity = Vector2(t.rotation.x, t.rotation.y).direction_hv() * move_speed *
			(input::is_held(input::KEY_W) - input::is_held(input::KEY_S));

		//Strafe
		k.velocity += Vector2(t.rotation.x, t.rotation.y).direction_hv_right() * move_speed *
			(input::is_held(input::KEY_D) - input::is_held(input::KEY_A));

		if (input::is_released(input::MOUSE_BUTTON_1))
			events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, t.rotation, bc.vs, bc.fs, bc.particle_file, c.radius);

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
	SYSTEM(FirstPersonControllerSystem, FirstPersonControllerComponent, TransformComponent, KinematicComponent, ProjectileComponent, CollisionComponent);

	//EXAMPLE Moveable sphere to demo collisions
	auto MoveSphereSystem = [](auto info, auto entity, auto&, TransformComponent& t)
	{
		t.position.z += 2.0 * info.dt * (input::is_held(input::KEY_X) - input::is_held(input::KEY_Z));
	};
	SYSTEM(MoveSphereSystem, MoveSphere, TransformComponent);

	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](auto info, auto entity, auto& t, auto& k)
	{
		k.velocity += k.acceleration * info.dt;
		t.position += k.velocity * info.dt;
	};
	SYSTEM(KinematicSystem, TransformComponent, KinematicComponent);

	//Updates the spatial partitioning grid
	auto SpatialGridSystem = [](SceneInfo info, auto entity, TransformComponent &t, CollisionComponent &)
	{
		auto i = info.scene.spatial_grid.update(t.position, entity, t.last_index);
		t.last_index = i;
	};
	SYSTEM(SpatialGridSystem, TransformComponent, CollisionComponent);

	//Detects collisions, updating pools and logging events
	auto CollisionSystem = [](SceneInfo info, auto entity, CollisionComponent& c1, TransformComponent& t1)
	{
		//Get set of all nearby entities
		auto [begin, end] = info.scene.spatial_grid.get_cells_near(t1.position);

		//Test against potentially colliding entities
		for (auto it = begin; it != end; ++it)
		{
			Entity other = *it;

			if (!info.registry.valid(entity) || !info.registry.valid(other)) continue;

			//Ignore self
			if (other == entity) continue;

			//Ignore if not collidable
			if (!info.registry.has<CollisionComponent>(other)) continue;

			auto& [c2, t2] = info.registry.get<CollisionComponent, TransformComponent>(other);

			if (info.registry.has<DetectionComponent>(other) && info.registry.has<FirstPersonControllerComponent>(entity))
			{
				DetectionComponent& d = info.registry.get<DetectionComponent>(other);
				c2 = d.c;
				t2 = info.registry.get<TransformComponent>(other);
			}

			if (info.registry.has<HitboxComponent>(other) && info.registry.has<BulletComponent>(entity))
			{
				HitboxComponent& h = info.registry.get<HitboxComponent>(other);
				c2 = h.c;
				t2 = info.registry.get<TransformComponent>(other);
			}

			//Test if currently colliding (distance between centres less than sum of radii)
			double d2 = sqrt(std::pow(t2.position.x - t1.position.x, 2) +
				std::pow(t2.position.y - t1.position.y, 2) +
				std::pow(t2.position.z - t1.position.z, 2));

			double sumRad = c1.radius + c2.radius;
			bool currently_colliding = d2 < sumRad;



			bool was_colliding = utility::contains(c1.colliding, other);

			//Log entering of collision
			if (currently_colliding && !was_colliding)
			{
				c1.colliding.insert(other);
				c2.colliding.insert(entity);
				//cout << "d2:" << d2 << " sumRad:" << sumRad << endl;
				events::dispatcher.enqueue<events::EnterCollision>(info.registry, entity, other);
			}

			//Log leaving of collision
			if (!currently_colliding && was_colliding)
			{
				c1.colliding.erase(other);
				c2.colliding.erase(entity);
				//cout << "d2:" << d2 << " sumRad:" << sumRad << endl;
				events::dispatcher.enqueue<events::LeaveCollision>(info.registry, entity, other);
			}
		}
	};
	SYSTEM(CollisionSystem, CollisionComponent, TransformComponent);

	//Makes a camera follow its target
	auto MoveCameraSystem = [](SceneInfo info, auto entity, CameraComponent& c)
	{
		if (info.registry.valid(c.follow))
		{
			TransformComponent& t = info.scene.get<TransformComponent>(c.follow);
			c.position = t.position;
			c.orientation = { t.rotation.x, t.rotation.y };
		}
	};
	SYSTEM(MoveCameraSystem, CameraComponent);

	//Animation system
	auto AnimationSystem = [](auto info, auto entity, auto& m)
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

	auto AISystem = [](SceneInfo info, Entity entity, TransformComponent &t, AIComponent &a, ProjectileComponent &bc, StatsComponent &s, DetectionComponent &d, HitboxComponent &h)
	{
		//Get reference to camera
		CameraComponent &c = info.scene.get<CameraComponent>(d.camera);
		s.mana += info.dt;

		if (s.health < 1)
		{
			info.scene.destroy(entity);
		}
		else
		{
			
			if (a.looking)
			{

				//move side to side. 
			}
			else if (!a.looking)
			{
				// Get the positions of both Entities
				Vector2 cameraPos = Vector2(c.position.x, c.position.z);
				Vector2 enemyPos = Vector2(t.position.x, t.position.z);
				Vector2 nonNormal = cameraPos - enemyPos;
				Vector2 fromPlayerToEnemy = Vector2(glm::normalize(nonNormal.ToGLM()));

		
				glm::mat4 matModel = glm::rotate(glm::radians((float)(t.rotation.z)), glm::vec3(0, 0, 1));
				glm::vec2 playerHeading = glm::vec2(matModel[2][0], matModel[2][2]);
				float cosinedegreesToRotate = glm::dot(playerHeading, fromPlayerToEnemy.ToGLM());


				// Apply acos to that value and set z-axis 360 rule. Then rotation to the AIModel
				if (fromPlayerToEnemy.x < 0)
					t.rotation.z = -(360 - glm::degrees(acos(cosinedegreesToRotate))) + 180;
				else
					t.rotation.z = -(glm::degrees(acos(cosinedegreesToRotate))) + 180;

				cout << s.mana << " : " << info.dt << endl;


				//if (input::is_pressed(input::KEY_LEFT_CONTROL))
				if (s.mana > 3)
				{
					s.mana = 0;
					Vector3 rotation = { -fmod(t.rotation.z+180,360), 0, 0 };
					// bc.model_file, t.position, t.rotation, bc.vs, bc.fs, bc.particle_file
					events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, rotation, bc.vs, bc.fs, bc.particle_file,h.c.radius);
				}
			}
		}
		
		
	};
	SYSTEM(AISystem, TransformComponent, AIComponent,ProjectileComponent, StatsComponent, DetectionComponent, HitboxComponent);
	
	auto ParticleSystem = [](auto info, auto entity, ParticleComponent &p, ColourComponent &c, TransformComponent &t, KinematicComponent &k)
	{
		Vector3 randomPosition = Vector3(
			((fmod(rand(), p.position_variation.x)) - p.position_variation.y) / p.position_variation.z,
			((fmod(rand(), p.position_variation.x)) - p.position_variation.y) / p.position_variation.z,
			((fmod(rand(), p.position_variation.x)) - p.position_variation.y) / p.position_variation.z);

		Vector3 randomVelocity = Vector3(
			((fmod(rand(), p.velocity_variation.x)) - p.velocity_variation.y) / p.velocity_variation.z,
			((fmod(rand(), p.velocity_variation.x)) - p.velocity_variation.y) / p.velocity_variation.z,
			((fmod(rand(), p.velocity_variation.x)) - p.velocity_variation.y) / p.velocity_variation.z);

		Vector3 randomColor = Vector3(
			(((fmod(rand(), p.color_variation.x)) - p.color_variation.y) / p.color_variation.z) * p.color_modifier.x,
			(((fmod(rand(), p.color_variation.x)) - p.color_variation.y) / p.color_variation.z) * p.color_modifier.y,
			(((fmod(rand(), p.color_variation.x)) - p.color_variation.y) / p.color_variation.z) * p.color_modifier.z);

		renderer::update_particle(info.dt, p.texture_file, p.respawn_count, randomPosition, randomVelocity, randomColor);// , t.position);
	};
	SYSTEM(ParticleSystem, ParticleComponent, ColourComponent, TransformComponent, KinematicComponent);

	auto BulletSystem = [](auto info, auto entity, BulletComponent &b) {

		b.timeAlive += info.dt;
		if (!b.draw || b.timeAlive > 5)
		{
			info.scene.destroy(entity);
		}

	};
	SYSTEM(BulletSystem, BulletComponent);
}

