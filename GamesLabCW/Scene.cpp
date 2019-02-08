/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include <iostream>

#include "Systems.h"

void game::Scene::tick(double dt)
{
	//Invoke all systems
	for (auto &s : SystemRegistry::system_invokers)
		s(dt, registry_);
}

void game::Scene::draw()
{

}

void game::Scene::instantiate()
{
	//Example kinematic entity creation
	auto entity = registry_.create();
	registry_.assign<TransformComponent>(entity);
	registry_.assign<KinematicComponent>(entity, Vector3(0,0,0), Vector3(1,0,0));

	//Example named entity creation
	auto entity2 = registry_.create();
	registry_.assign<NameComponent>(entity2, "MyEntity");
}
