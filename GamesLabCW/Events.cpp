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
		if (e.registry.has<KeyComponent>(e.a)) {
			HandleKeyCollision(e);
		}
		else if (e.registry.has<AIComponent>(e.b)) {
			HandleBulletCollision(e);
		}
			

		//std::cout << "Enter: " << e.a << " " << e.b << std::endl;
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

		
	}
	RESPONSE(FireBulletResponse, FireBullet);

	void HandleKeyCollision(const EnterCollision &e)
	{
		auto &[k, pl, t] = e.registry.get<KeyComponent, PointLightComponent, TransformComponent>(e.a);
		k.pickedUp = true;
		pl.on = false;
		t.position = k.destination;
	}

	void HandleBulletCollision(const EnterCollision &e) {

		auto &t = e.registry.get<TransformComponent>(e.b);
		auto &c = e.registry.get<ColourComponent>(e.b);

		c.colour = {255,0,0};
		
	}
}
