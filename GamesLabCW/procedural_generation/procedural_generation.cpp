/**
 * procedural_generation.h
 * Implements the interface for procedural generation
 * of game mazes.
 */

#include "procedural_generation.h"

#include <vector>
#include <set>
#include <unordered_set>
#include <stack>
#include <random>
#include <iostream>

#include "../Utility.h"
#include "../Components.h"

namespace game::procgen
{
	//Provides a random engine seeded once at the start of the game
	std::default_random_engine &rng()
	{
		static bool set_seed = false;
		static unsigned seed = 0;

		if (!set_seed)
		{
			seed = std::random_device()();
			std::cout << "Procgen seed: " << seed << std::endl;
			set_seed = true;
		}

		static std::default_random_engine rng(seed);
		return rng;
	}

	//Represents a single cell in maze space
	struct Cell
	{
		bool solid = true; //Is the cell solid wall?
		int section = 0; //Maze section ID
	};

	//Represents a grid of cells which can represent and generate a maze with rooms
	class Grid
	{
	private:
		std::vector<Cell> grid_;
		size_t size_;
		std::unordered_set<int> connected_;

	public:
		using Coords = std::pair<int, int>;

		//Construct grid, with the given odd width/height
		Grid(size_t size) : size_(size), grid_(size * size)
		{
		}

		//Converts a pair of coordinated to flat index representation
		size_t coords_to_index(int x, int y) const
		{
			return x * size_ + y;
		}

		//Converts a flat index to its semantic coordinates
		Coords index_to_coords(size_t i) const
		{
			return { i / size_, i % size_ };
		}

		//Prints out an ASCII representation of the grid
		void print() const
		{
			for (int y = 0; y < size_; y++)
			{
				for (int x = 0; x < size_; x++)
					std::cout << (grid_[coords_to_index(x, y)].solid ? "#" : " ");

				std::cout << std::endl;
			}
		}

		//Gets the number of solid cells in the grid
		int number_solid() const
		{
			int n = 0;
			for (auto &c : grid_)
				if (c.solid) n++;

			return n;
		}

		//Returns true if the given cell is a dead end
		bool is_dead_end(size_t i) const
		{
			auto[x, y] = index_to_coords(i);
			int num_solid = 0;
			if (x + 1 < size_ && grid_[coords_to_index(x + 1, y)].solid) num_solid++;
			if (x - 1 >= 0 && grid_[coords_to_index(x - 1, y)].solid) num_solid++;
			if (y + 1 < size_ && grid_[coords_to_index(x, y + 1)].solid) num_solid++;
			if (y - 1 >= 0 && grid_[coords_to_index(x, y - 1)].solid) num_solid++;

			return !grid_[i].solid && num_solid == 3;
		};

