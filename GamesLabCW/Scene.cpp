/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include "Systems.h"
#include "Prototypes.h"
#include "renderer/Renderer.h"
#include "Utility.h"

game::Scene::Scene()
{
	create(GameStateComponent());
}

void game::Scene::tick(double dt)
{
	//Invoke all systems
	for (auto &s : systems::system_invokers)
		s({ *this, dt }, registry_);

	//Detect collisions
	auto view = registry_.view<SphereCollisionComponent, TransformComponent>();
	for (auto i = view.begin(); i != view.end(); ++i)
	{
		auto &[s1, t1] = view.get<SphereCollisionComponent, TransformComponent>(*i);

		//Test against all remaining entities (avoids duplicate detection)
		auto j = i;
		for (++j; j != view.end(); ++j)
		{
			auto &[s2, t2] = view.get<SphereCollisionComponent, TransformComponent>(*j);

			//Test if currently colliding (distance between centres less than sum of radii)
			double d2 = std::pow(t2.position.x - t1.position.x, 2) +
				std::pow(t2.position.y - t1.position.y, 2) +
				std::pow(t2.position.z - t1.position.z, 2);

			bool currently_colliding = d2 < std::pow(s1.radius + s2.radius, 2);
			bool was_colliding = utility::contains(s1.colliding, *j);

			//Log entering of collision
			if (currently_colliding && !was_colliding)
			{
				s1.colliding.insert(*j);
				s2.colliding.insert(*i);

				std::cout << "Enter!" << std::endl;
			}

			//Log leaving of collision
			if (!currently_colliding && was_colliding)
			{
				s1.colliding.erase(*j);
				s2.colliding.erase(*i);

				std::cout << "Leave!" << std::endl;
			}
		}
	}
}

void game::Scene::draw()
{
	//Get all ambient lights
	size_t n_a = registry_.raw_view<AmbientLightComponent>().size();
	AmbientLightComponent *a = registry_.raw_view<AmbientLightComponent>().raw();

	//Get all directional lights
	size_t n_d = registry_.raw_view<DirectionalLightComponent>().size();
	DirectionalLightComponent *d = registry_.raw_view<DirectionalLightComponent>().raw();

	//Get all point lights
	size_t n_p = registry_.raw_view<PointLightComponent>().size();
	PointLightComponent *p = registry_.raw_view<PointLightComponent>().raw();

	//Render all models in the scene for each camera
	registry_.view<CameraComponent>().each([&](auto, auto &cam) {
		registry_.view<ModelComponent, ColourComponent, TransformComponent>().each([&](auto, auto &m, auto &c, auto &t) {
			renderer::render_model(cam, m, c, t,
				n_a, a, n_d, d, n_p, p);
		});
	});
}

game::Entity game::Scene::instantiate(std::initializer_list<std::string> p)
{
	//Initialises a new entity with the given prototypes
	auto e = registry_.create();
	for (auto &s : p)
		prototypes::prototypes.at(s)(registry_, e);
	return e;
}

void game::Scene::destroy(Entity e)
{
	registry_.destroy(e);
}
