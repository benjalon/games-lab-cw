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
		std::cout << "Enter: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereEnterCollideResponse, EnterCollision);

	void SphereLeaveCollideResponse(const LeaveCollision &e)
	{
		std::cout << "Leave: " << e.a << " " << e.b << std::endl;
	}
	RESPONSE(SphereLeaveCollideResponse, LeaveCollision);

	void FireBulletResponse(const FireBullet &e)
	{
		TransformComponent t; t.scale = { 0.5, 0.5, 0.5 }; t.position = e.position; t.rotation = e.rotation;
		ModelComponent m; m.model_file = "models/Key/Key_B_02.obj";

		Vector3 currentPos = Vector3(glm::normalize(t.position.ToGLM()));

		glm::vec3 heading = glm::radians(glm::vec3(t.rotation));

		Vector3 direction = glm::cross(heading, currentPos.ToGLM());

		KinematicComponent k; k.velocity = direction;

		e.scene.instantiate("Bullet", m, t, k);
	}
	RESPONSE(FireBulletResponse, FireBullet);

}
