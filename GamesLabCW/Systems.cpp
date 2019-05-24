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
	auto GameStateSystem = [](auto info, auto entity, auto &g)
	{
		//ESC quits the game
		if (input::is_pressed(input::KEY_ESCAPE))
			events::dispatcher.trigger<events::QuitGame>();

		//EXAMPLE Simulate collision with portal
		if (input::is_pressed(input::KEY_ENTER))
			events::dispatcher.enqueue<events::GenerateMaze>(info.scene);
	};
	SYSTEM(GameStateSystem, GameStateComponent);


	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, CollisionComponent &c, TransformComponent &t, KinematicComponent &k, BulletComponent &bc)
	{
		double mouse_sensitivity = 5.0;
		double move_speed = 30.0;

		//Rotate using cursor offsetW
		t.rotation.x += mouse_sensitivity * input::cursor_pos.x * info.dt;
		t.rotation.y += mouse_sensitivity * input::cursor_pos.y * info.dt;

		//Clamp y rotation to avoid flipping
		t.rotation.y = std::clamp(t.rotation.y, -85.0, 85.0);

		//Move forward/backward
		Vector3 move_dir = Vector2(t.rotation.x, t.rotation.y).direction_hv() *	(input::is_held(input::KEY_W) - input::is_held(input::KEY_S));

		//Strafe
		move_dir += Vector2(t.rotation.x, t.rotation.y).direction_hv_right() * (input::is_held(input::KEY_D) - input::is_held(input::KEY_A));

		if (!NOCLIP)
		{
			//Constrain movement instruction to horizontal plane
			move_dir.y = 0;

			//Acceleration due to gravity
			k.acceleration.y -= 50;
		}

		//Apply movement instruction
		k.move_velocity = move_dir * move_speed;

		//Fire bullet
		if (input::is_released(input::MOUSE_BUTTON_1))
			events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, t.rotation);
	};
	SYSTEM(FirstPersonControllerSystem, FirstPersonControllerComponent, CollisionComponent, TransformComponent, KinematicComponent, BulletComponent);


	//EXAMPLE Moveable sphere to demo collisions
	auto MoveSphereSystem = [](auto info, auto entity, auto &, TransformComponent &t)
	{
		t.position.z += 2.0 * info.dt * (input::is_held(input::KEY_X) - input::is_held(input::KEY_Z));
	};
	SYSTEM(MoveSphereSystem, MoveSphere, TransformComponent);


	//Basic kinematic system of calculus of motion
	auto KinematicSystem = [](auto info, auto entity, TransformComponent &t, KinematicComponent &k)
	{
		//Log current as old values
		t.position_old = t.position;
		k.velocity_old = k.velocity;
		k.acceleration_old = k.acceleration;

		//Integrate acceleration and velocity (improved Euler)
		k.velocity += k.acceleration * info.dt;
		t.position += (k.velocity + k.velocity_old) / 2.0 * info.dt;

		//Simulate momentary nature of force by resetting acceleration
		k.acceleration = { 0, 0, 0 };

		//Integrate angular velocity
		t.rotation += k.angular_velocity * info.dt;
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

				events::dispatcher.enqueue<events::EnterCollision>(info, entity, other);
			}

			//Log leaving of collision
			if (!currently_colliding && was_colliding)
			{
				c1.colliding.erase(other);
				c2.colliding.erase(entity);

				events::dispatcher.enqueue<events::LeaveCollision>(info, entity, other);
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
		//cout << " X:" << t.rotation.x << " Y:" << t.rotation.y << " Z:" << t.rotation.x*t.rotation.y << endl;
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

	auto AISystem = [](SceneInfo info, Entity entity, ModelComponent &m, ColourComponent &colour, TransformComponent &t, KinematicComponent &k, AIComponent &a, CollisionComponent &col, DetectionComponent &d, BulletComponent &bc)
	{
		//Get reference to camera
		CameraComponent &c = info.scene.get<CameraComponent>(d.camera);

		//goal: rotate t on the z axis.
		
		if (a.looking)
		{
			a.looking = false;
			//move side to side. 
		}
		else if (!a.looking)
		{
			if (true)
			{
				// Get the positions of both Entities
				Vector2 cameraPos = Vector2(c.position.x, c.position.z);
				Vector2 enemyPos = Vector2(t.position.x, t.position.z);
				Vector2 fromPlayerToEnemy = cameraPos - enemyPos;
				fromPlayerToEnemy = Vector2(glm::normalize(fromPlayerToEnemy.ToGLM()));

				glm::mat4 matModel = glm::rotate(glm::radians((float)(t.rotation.z)), glm::vec3(0, 0, 1));

				// Get the current heading of the player (this should already be Normalized)
				//glm::vec2 playerHeading = glm::vec2(t.rotation.x, t.rotation.z);
				glm::vec2 playerHeading = glm::vec2(matModel[2][0], matModel[2][2]);


				// Now calculate the Dot product between the two (Normalized) vectors, playerHeading and fromPlayerToEnemy
				// Note: this process is different between 2-dimensional and 3-dimensional vectors, 
				//  so I'll just represent this process by a function
				float cosinedegreesToRotate = glm::dot(playerHeading, fromPlayerToEnemy.ToGLM());


				// Apply this rotation to the player
				//player.Rotate(radiansToRotate);
				if(fromPlayerToEnemy.x < 0)
					t.rotation.z = -(360 - glm::degrees(acos(cosinedegreesToRotate)));
				else
					t.rotation.z = -(glm::degrees(acos(cosinedegreesToRotate)));

				//t.rotation.z = rotate *-1;
				//cout << fromPlayerToEnemy.x << " " << fromPlayerToEnemy.y << " " <<  cosinedegreesToRotate << " " << t.rotation.z << endl;// ":" << fromPlayerToEnemy << ":" << degreesToRotate << endl;


				
			}
			else {
				// Get the positions of both Entities
				Vector3 cameraPos = c.position;
				Vector3 enemyPos = t.position;
				Vector3 direction = enemyPos - cameraPos;


				// Get the current heading of the player (this should already be Normalized)
				//glm::vec3 playerHeading = glm::radians(glm::vec3(t.rotation.x, t.rotation.z));
				//Vector3 playerHeading = Vector2(t.rotation.x, t.rotation.y).direction_hv();

				// Now calculate the Dot product between the two (Normalized) vectors, playerHeading and fromPlayerToEnemy
				// Note: this process is different between 2-dimensional and 3-dimensional vectors, 
				//  so I'll just represent this process by a function
				//float rotate = glm::dot(playerHeading.ToGLM(), fromPlayerToEnemy.ToGLM());

				
				//auto g = glm::lookAt(t.position.ToGLM(), glm::normalize(direction.ToGLM()) + enemyPos.ToGLM(), { 0.0f, 1.0f, 0.0f });

				//t.follow = true;
				//t.lookAt = glm::inverse(g);
				
				//auto b = glm::look
				// Apply this rotation to the player
				//player.Rotate(radiansToRotate);
				//rotate.x = glm::degrees(rotate.x);
				//rotate.y = glm::degrees(rotate.y);
				//rotate.z = glm::degrees(rotate.z);
				//float radius = 180.0;
				//t.rotation.z = rotate;
			}
			
			

			
			if (input::is_pressed(input::KEY_LEFT_CONTROL))
			{
				Vector3 tmp = { t.rotation.x,t.rotation.z,t.rotation.y }; //really shitty doesn't work
				events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, tmp);
			}
			
		}

		


		
	};
	SYSTEM(AISystem, ModelComponent, ColourComponent, TransformComponent, KinematicComponent, AIComponent, CollisionComponent,DetectionComponent,BulletComponent);
	
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

	//Handles collision response for kinematic bodies against solid planes
	auto SolidPlaneSystem = [](SceneInfo info, auto entity, TransformComponent &t, KinematicComponent &k)
	{
		//Scalar projection of a onto b
		auto scalar_projection = [](glm::vec3 a, glm::vec3 b)
		{
			return glm::dot(a, b) / glm::length(b);
		};

		//Vector projection of a onto b
		auto vector_projection = [scalar_projection](glm::vec3 a, glm::vec3 b)
		{
			return scalar_projection(a, b) * b / glm::length(b);
		};


		//Undo resetting of acceleration, for now
		k.acceleration = k.acceleration_old;

		//Add movement speed
		t.position += k.move_velocity * info.dt;

		//Calculate collision response for every solid plane
		if (!NOCLIP)
		{
			info.registry.view<SolidPlaneComponent>().each([&](auto entity, SolidPlaneComponent &sp)
			{
				Vector3 normal = sp.normal;
				double pos = scalar_projection(sp.position, normal);

				if (scalar_projection(t.position, normal) <= pos)
				{
					//Determine point of collision along surface normal
					double s1 = scalar_projection(t.position_old, normal);
					double s2 = scalar_projection(t.position, normal);
					double fraction = (s1 - pos) / (s1 - s2);

					fraction = abs(fraction);
					if (fraction <= 1)
					{
						//Integrate as usual up to collision
						double dt1 = fraction * info.dt;
						k.velocity = k.velocity_old + k.acceleration * dt1;
						Vector3 total_velocity = (k.velocity + k.velocity_old) / 2.0 + k.move_velocity;
						t.position = t.position_old + total_velocity * dt1;

						//Integrate, removing normal component, after collision
						double dt2 = info.dt - dt1;

						k.acceleration -= vector_projection(k.acceleration, normal);
						k.velocity -= vector_projection(k.velocity, normal);
						total_velocity -= vector_projection(total_velocity, normal);
						k.move_velocity -= vector_projection(k.move_velocity, normal);
						k.velocity_old -= vector_projection(k.velocity_old, normal);

						k.velocity += k.acceleration * dt2;
						t.position += total_velocity * dt2;
					}
				}
			});
		}

		//Reset acceleration again
		k.acceleration = { 0, 0, 0 };
	};
	SYSTEM(SolidPlaneSystem, TransformComponent, KinematicComponent);
}
