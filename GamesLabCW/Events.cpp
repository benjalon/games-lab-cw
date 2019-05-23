/**
 * Events.h
 * Defines the responses used for event signalling.
 */

#include "Events.h"
#include "Components.h"
#include "procedural_generation/procedural_generation.h"

using namespace std;

entt::dispatcher game::events::dispatcher{};
std::vector<std::unique_ptr<game::events::ResponseBase>> game::events::responses;

namespace game::events
{
	void HandleKeyCollision(const EnterCollision &);
	void HandleDoorCollision(const EnterCollision &, Entity);
	void HandleBulletCollision(const EnterCollision &, Entity);

	void SphereEnterCollideResponse(const EnterCollision &e)
	{
		bool b = false;
		if (e.info.registry.has<KeyComponent, PointLightComponent, TransformComponent>(e.a) ||
			e.info.registry.has<KeyComponent, PointLightComponent, TransformComponent>(e.b))
		{
			HandleKeyCollision(e);
		}
		else if (e.info.registry.has<DoorComponent>(e.a) ||
			(b = e.info.registry.has<DoorComponent>(e.b)))
		{
			HandleDoorCollision(e, b ? e.b : e.a);
		}
		else if (e.info.registry.has<AIComponent>(e.a) ||
			(b = e.info.registry.has<AIComponent>(e.b)))
		{
			HandleBulletCollision(e, b ? e.b : e.a);
		}

		/*std::cout << "Enter: " << e.a << " " << e.b << std::endl;*/
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		//std::cout << "Leave: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereLeaveCollideResponse, LeaveCollision);

	void FireBulletResponse(const FireBullet &e)
	{
		TransformComponent t; t.scale = { 0.5, 0.5, 0.5 }; t.position = e.position; t.rotation = e.rotation;
		ModelComponent m; m.model_file = e.bullet_file;
		
		KinematicComponent k; k.velocity = Vector2(t.rotation.x, t.rotation.y).direction_hv() * 30;

		e.scene.instantiate("Bullet", m, t, k);

		ParticleComponent p_fireball; p_fireball.texture_file = "models/fire2.png"; p_fireball.respawn_count = 1;
		p_fireball.position_variation = Vector3(100, 50, 100);
		p_fireball.velocity_variation = Vector3(100, 50, 10);
		p_fireball.color_variation = Vector3(100, -0.5, 100);
		p_fireball.color_modifier = Vector3(1, 0.15, 0);
		TransformComponent t_torch; t_torch.position = e.position;
		e.scene.instantiate("ParticleEffect", p_fireball, t_torch, k);
		
	}
	RESPONSE(FireBulletResponse, FireBullet);

	int keyCount = 0;
	void HandleKeyCollision(const EnterCollision &e)
	{
		//Return to the hub, incrementing keyCount
		keyCount++;
		e.info.scene.clear();
		procgen::load_hub(e.info.scene, keyCount);
	}

	void HandleDoorCollision(const EnterCollision &e, Entity door)
	{
		if (keyCount < 6) 
		{
			return; // Not enough keys so you can't leave
		}

		// Game over happy days
	}

	void HandleBulletCollision(const EnterCollision &e, Entity bullet) {

		auto &t = e.info.registry.get<TransformComponent>(bullet);
		auto &c = e.info.registry.get<ColourComponent>(bullet);

		c.colour = {255,0,0};
		
	}

	void GenerateMazeResponse(const GenerateMaze &e)
	{
		e.scene.clear();
		Vector3 player_pos = procgen::generate_maze(e.scene, 21, 120, 3, 4, 6);

		auto player = e.scene.instantiate("FirstPersonController", TransformComponent{ player_pos , { 180,0,0 } });
		auto camera = e.scene.instantiate("Camera", CameraComponent{ player });

		// Generic scene lighting
		e.scene.instantiate("AmbientLight", AmbientLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 0.2 });
		e.scene.instantiate("DirectionalLight", DirectionalLightComponent{ {0, 0, 0}, 0, {0,0,0} });
		e.scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 1, {0,5,0} });
	}
	RESPONSE(GenerateMazeResponse, GenerateMaze);
}
