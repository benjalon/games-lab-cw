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
		s({ *this, dt, registry_ }, registry_);

	//Broadcast all queued events
	events::dispatcher.update();
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

	//Filter point lights to nearby only
	if (CULL_POINT_LIGHTS)
	{
		auto player = registry_.view<FirstPersonControllerComponent>();
		if (drawn_yet && player.size() > 0)
		{
			auto t_player = registry_.get<TransformComponent>(*player.begin());

			std::vector<PointLightComponent> nearby;
			nearby.reserve(n_p);
			for (int i = 0; i < n_p; i++)
			{
				Vector3 to_player = p[i].position - t_player.position;
				double dist = std::sqrt(to_player.x * to_player.x +
					to_player.y * to_player.y +
					to_player.z * to_player.z
				);

				if (dist < RENDER_DISTANCE - 100.0)
					nearby.emplace_back(p[i]);
			}

			//Update data, ensuring at least one point light is passed in
			if (nearby.size() > 0)
			{
				n_p = nearby.size();
				p = nearby.data();
			}
			else if (n_p > 0)
				n_p = 1;
		}
	}

	//Render all models in the scene for each camera
	registry_.view<CameraComponent>().each([&](auto, auto &cam) {
		registry_.view<ModelComponent, ColourComponent, TransformComponent>().each([&](auto, auto &m, auto &c, auto &t) {
			renderer::render_model(cam, m, c, t,
				n_a, a, n_d, d, n_p, p);
		});

		registry_.view<ParticleComponent, ColourComponent, TransformComponent>().each([&](auto, auto &p, auto &c, auto &t) {
			renderer::render_particle(cam, p, c, t);
		});

		registry_.view<ImageComponent>().each([&](auto, auto &i) {
			renderer::render_image(cam, i);
		});
	});

	drawn_yet = true;
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

void game::Scene::clear()
{
	registry_.reset();
	spatial_grid.clear();
	drawn_yet = false;

	create(GameStateComponent());
}