		//Carves a perfect maze to span the grid
		void generate_maze()
		{
			/* Run depth-first generation, but with pure block connections
			instead	of walls. Cells with both even x and y are always wall,
			and not	considered by the visiting algorithm, but are knocked
			down as connections. */
			
			//Current path stack for backtracking procedure
			std::stack<Coords> path;

			//Cells not yet visited
			std::set<Coords> remaining;
			for (int x = 1; x < size_; x += 2)
				for (int y = 1; y < size_; y += 2)
					remaining.emplace(x, y);

			//Randomly select starting point
			std::uniform_int_distribution<> grid_dist(0, size_ / 2 - 1);
			int x = grid_dist(rng()) * 2 + 1;
			int y = grid_dist(rng()) * 2 + 1;

			//Mark as visited, with section ID 1
			grid_[coords_to_index(x, y)] = { false, 1 };
			remaining.erase({ x, y });
			path.emplace(x, y);

			while (!path.empty())
			{
				//Get all unvisited neighbours
				std::vector<Coords> unvisited_neighbours;
				for (int i = -2; i <= 2; i += 4)
					if (x + i > 0 && utility::contains(remaining, { x + i, y }))
					{
						unvisited_neighbours.emplace_back(x + i, y);
					}
				for (int i = -2; i <= 2; i += 4)
					if (y + i > 0 && utility::contains(remaining, { x, y + i }))
					{
						unvisited_neighbours.emplace_back(x, y + i);
					}

				//If the current cell has unvisited neighbours
				if (!unvisited_neighbours.empty())
				{
					//Randomly pick a neighbour
					std::uniform_int_distribution<size_t> neighbour_dist(0, unvisited_neighbours.size() - 1);
					size_t ind = neighbour_dist(rng());
					Coords neighbour = unvisited_neighbours[ind];

					//Knock down the connecting wall
					grid_[coords_to_index(
						(neighbour.first + x) / 2, (neighbour.second + y) / 2
					)] = { false, 1 };

					//Mark as visited
					x = neighbour.first;
					y = neighbour.second;
					grid_[coords_to_index(x, y)] = { false, 1 };
					remaining.erase({ x, y });
					path.emplace(x, y);
				}
				else
				{
					auto next = path.top();
					x = next.first;
					y = next.second;
					path.pop();
				}
			}

			connected_.emplace(1);
		}

		//Increase the sparsity by retracting n dead ends
		void sparsify(int n)
		{
			if (n < 1) return;

			//Set of dead ends
			std::unordered_set<size_t> dead_ends;

			//Find all dead ends
			for (size_t i = 0; i < grid_.size(); i++)
				if (is_dead_end(i))
					dead_ends.emplace(i);

			//Erase n dead ends
			for (int i = 0; i < n && dead_ends.size() > 0; i++)
			{
				//Select random dead end
				std::uniform_int_distribution<> dead_ends_dist(0, dead_ends.size() - 1);
				auto it = dead_ends.begin();
				std::advance(it, dead_ends_dist(rng()));

				//Make solid
				auto [x, y] = index_to_coords(*it);
				grid_[*it] = { true, 0 };
				dead_ends.erase(it);

				//Log neighbours if new dead ends
				if (is_dead_end(coords_to_index(x + 1, y)))
					dead_ends.emplace(coords_to_index(x + 1, y));
				if (is_dead_end(coords_to_index(x - 1, y)))
					dead_ends.emplace(coords_to_index(x - 1, y));
				if (is_dead_end(coords_to_index(x, y + 1)))
					dead_ends.emplace(coords_to_index(x, y + 1));
				if (is_dead_end(coords_to_index(x, y - 1)))
					dead_ends.emplace(coords_to_index(x, y - 1));
			}
		}

