/**
 * Events.h
 * Defines the responses used for event signalling.
 */
#define GLM_ENABLE_EXPERIMENTAL

#include "Events.h"
#include "Components.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;

entt::dispatcher game::events::dispatcher{};
std::vector<std::unique_ptr<game::events::ResponseBase>> game::events::responses;

namespace game::events
{
	void SphereEnterCollideResponse(const EnterCollision &e)
	{
		if (!e.registry.valid(e.a) || !e.registry.valid(e.b)) return;

		if (e.registry.has<KeyComponent, PointLightComponent, TransformComponent>(e.a))
		{
			HandleKeyCollision(e);
		}
		else if (e.registry.has<DoorComponent>(e.a))
		{
			HandleDoorCollision(e);
		}
		else if (e.registry.has<DetectionComponent>(e.a) && e.registry.has< FirstPersonControllerComponent>(e.b))
		{
			HandleDetectionCollision(e);
		}
		else if (e.registry.has<AIComponent>(e.b) && e.registry.has<BulletComponent>(e.a)) 
		{
			HandleBulletCollision(e);
		}

		/*std::cout << "Enter: " << e.a << " " << e.b << std::endl;*/
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		if (!e.registry.valid(e.a) || !e.registry.valid(e.b)) return;

		if (e.registry.has<DetectionComponent>(e.b) && e.registry.has< FirstPersonControllerComponent>(e.a))
		{
			HandleDetectionCollisionLeaving(e);
		}
		//std::cout << "Leave: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereLeaveCollideResponse, LeaveCollision);

	void FireBulletResponse(const FireBullet &e)
	{
		TransformComponent t; t.scale = { 0.5, 0.5, 0.5 }; t.position = e.position; t.rotation = e.rotation;
		ModelComponent m; m.model_file = e.bullet_file; m.vertex_shader = e.bullet_vs; m.fragment_shader = e.bullet_fs;
		
		KinematicComponent k; k.velocity = Vector2(t.rotation.x, t.rotation.y).direction_hv() * 30;

		auto vel = glm::normalize(k.velocity.ToGLM());
		double x = e.radius * 2.5;
		glm::vec3 vel2 = { x,x,x };
		t.position = Vector3(t.position.ToGLM() + vel * vel2);
		//k.velocity = { 0,0,0 };


		ParticleComponent p_fireball; p_fireball.texture_file = e.bullet_particles; p_fireball.respawn_count = 1;

		p_fireball.position_variation = Vector3(100, 50, 100);
		p_fireball.velocity_variation = Vector3(100, 50, 10);
		p_fireball.color_variation = Vector3(100, -0.5, 100);
		p_fireball.color_modifier = Vector3(1, 0.15, 0);
		e.scene.instantiate("Bullet", m, t, k, p_fireball);
		
	}
	RESPONSE(FireBulletResponse, FireBullet);

	int keyCount = 0;
	void HandleKeyCollision(const EnterCollision &e)
	{
		auto &[k, pl, t] = e.registry.get<KeyComponent, PointLightComponent, TransformComponent>(e.a);

		if (k.pickedUp)
		{
			return; // Already handled, ignore
		}

		// Register the key, move it to the door and remove the point light from it
		k.pickedUp = true;
		pl.on = false;
		t.position = k.destination;

		// Track win condition
		auto &s = e.registry.get<StatsComponent>(e.b);
		s.keyCount++;
	}

	void HandleDoorCollision(const EnterCollision &e)
	{
		auto &s = e.registry.get<StatsComponent>(e.b);

		if (s.keyCount < 6) 
		{
			return; // Not enough keys so you can't leave
		}
		else
		{
			//open door/end game here
			
		}
	}

	void HandleBulletCollision(const EnterCollision &e) 
	{
		auto &ai = e.registry.get<AIComponent>(e.b);
		auto &s = e.registry.get<StatsComponent>(e.b);
		auto &bs = e.registry.get<BulletComponent>(e.a);

		if (ai.dodgeBullet)
		{
			auto& bt = e.registry.get<TransformComponent>(e.a);
			auto& at = e.registry.get<TransformComponent>(e.b);
			Vector2 cameraPos = Vector2(bt.position.x, bt.position.z);
			Vector2 enemyPos = Vector2(at.position.x, at.position.z);
			Vector2 nonNormal = cameraPos - enemyPos;
			Vector2 fromPlayerToEnemy = Vector2(glm::normalize(nonNormal.ToGLM()));
			glm::mat4 matModel = glm::rotate(glm::radians((float)(at.rotation.z)), glm::vec3(0, 0, 1));
			glm::vec2 playerHeading = glm::vec2(matModel[2][0], matModel[2][2]);
			float cosinedegreesToRotate = glm::dot(playerHeading, fromPlayerToEnemy.ToGLM());

			if (fromPlayerToEnemy.x < 0)
				ai.direction = -(360 - glm::degrees(acos(cosinedegreesToRotate))) + 180;
			else
				ai.direction = -(glm::degrees(acos(cosinedegreesToRotate))) + 180;

			ai.state = ai.Dodge;
			//ai.dodgeBullet = false;
		}
		else
		{
			bs.draw = false;
			s.health -= 1;
		}
	}

	void HandleDetectionCollision(const EnterCollision &e)
	{
		auto &ai = e.registry.get<AIComponent>(e.a);
		ai.state = ai.Shoot;

	}

	void HandleDetectionCollisionLeaving(const LeaveCollision &e)
	{
		auto &ai = e.registry.get<AIComponent>(e.b);
		ai.state = ai.Look;
	}
}
