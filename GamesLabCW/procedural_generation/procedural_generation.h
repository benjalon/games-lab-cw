/**
 * procedural_generation.h
 * Defines the interface for procedural generation
 * of game mazes.
 */

#pragma once

namespace game
{
	namespace procgen
	{
		//Generate a game maze (currently, just prints grid to console)
		void generate_maze(size_t grid_size, int sparsification, int number_rooms, int min_room_size, int max_room_size);
	}
}
