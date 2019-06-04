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

	const int MAX_MANA = 3;
	const int MIN_HEALTH = 1;
	const double Y_AXIS_CLAMP = 85.0;
	const double MOUSE_SENSITIVITY = 5.0;
	//First-person control by the player
	auto FirstPersonControllerSystem = [](SceneInfo info, auto entity, FirstPersonControllerComponent &f, TransformComponent &t, KinematicComponent &k, ProjectileComponent &bc, CollisionComponent &c, StatsComponent &s)
	{
		// Game completion states
		if (s.health < MIN_HEALTH)
		{
			OverlayComponent i_lose;
			i_lose.texture_file = "models/UI/lose.png";
			info.scene.instantiate("Overlay", i_lose);
			return;
		}
		if (s.gameComplete)
		{
			OverlayComponent i_win;
			i_win.texture_file = "models/UI/win.png";
			info.scene.instantiate("Overlay", i_win);
			return;
		}

		s.mana += info.dt * 2;

		if (s.mana > MAX_MANA)
		{
			s.mana = MAX_MANA;
		}

		//Rotate using cursor offsetW
		t.rotation.x += MOUSE_SENSITIVITY * input::cursor_pos.x * info.dt;
		t.rotation.y += MOUSE_SENSITIVITY * input::cursor_pos.y * info.dt;

		//Clamp y rotation to avoid flipping
		t.rotation.y = std::clamp(t.rotation.y, -Y_AXIS_CLAMP, Y_AXIS_CLAMP);
		

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
		k.move_velocity = move_dir * f.moveSpeed;

		//Fire bullet
		if (input::is_released(input::MOUSE_BUTTON_1) && s.mana >= MAX_MANA)
		{
			s.mana = 0;
			events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, t.rotation, bc.vs, bc.fs, bc.particle_file, c.radius, true);
		}

		OverlayComponent i_hp; 
		OverlayComponent i_mp;
		OverlayComponent i_ch;
		
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

		i_ch.texture_file = "models/UI/crosshair.png";

		info.scene.instantiate("Overlay", i_hp);
		info.scene.instantiate("Overlay", i_mp);
		info.scene.instantiate("Overlay", i_ch);

		// DEBUGGING FOR DEMO - increase key point
		if (input::is_pressed(input::KEY_F1))
		{
			s.keyCount++;

			if (s.keyCount == 6)
			{
				info.scene.instantiate("PointLight", PointLightComponent{ {255, 215, 0}, 10.0, { -30, 12, 0 } });
			}
		}
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

			bool detect = false;
			bool hitbox = false;
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
					c2 = d.c; 
					detect = true;
				}
				else
				{
					HitboxComponent& h = info.registry.get<HitboxComponent>(other);
					c2 = h.c;
					hitbox = true;
					
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
				if (info.registry.has<AIComponent>(other))
				{
					AIComponent &enterAI = info.registry.get<AIComponent>(other);
					enterAI.hitBox = hitbox;
					enterAI.dodgeBullet = detect;
				}

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
	const float ANIMATION_DELAY = 0.05;
	auto AnimationSystem = [](auto info, auto entity, auto& m, auto& ai)
	{
		if (!m.isAnimated)
		{
			return;
		}

		ai.t += info.dt;

		if (ai.timeSinceLastUpdate < ANIMATION_DELAY)
		{
			ai.timeSinceLastUpdate += info.dt;
			return;
		}

		ai.timeSinceLastUpdate = 0;

		renderer::animate_model(ai.t, m.model_file);
	};
	SYSTEM(AnimationSystem, ModelComponent, AIComponent);

	const int MAX_WAITING_TIME = 7;
	const int MAX_MOVING_TIME = 4;
	const double MAX_DODGE_TIME = 0.5;
	const int WALK_SPEED = 300;
	const int DODGE_SPEED = 1000;
	const int ATTACK_ANIMATION_DURATION = 1;
	const int HIT_ANIMATION_DURATION = 0.6;
	auto AISystem = [](SceneInfo info, Entity entity, ModelComponent &m, TransformComponent &t, AIComponent &a, ProjectileComponent &bc, StatsComponent &s, DetectionComponent &d, HitboxComponent &h, KinematicComponent &k)
	{
		//Get reference to camera
		CameraComponent &c = info.scene.get<CameraComponent>(d.camera);
		s.mana += info.dt;
		a.animationTime += info.dt;

		if (s.health < 1)
		{
			info.scene.destroy(entity);
		}
		else if (a.isHit)
		{
			a.animationTime = 0;
			m.model_file = a.get_hit_file;
			a.isHit = false;
		}
		else if (a.animationTime > HIT_ANIMATION_DURATION)
		{
			a.dodgeCooldown += info.dt;

			if (a.state == a.Look)
			{
				a.moving += info.dt;
				if (a.moving > MAX_WAITING_TIME)
				{
					m.model_file = a.walk_file;
					auto r = rand() % 360;
					auto direction = Vector2(fmod(t.rotation.y + r, 360), 0).direction_hv().ToGLM();
					Vector3 move = glm::normalize(direction);
					a.moving = 0;
					k.move_velocity = move * WALK_SPEED * info.dt;
					t.rotation.y = Vector2(fmod(t.rotation.y + r, 360), 0).abs();
				}
				else if (a.moving > MAX_MOVING_TIME)
				{
					m.model_file = a.idle_file;
					k.move_velocity = { 0,0,0 };
				}
				else if (a.dodgeCooldown > MAX_DODGE_TIME && a.dodgeBullet)
				{
					m.model_file = a.idle_file;
					k.move_velocity = { 0,0,0 };
					a.dodgeBullet = false;
				}
				
			}
			else if (a.state == a.Dodge)
			{
				m.model_file = a.walk_file;
				cout << (a.dodgeCooldown > a.dodgeMax) << endl;

				if (a.dodgeCooldown > a.dodgeMax)
				{
					k.move_velocity = { 0,0,0 };
					a.dodgeCooldown = 0;
					auto direction = Vector2(-fmod(a.direction, 360), 0).direction_hv_right().ToGLM();
					Vector3 move = glm::normalize(direction);
					a.moving = 0;
					auto r = rand() % 2;
					if (r == 1)
					{
						k.move_velocity = move * DODGE_SPEED * info.dt;
					}
					else
					{
						k.move_velocity = move * DODGE_SPEED * info.dt * -1;
					}
				}
				a.state = a.Look;

			}
			else if (a.state == a.Shoot)
			{
				//animationTime += info.dt;
				k.move_velocity = { 0,0,0 };

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
					t.rotation.y = (360 - glm::degrees(acos(cosinedegreesToRotate)));
				else
					t.rotation.y = (glm::degrees(acos(cosinedegreesToRotate)));

				if (s.mana > MAX_MANA)
				{
					s.mana = 0;
					Vector3 rotation = { fmod(t.rotation.y,360), 0, 0 };
					//events::dispatcher.enqueue<events::FireBullet>(info.scene, bc.model_file, t.position, rotation, bc.vs, bc.fs, bc.particle_file,h.c.radius, false);
					m.model_file = a.attack_file;
					a.animationTime = 0;
				}

				if (a.animationTime > ATTACK_ANIMATION_DURATION)
				{
					m.model_file = a.idle_file;
				}
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

		renderer::update_particle(info.dt, p.texture_file, p.respawn_count, randomPosition, randomVelocity, randomColor);
	};
	SYSTEM(ParticleSystem, ParticleComponent, ColourComponent, TransformComponent, KinematicComponent);

	const int BULLET_TIMEOUT = 5;
	auto BulletSystem = [](auto info, auto entity, BulletComponent &b) {

		b.timeAlive += info.dt;
		if (!b.draw || b.timeAlive > BULLET_TIMEOUT)
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

