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
#include "procedural_generation/procedural_generation.h"

using namespace std;

entt::dispatcher game::events::dispatcher{};
std::vector<std::unique_ptr<game::events::ResponseBase>> game::events::responses;

namespace game::events
{
	void HandleKeyCollision(const EnterCollision &, Entity);
	void HandleDoorCollision(const EnterCollision &, Entity);
	void HandleBulletCollision(const EnterCollision &, Entity, Entity);
	void HandlePortalCollision(const EnterCollision &);
	void HandleDetectionCollision(const EnterCollision &, Entity);
	void HandleDetectionCollisionLeaving(const LeaveCollision &);
	void HandlePlayerBulletCollision(const EnterCollision&, Entity, Entity);

	void SphereEnterCollideResponse(const EnterCollision &e)
	{
		if (!e.info.registry.valid(e.a) || !e.info.registry.valid(e.b)) return;

		bool b = false;
		if (collide_which<KeyComponent, PointLightComponent, TransformComponent>(e, b) &&
			collide<FirstPersonControllerComponent>(e, b))
		{
			HandleKeyCollision(e, b ? e.a : e.b);
		}
		else if (collide_which<DoorComponent>(e, b) &&
			collide<FirstPersonControllerComponent>(e, b))
		{
			HandleDoorCollision(e, b ? e.a : e.b);
		}
		else if (collide_which<AIComponent>(e, b) &&
			collide<BulletComponent>(e, b))
		{
			HandleBulletCollision(e, b ? e.a : e.b, b ? e.b : e.a);
		}
		else if (collide_which<PortalComponent>(e, b) &&
			collide<FirstPersonControllerComponent>(e, b))
		{
			HandlePortalCollision(e);
		}
		else if (collide_which<DetectionComponent>(e, b) &&
			collide<FirstPersonControllerComponent>(e, b))
		{
			HandleDetectionCollision(e, b ? e.b : e.a);
		}
		else if (collide_which<FirstPersonControllerComponent>(e, b) &&
			collide<BulletComponent>(e, b))
		{
			HandlePlayerBulletCollision(e, b ? e.a : e.b, b ? e.b : e.a);
		}
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		if (!e.info.registry.valid(e.a) || !e.info.registry.valid(e.b)) return;

		if (e.info.registry.has<DetectionComponent>(e.b) && e.info.registry.has< FirstPersonControllerComponent>(e.a))
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
		BulletComponent b; b.isPlayers = e.isPlayers;
		e.scene.instantiate("Bullet", m, t, k, p_fireball,b);
		
	}
	RESPONSE(FireBulletResponse, FireBullet);

	int keyCount = 0;
	void HandleKeyCollision(const EnterCollision &e, Entity player)
	{
		//Return to the hub, incrementing keyCount
		keyCount++;
		e.info.scene.clear();
		procgen::load_hub(e.info.scene, keyCount);

		if (keyCount == 6)
		{
			e.info.scene.instantiate("PointLight", PointLightComponent{ {255, 215, 0}, 10.0, { -30, 12, 0 } });
		}
	}

	void HandleDoorCollision(const EnterCollision &e, Entity player)
	{
		auto &s = e.info.registry.get<StatsComponent>(player);

		if (s.keyCount > 5) 
		{
			s.gameComplete = true;
		}
	}

	void HandleBulletCollision(const EnterCollision &e, Entity bullet, Entity aic)
	{
		auto &ai = e.info.registry.get<AIComponent>(aic);
		auto &s = e.info.registry.get<StatsComponent>(aic);
		auto &bs = e.info.registry.get<BulletComponent>(bullet);

		if (ai.dodgeBullet)
		{
			auto& bt = e.info.registry.get<TransformComponent>(bullet);
			auto& at = e.info.registry.get<TransformComponent>(aic);
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
		}
		else
		{
			bs.draw = false;
			s.health -= 1;
		}
	}

	void HandleDetectionCollision(const EnterCollision &e, Entity aic)
	{
		auto &ai = e.info.registry.get<AIComponent>(aic);
		ai.state = ai.Shoot;

	}

	void HandleDetectionCollisionLeaving(const LeaveCollision &e)
	{
		auto &ai = e.info.registry.get<AIComponent>(e.b);
		ai.state = ai.Look;
	}

	void HandlePortalCollision(const EnterCollision &e)
	{
		e.info.scene.clear();
		Vector3 player_pos = procgen::generate_maze(e.info.scene, 21, 120, 3, 4, 6);

		auto player = e.info.scene.instantiate("FirstPersonController", TransformComponent{ player_pos , { 180,0,0 } }, CollisionComponent{ 6 }, KinematicComponent{ true });
		e.info.scene.camera = e.info.scene.instantiate("Camera", CameraComponent{ player });

		// Generic scene lighting
		e.info.scene.instantiate("AmbientLight", AmbientLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 0.1 });
		e.info.scene.instantiate("DirectionalLight", DirectionalLightComponent{ {0, 0, 0}, 0, {0,0,0} });
		e.info.scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 0, {0,5,0} });

		OverlayComponent i_hp; i_hp.texture_file = "models/UI/hearts-2.png";
		e.info.scene.instantiate("Overlay", i_hp);
		OverlayComponent i_mp; i_mp.texture_file = "models/UI/mana-3.png";
		e.info.scene.instantiate("Overlay", i_mp);
		OverlayComponent i_ch; i_ch.texture_file = "models/UI/crosshair.png";
		e.info.scene.instantiate("Overlay", i_ch);
	}

	//HandlePlayerBulletCollision

	void HandlePlayerBulletCollision(const EnterCollision& e, Entity bullet, Entity player)
	{
		auto& s = e.info.registry.get<StatsComponent>(player);
		auto& bs = e.info.registry.get<BulletComponent>(bullet);

		bs.draw = false;
		s.health -= 1;
	}
}
