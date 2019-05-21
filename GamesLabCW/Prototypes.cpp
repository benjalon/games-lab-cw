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
	PROTOTYPE(FirstPersonController, FirstPersonControllerComponent, CollisionComponent, TransformComponent, KinematicComponent, BulletComponent);

	PROTOTYPE(Model, ModelComponent, ColourComponent, TransformComponent);
	
	PROTOTYPE(Bullet, ModelComponent, ColourComponent, TransformComponent, CollisionComponent, KinematicComponent);
	PROTOTYPE(AIModel, ModelComponent, ColourComponent, TransformComponent, KinematicComponent, AIComponent, CameraComponent, CollisionComponent, BulletComponent);
	PROTOTYPE(DetectionRadius, DetectionComponent, TransformComponent, CollisionComponent);

	PROTOTYPE(ParticleEffect, ParticleComponent, ColourComponent, TransformComponent, KinematicComponent);

	PROTOTYPE(Key, ModelComponent, ColourComponent, CollisionComponent, TransformComponent, KeyComponent, PointLightComponent);

	PROTOTYPE(AmbientLight, AmbientLightComponent);
	PROTOTYPE(DirectionalLight, DirectionalLightComponent);
	PROTOTYPE(PointLight, PointLightComponent);

	PROTOTYPE(KinematicBody, TransformComponent, KinematicComponent);
}

