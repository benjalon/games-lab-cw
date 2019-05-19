/**
 * Events.h
 * Defines the responses used for event signalling.
 */

#include "Events.h"

#include "Components.h"

entt::dispatcher game::events::dispatcher{};
std::vector<std::unique_ptr<game::events::ResponseBase>> game::events::responses;

namespace game::events
{
	void SphereEnterCollideResponse(const EnterCollision &e)
	{
		if (e.registry.has<KeyComponent>(e.a))
			HandleKeyCollision(e);

		std::cout << "Enter: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		std::cout << "Leave: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereLeaveCollideResponse, LeaveCollision);

	void HandleKeyCollision(const EnterCollision &e)
	{
		auto &[k, pl, t] = e.registry.get<KeyComponent, PointLightComponent, TransformComponent>(e.a);
		k.pickedUp = true;
		pl.on = false;
		t.position = k.destination;
	}
}
