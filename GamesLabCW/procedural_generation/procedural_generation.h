/**
 * procedural_generation.h
 * Defines the interface for procedural generation
 * of game mazes.
 */

#pragma once

#include "../Scene.h"

namespace game
{
	namespace procgen
	{
		//Generate a game maze, returning the suggested player position
		Vector3 generate_maze(Scene &scene, size_t grid_size, int sparsification, int number_rooms, int min_room_size, int max_room_size);

		//Load the main hub level
		void load_hub(Scene &scene, int keys_collected = 0);
	}
}
