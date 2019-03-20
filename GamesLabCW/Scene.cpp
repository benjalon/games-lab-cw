/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include "Systems.h"
#include "Prototypes.h"
#include "renderer/Renderer.h"

game::Scene::Scene()
{
	create(GameStateComponent());
}

void game::Scene::tick(double dt)
{
	//Invoke all systems
	for (auto &s : systems::system_invokers)
		s({ *this, dt }, registry_);
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
		registry_.view<ModelComponent, CubemapComponent, ColourComponent, TransformComponent>().each([&](auto, auto &m, auto &cm, auto &c, auto &t) {
			renderer::render_model(cam, m, cm, c, t,
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
