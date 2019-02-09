/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include "Systems.h"
#include "Prototypes.h"
#include "Renderer.h"

void game::Scene::tick(double dt)
{
	//Invoke all systems
	for (auto &s : systems::system_invokers)
		s(dt, registry_);
}

void game::Scene::draw()
{
	//Render all models in the scene for each camera
	registry_.view<CameraComponent>().each([&](auto, auto &c) {
		registry_.view<ModelComponent, TransformComponent>().each([&](auto, auto &m, auto &t) {
			renderer::render_model(c, m, t);
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