		//Place n rooms of random sizes
		void populate_rooms(int n, int min_size, int max_size, int id_from = 2)
		{
			//Each room is randomly between min and max size
			std::uniform_int_distribution<> size_dist(min_size, max_size);
			int room_size = size_dist(rng());

			//While rooms are left to populate
			for (int i = 0; n > 0 && i < grid_.size(); i++)
			{
				//Don't bother checking extremes
				auto [x, y] = index_to_coords(i);
				if (x > 0 && x < (size_ - room_size) && y > 0 && y < (size_ - room_size))
				{
					//If destination is all solid, including surroundings (but not border corners)
					bool all_solid = grid_[i].solid;
					for (int dx = -1; dx <= room_size; dx++)
						for (int dy = -1; dy <= room_size; dy++)
							if (dx != -1 && dx != room_size || dy != -1 && dy != room_size)
								all_solid &= grid_[coords_to_index(x + dx, y + dy)].solid;

					if (all_solid)
					{
						//Place room
						grid_[i].solid = false;
						for (int dx = 0; dx < room_size; dx++)
							for (int dy = 0; dy < room_size; dy++)
								grid_[coords_to_index(x + dx, y + dy)] = { false, id_from };

						/* Attempts to connect to corridor can get stuck in on itself, so a
						Las Vegas approach just keeps randomly trying until success, as a valid
						solution should always exist. */

						bool unfinished = true;
						while (unfinished)
						{
							//Set of walls knocked down for this path, to be undone if necessary
							std::unordered_set<size_t> current_path;

							//Pick random edge border cell (not on grid border)
							std::unordered_set<size_t> border_cells;
							for (int dx = -1; dx <= room_size; dx++)
								for (int dy = -1; dy <= room_size; dy++)
									if ((dx == -1 || dx == room_size) ^
										(dy == -1 || dy == room_size) &&
										x + dx > 0 && y + dy > 0 &&
										x + dx < size_ - 1 && y + dy < size_ - 1)
										border_cells.emplace(coords_to_index(x + dx, y + dy));

							std::uniform_int_distribution<size_t> border_dist(0, border_cells.size() - 1);
							auto it = border_cells.begin();
							std::advance(it, border_dist(rng()));

							//Carve out path from border cell to corridors
							size_t current_cell = *it;
							grid_[current_cell] = { false, id_from };
							current_path.emplace(current_cell);

							bool closed = true;
							while (closed)
							{
								unfinished = false;

								//Current cell coords
								auto [xc, yc] = index_to_coords(current_cell);

								//Would the given offset open up to the connected maze?
								auto open_to_maze = [&](int x, int y)
								{
									return (x + 1 < size_ && utility::contains(connected_, grid_[coords_to_index(x + 1, y)].section)) ||
										(x > 0 && utility::contains(connected_, grid_[coords_to_index(x - 1, y)].section)) ||
										(y + 1 < size_ && utility::contains(connected_, grid_[coords_to_index(x, y + 1)].section)) ||
										(y > 0 && utility::contains(connected_, grid_[coords_to_index(x, y - 1)].section));
								};

								//If a connection opens to the maze, we're done
								if (open_to_maze(xc + 1, yc))
								{
									grid_[coords_to_index(xc + 1, yc)] = { false, id_from };
									closed = false;
								}
								else if (xc - 1 > 0 && open_to_maze(xc - 1, yc))
								{
									grid_[coords_to_index(xc - 1, yc)] = { false, id_from };
									closed = false;
								}
								else if (open_to_maze(xc, yc + 1))
								{
									grid_[coords_to_index(xc, yc + 1)] = { false, id_from };
									closed = false;
								}
								else if (yc - 1 > 0 && open_to_maze(xc, yc - 1))
								{
									grid_[coords_to_index(xc, yc - 1)] = { false, id_from };
									closed = false;
								}
								//Failing that, pick a neighbour that leads to another dead end
								else
								{
									std::vector<Coords> neighbours;

									auto open_to_dead_end = [&](int x, int y)
									{
										bool state = grid_[coords_to_index(x, y)].solid;
										grid_[coords_to_index(x, y)].solid = false;
										bool result = is_dead_end(coords_to_index(x, y));
										grid_[coords_to_index(x, y)].solid = state;
										return result;
									};

									if (open_to_dead_end(xc + 1, yc)) neighbours.emplace_back(xc + 1, yc);
									if (open_to_dead_end(xc - 1, yc)) neighbours.emplace_back(xc - 1, yc);
									if (open_to_dead_end(xc, yc + 1)) neighbours.emplace_back(xc, yc + 1);
									if (open_to_dead_end(xc, yc - 1)) neighbours.emplace_back(xc, yc - 1);

									std::uniform_int_distribution<size_t> neighbours_dist(0, neighbours.size() - 1);

									//If solution is invalid, stop and try again
									if (neighbours.size() == 0)
									{
										for (size_t ind : current_path)
											grid_[ind] = { true, 0 };

										closed = false;
										unfinished = true;
									}
									//Otherwise, carve out neighbour and continue path
									else
									{
										auto [x2, y2] = neighbours[neighbours_dist(rng())];
										current_cell = coords_to_index(x2, y2);
										grid_[current_cell] = { false, id_from };
										current_path.emplace(current_cell);
									}
								}
							}
						}

						//Update room(s) still to place
						n--;
						room_size = size_dist(rng());
						connected_.emplace(id_from++);
					}
				}
			}

			//If rooms are still left, try again with smaller rooms
			if (n > 0 && max_size > min_size)
				populate_rooms(n, min_size, max_size - 1, id_from);
		}

