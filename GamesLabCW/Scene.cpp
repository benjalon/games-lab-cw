/**
 * Scene.cpp
 * Implements the Scene class, representing the current
 * world of entities.
 */

#include "Scene.h"

#include "Components.h"
#include <iostream>

void game::Scene::tick(double dt)
{
	//Example simple kinematic system
	registry_.view<TransformComponent, KinematicComponent>().each(
		[dt](const auto, auto &t, auto &k)
	{
		k.velocity += k.acceleration * dt;
		t.position += k.velocity * dt;

		std::cout << "s=" << t.position << " v=" << k.velocity << " a=" << k.acceleration << std::endl;
	}
	);
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
}
