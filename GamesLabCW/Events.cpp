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

	void FireBulletResponse(const FireBullet &e)
	{
		TransformComponent t; t.scale = { 0.5, 0.5, 0.5 }; t.position = e.position; t.rotation = e.rotation;
		ModelComponent m; m.model_file = "models/Key/Key_B_02.obj";

		double x = fmod(t.rotation.x,360);
		double y = fmod(t.rotation.y, 360);
		double z = 0;
		

		Vector3 orientation = { x,y,z };
		orientation = Vector3(glm::normalize(orientation.ToGLM()));
		//m_vecDirection += vecLook * fSpeed*fTimeDelta;e

		//auto cam = e.registry.view<CameraComponent>().begin();
		
		e.registry.view<CameraComponent>().each([&](auto, auto &cam) {
			//glm::vec3 heading = glm::radians(glm::vec3(orientation));

		//Vector3 direction = glm::cross(orientation.ToGLM(), currentPos.ToGLM());
		
			Vector3 direction = { cam.orientation.x, cam.orientation.y, 0 };
			KinematicComponent k; k.velocity = direction;

			cout << " X:" << cam.orientation.x << " Y:" << cam.orientation.y << " Z:" << 0 << endl;



			e.scene.instantiate("Bullet", m, t, k);
		});

		
	}
	RESPONSE(FireBulletResponse, FireBullet);

	void HandleKeyCollision(const EnterCollision &e)
	{
		auto &[k, pl, t] = e.registry.get<KeyComponent, PointLightComponent, TransformComponent>(e.a);
		k.pickedUp = true;
		pl.on = false;
		t.position = k.destination;
	}
}
