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

	PROTOTYPE(FirstPersonController, FirstPersonControllerComponent, CollisionComponent, TransformComponent, KinematicComponent, ProjectileComponent,StatsComponent);

	PROTOTYPE(Model, ModelComponent, ColourComponent, TransformComponent);

	PROTOTYPE(Bullet, ModelComponent, ColourComponent, TransformComponent, CollisionComponent, KinematicComponent, BulletComponent, ParticleComponent);

	PROTOTYPE(AIModel, ModelComponent, ColourComponent, TransformComponent, HitboxComponent, KinematicComponent, AIComponent, ProjectileComponent, DetectionComponent,StatsComponent, CollisionComponent);

	PROTOTYPE(AITest, ModelComponent, ColourComponent, TransformComponent, HitboxComponent, KinematicComponent, AIComponent, ProjectileComponent, CameraComponent);



	PROTOTYPE(ParticleEffect, ParticleComponent, ColourComponent, TransformComponent, KinematicComponent);

	PROTOTYPE(Overlay, OverlayComponent);

	PROTOTYPE(Key, ModelComponent, ColourComponent, CollisionComponent, TransformComponent, KeyComponent, PointLightComponent, KinematicComponent);

	PROTOTYPE(AmbientLight, AmbientLightComponent);

	PROTOTYPE(DirectionalLight, DirectionalLightComponent);
	
	PROTOTYPE(PointLight, PointLightComponent);

	PROTOTYPE(KinematicBody, TransformComponent, KinematicComponent);
	PROTOTYPE(SolidPlane, SolidPlaneComponent);

	PROTOTYPE(Portal, PortalComponent, CollisionComponent, TransformComponent);
}

