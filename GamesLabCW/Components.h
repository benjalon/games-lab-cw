/**
 * Components.h
 * Defines the basic component data structures.
 */

#pragma once

#include <string>
#include <unordered_set>

#include "GameEngine.h"
#include "Vector2.h"
#include "Vector3.h"
#include "SpatialGrid.h"

namespace game
{
	/* GAME AND ENGINE */

	struct GameStateComponent {};
	struct FirstPersonControllerComponent {};
	struct PortalComponent {};

	// Tracks how many keys are picked up, and where in the game they should be rendered
	struct KeyComponent
	{
		bool pickedUp = false;
		Vector3 destination;
	};

	struct DoorComponent {};

	/* CAMERAS AND RENDERING */

	struct TransformComponent
	{
		Vector3 position;
		Vector3 rotation;
		Vector3 scale{ 1.0, 1.0, 1.0 };

		Vector3 position_old;
		SpatialGrid<Entity>::index_type last_index;
	};

	struct CameraComponent
	{
		Entity follow;
		Vector3 position;
		Vector2 orientation;
		double fov = 60.0;
	};

	struct ModelComponent
	{
		std::string model_file;
		double shininess = 32.0;
		std::string vertex_shader;
		std::string fragment_shader;
		bool isAnimated = false;
	};

	struct ColourComponent
	{
		Vector3 colour;
		double alpha = 1.0;
	};

	struct ParticleComponent
	{
		std::string texture_file;
		int respawn_count;
		Vector3 position_variation;
		Vector3 velocity_variation;
		Vector3 color_variation;
		Vector3 color_modifier;
	};

	struct OverlayComponent
	{
		std::string texture_file;
	};

	/* LIGHTING */

	struct AmbientLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
		bool on = true;
	};

	struct DirectionalLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
		Vector3 direction{ 0.0, 0.0, 0.0 };
		bool on = true;
	};

	struct PointLightComponent
	{
		Vector3 colour{ 1.0, 1.0, 1.0 };
		double intensity = 1.0;
		Vector3 position{ 0.0, 0.0, 0.0 };
		double constant = 0;
		double linear = 0;
		double exponent = 1;
		bool on = true;
	};


	/* PHYSICS */

	struct KinematicComponent
	{
		bool solid = false; //Is this blocked by SolidPlanes?

		Vector3 velocity; //Current velocity
		Vector3 acceleration; //Current acceleration (reset every tick)
		Vector3 angular_velocity; //Current angular velocity

		Vector3 move_velocity; //Instruct movement

		Vector3 velocity_old; //Last tick's velocity
		Vector3 acceleration_old; //Last tick's acceleration
	};

	struct CollisionComponent
	{
		double radius = 3;
		std::unordered_set<Entity> colliding;
	};

	struct SolidPlaneComponent
	{
		Vector3 normal;
		Vector3 position;
		double size = std::numeric_limits<double>::infinity();
	};

	struct MoveSphere {};

	struct AIComponent {
		enum State {Look, Dodge, Shoot};
		std::string idle_file = "models/Minotaur/Minotaur@Idle.fbx";
		std::string walk_file = "models/Minotaur/Minotaur@Walk.fbx";
		std::string get_hit_file = "models/Minotaur/Minotaur@Get_Hit.fbx";
		std::string attack_file = "models/Minotaur/Minotaur@Attack.fbx";

		State state;
		double moving = 0;
		
		//Dodge trackers
		double dodgeCooldown = 0;
		double dodgeMax = 5;
		bool canDodge = dodgeCooldown > dodgeMax;
		bool dodgeBullet = false;
		double direction = 0;

		bool isHit = false;

	};

	struct ProjectileComponent{
		std::string model_file = "models/Fireball/fireball.obj";
		std::string particle_file = "models/Particles/fire2.png";
		std::string vs = "shaders/FlatColor.vert";
		std::string fs = "shaders/FlatColor.frag";
	};

	struct DetectionComponent {
		CollisionComponent c;
		Entity camera;
	};

	struct StatsComponent {
		int health = 3;
		double mana = 3;
		int keyCount = 0;
		bool gameComplete = false;
	};

	struct BulletComponent {
		bool draw = true;
		double timeAlive = 0;
		bool isPlayers = false;
	};

	struct HitboxComponent {
		CollisionComponent c;
	};
}
