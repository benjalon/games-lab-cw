/**
 * Prototypes.cpp
 * Defines the prototypes in the game engine.
 */

#include "Prototypes.h"

#include "Components.h"

std::unordered_map<std::string, entt::prototype<game::Entity>> game::prototypes::prototypes;

void game::prototypes::register_prototypes()
{
	PROTOTYPE(Camera, CameraComponent);
	PROTOTYPE(Model, ModelComponent, ColourComponent, TransformComponent);

	PROTOTYPE(NamedEntity, NameComponent);
	PROTOTYPE(KinematicBody, TransformComponent, KinematicComponent);
}
