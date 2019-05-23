/**
 * Events.h
 * Defines the responses used for event signalling.
 */

#include "Events.h"
#include "Components.h"
using namespace std;

entt::dispatcher game::events::dispatcher{};
std::vector<std::unique_ptr<game::events::ResponseBase>> game::events::responses;

namespace game::events
{
	void SphereEnterCollideResponse(const EnterCollision &e)
	{
		if (e.registry.has<KeyComponent, PointLightComponent, TransformComponent>(e.a))
		{
			HandleKeyCollision(e);
		}
		else if (e.registry.has<DoorComponent>(e.a))
		{
			HandleDoorCollision(e);
		}
		else if (e.registry.has<AIComponent>(e.b)) 
		{
			HandleBulletCollision(e);
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
		ModelComponent m; m.model_file = e.bullet_file; m.vertex_shader = e.bullet_vs; m.fragment_shader = e.bullet_fs;
		
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
		keyCount++;
	}

	void HandleDoorCollision(const EnterCollision &e)
	{
		if (keyCount < 6) 
		{
			return; // Not enough keys so you can't leave
		}

		// Game over happy days
	}

	void HandleBulletCollision(const EnterCollision &e) {

		auto &t = e.registry.get<TransformComponent>(e.b);
		auto &c = e.registry.get<ColourComponent>(e.b);

		c.colour = {255,0,0};
		
	}
}
