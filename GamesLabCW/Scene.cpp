/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include "Systems.h"
#include "Prototypes.h"

void game::Scene::tick(double dt)
{
	//Invoke all systems
	for (auto &s : SystemRegistry::system_invokers)
		s(dt, registry_);
}

void game::Scene::draw()
{

}

game::Entity game::Scene::instantiate(std::string p)
{
	return prototypes::prototypes.at(p)(registry_);
}