		//Builds the maze into the given scene
		void build_scene(Scene &scene)
		{
			//Instantiate models for each cell type
			ModelComponent m_type_1; m_type_1.model_file = "models/Procedural/type1.obj";
			ModelComponent m_type_2; m_type_2.model_file = "models/Procedural/type2.obj";
			ModelComponent m_type_3; m_type_3.model_file = "models/Procedural/type3.obj";
			ModelComponent m_type_4; m_type_4.model_file = "models/Procedural/type4.obj";

			//Parameters of model
			double model_size = 20.0;
			double scale = 0.5;

			//Effective size
			double cell_size = model_size * scale;

			//Separation between cells to prevent z-fighting
			double give = 0.001;

			//Instantiate correct type for each cell in grid
			for (int x = 1; x < size_ - 1; x++)
				for (int y = 1; y < size_ - 1; y++)
					if (!grid_[coords_to_index(x, y)].solid)
					{
						//Universally applicable transformation
						TransformComponent t;
						t.position = { x * (cell_size + give), 0, y * (cell_size + give) };
						t.scale = { scale, scale, scale };

						//Determine number of solid neighbours
						int num_neighbours = 0;
						bool east, west, south, north;
						if (east = grid_[coords_to_index(x + 1, y)].solid) num_neighbours++;
						if (west = grid_[coords_to_index(x - 1, y)].solid) num_neighbours++;
						if (south = grid_[coords_to_index(x, y + 1)].solid) num_neighbours++;
						if (north = grid_[coords_to_index(x, y - 1)].solid) num_neighbours++;

						//Determine which type to use and rotate as necessary
						switch (num_neighbours)
						{
						case 1:
							//Edge piece
							if (south) t.rotation = { 0, 90.0, 0 };
							else if (east) t.rotation = { 0, 180.0, 0 };
							else if (north) t.rotation = { 0, 270.0, 0 };

							scene.instantiate("Model", m_type_3, t);
							break;

						case 2:
							//Corridor piece
							if (east && west)
							{
								scene.instantiate("Model", m_type_1, t);
							}
							else if (north && south)
							{
								t.rotation = { 0, 90.0, 0 };
								scene.instantiate("Model", m_type_1, t);
							}
							//Corner piece
							else
							{
								if (south && east) t.rotation = { 0, 90.0, 0 };
								else if (east && north) t.rotation = { 0, 180.0, 0 };
								else if (north && west) t.rotation = { 0, 270.0, 0 };

								scene.instantiate("Model", m_type_2, t);
							}
							break;

						case 3:
							//Dead end piece
							if (!north) t.rotation = { 0, 90.0, 0 };
							else if (!west) t.rotation = { 0, 180.0, 0 };
							else if (!south) t.rotation = { 0, 270.0, 0 };

							scene.instantiate("Model", m_type_4, t);
							break;

						default:
							break;
						}
					}
		}
	};

	void generate_maze(Scene &scene, size_t grid_size, int sparsification, int number_rooms, int min_room_size, int max_room_size)
	{
		//Prepare fully-solid grid
		Grid g(grid_size);

		//Depth-first perfect maze generation
		g.generate_maze();

		//Retract dead ends to increase sparsity
		g.sparsify(sparsification);

		//Place rooms
		if (g.number_solid() < grid_size * grid_size)
			g.populate_rooms(number_rooms, min_room_size, max_room_size);

		//Print minimap to console
		g.print();

		//Build the map into the scene
		g.build_scene(scene);
	}
}
