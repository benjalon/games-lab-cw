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
		if (e.registry.has<KeyComponent, PointLightComponent, TransformComponent>(e.a))
			HandleKeyCollision(e);
		else if (e.registry.has<DoorComponent>(e.a))
			HandleDoorCollision(e);

		std::cout << "Enter: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		std::cout << "Leave: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereLeaveCollideResponse, LeaveCollision);

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
}
