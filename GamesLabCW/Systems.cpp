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

		//F11 toggles fullscreen
		if (input::is_pressed(input::KEY_F11))
			events::dispatcher.trigger<events::ToggleFullscreen>();
	};
	SYSTEM(GameStateSystem, GameStateComponent);

	auto OverlaySystem = [](SceneInfo info, auto entity, OverlayComponent &o)
	{
		info.scene.destroy(entity);
	};
	SYSTEM(OverlaySystem, OverlayComponent);

	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, TransformComponent &t, KinematicComponent &k, ProjectileComponent &bc, CollisionComponent &c, StatsComponent &s)
	{
		s.mana += info.dt;
		if (s.mana > 3)
		{
			s.mana = 3;
		}
		if (s.health < 1)
		{
			info.scene.destroy(entity);
		}
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
		if (input::is_released(input::MOUSE_BUTTON_1) && s.mana >= 3)
		{
			s.mana = 0;
			events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, t.rotation, bc.vs, bc.fs, bc.particle_file, c.radius, true);
		}

		OverlayComponent i_hp; 
		OverlayComponent i_mp; 
		
		switch (s.health)
		{
		case 1:
			i_hp.texture_file = "models/UI/hearts-1.png";
			break;
		case 2:
			i_hp.texture_file = "models/UI/hearts-2.png";
			break;
		case 3:
			i_hp.texture_file = "models/UI/hearts-3.png";
			break;
		default:
			i_hp.texture_file = "models/UI/hearts-3.png";
			break;
		}

		switch (int(s.mana))
		{
		case 0:
			i_mp.texture_file = "models/UI/mana-0.png";
			break;
		case 1:
			i_mp.texture_file = "models/UI/mana-1.png";
			break;
		case 2:
			i_mp.texture_file = "models/UI/mana-2.png";
			break;
		case 3:
			i_mp.texture_file = "models/UI/mana-3.png";
			break;
		default:
			i_mp.texture_file = "models/UI/mana-3.png";
			break;
		}

		info.scene.instantiate("Overlay", i_hp);
		info.scene.instantiate("Overlay", i_mp);
	};
	SYSTEM(FirstPersonControllerSystem, FirstPersonControllerComponent, TransformComponent, KinematicComponent, ProjectileComponent, CollisionComponent, StatsComponent);

	//EXAMPLE Moveable sphere to demo collisions
	auto MoveSphereSystem = [](auto info, auto entity, auto&, TransformComponent& t)
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

			if (info.registry.has<DetectionComponent>(other) && info.registry.has<BulletComponent>(entity))
			{
				AIComponent &ai = info.registry.get<AIComponent>(other);
				BulletComponent& bc = info.registry.get<BulletComponent>(entity);

				if (ai.dodgeCooldown > ai.dodgeMax && bc.isPlayers)
				{
					DetectionComponent& d = info.registry.get<DetectionComponent>(other);
					ai.dodgeBullet = true;
					c2 = d.c; 
				}
				else
				{
					HitboxComponent& h = info.registry.get<HitboxComponent>(other);
					c2 = h.c;
				}
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
		// Animation works but is too laggy to use
		if (!m.isAnimated)
		{
			return;
		}

		static double timeSinceLastUpdate;
		static double t = 0;
		t += info.dt;

		if (timeSinceLastUpdate < 0.1)
		{
			timeSinceLastUpdate += info.dt;
			return;
		}

		timeSinceLastUpdate = 0;

		renderer::animate_model(t, m.model_file);
	};
	SYSTEM(AnimationSystem, ModelComponent);

	auto AISystem = [](SceneInfo info, Entity entity, ModelComponent &m, TransformComponent &t, AIComponent &a, ProjectileComponent &bc, StatsComponent &s, DetectionComponent &d, HitboxComponent &h, KinematicComponent &k)
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
			a.dodgeCooldown += info.dt;

			if (a.state == a.Look)
			{
				//cout << "Looking" << endl;

				a.moving += info.dt;
				if (a.moving > 7)
				{
					m.model_file = a.walk_file;
					auto r = rand() % 360;
					auto direction = Vector2(-fmod(t.rotation.z + r, 360), 0).direction_hv().ToGLM();
					Vector3 move = glm::normalize(direction);
					a.moving = 0;
					int speed = 2;
					speed *= 100;
					k.move_velocity = move * speed * info.dt;
					t.rotation.z = Vector2(-fmod(t.rotation.z + r, 360), 0).abs() + 180;
				}
				else if (a.moving > 4)
				{
					m.model_file = a.idle_file;
					k.move_velocity = { 0,0,0 };
				}
				else if (a.dodgeCooldown > 0.5 && a.dodgeBullet)
				{
					m.model_file = a.idle_file;
					k.move_velocity = { 0,0,0 };
					a.dodgeBullet = false;
				}
				
			}
			else if (a.state == a.Dodge)
			{
				m.model_file = a.walk_file;
				//cout << "Dodging" << endl;
				cout << (a.dodgeCooldown > a.dodgeMax) << endl;

				if (a.dodgeCooldown > a.dodgeMax)
				{
					k.move_velocity = { 0,0,0 };
					a.dodgeCooldown = 0;
					auto direction = Vector2(-fmod(a.direction, 360), 0).direction_hv_right().ToGLM();
					Vector3 move = glm::normalize(direction);
					a.moving = 0;
					int speed = 10;
					speed *= 100;
					auto r = rand() % 2;
					if (r == 1)
					{
						k.move_velocity = move * speed * info.dt;
					}
					else
					{
						k.move_velocity = move * speed * info.dt * -1;
					}
				}
				a.state = a.Look;

			}
			else if (a.state == a.Shoot)
			{
				//cout << "Attacking" << endl;

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
					t.rotation.y = -(360 - glm::degrees(acos(cosinedegreesToRotate))) + 180;
				else
					t.rotation.y = -(glm::degrees(acos(cosinedegreesToRotate))) + 180;

				if (s.mana > 3)
				{
					s.mana = 0;
					Vector3 rotation = { -fmod(t.rotation.z+180,360), 0, 0 };
					events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, rotation, bc.vs, bc.fs, bc.particle_file,h.c.radius, false);
					m.model_file = a.attack_file;
				}
				//a.state = a.Look;
			}
		}
		
		
	};
	SYSTEM(AISystem, ModelComponent, TransformComponent, AIComponent, ProjectileComponent, StatsComponent, DetectionComponent, HitboxComponent, KinematicComponent);
	
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
	
	//Handles collision response for kinematic bodies against solid planes
	auto SolidPlaneSystem = [](SceneInfo info, auto entity, TransformComponent &t, KinematicComponent &k, CollisionComponent &c)
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
		if (!NOCLIP && k.solid)
		{
			info.registry.view<SolidPlaneComponent>().each([&](auto entity, SolidPlaneComponent &sp)
			{
				Vector3 normal = sp.normal;
				double pos = scalar_projection(sp.position, normal) + c.radius;

				if (scalar_projection(t.position, normal) <= pos)
				{
					//Determine point of collision along surface normal
					double s1 = scalar_projection(t.position_old, normal);
					double s2 = scalar_projection(t.position, normal);
					double depth = s2 - pos;
					double fraction = (s1 - pos) / (s1 - s2);

					//Absolute distance to plane centre
					double dist = glm::length(glm::vec3(sp.position - t.position));

					//Squared distance perpendicular to plane
					double d2 = dist * dist - depth * depth;

					fraction = abs(fraction);
					if (fraction <= 1 && d2 <= sp.size * sp.size)
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
						k.move_velocity -= vector_projection(k.move_velocity, normal);
						k.velocity_old -= vector_projection(k.velocity_old, normal);

						k.velocity += k.acceleration * dt2;
						total_velocity = (k.velocity + k.velocity_old) / 2.0 + k.move_velocity;
						t.position += total_velocity * dt2;
					}
				}
			});
		}

		//Reset acceleration again
		k.acceleration = { 0, 0, 0 };
	};
	SYSTEM(SolidPlaneSystem, TransformComponent, KinematicComponent, CollisionComponent);
}

