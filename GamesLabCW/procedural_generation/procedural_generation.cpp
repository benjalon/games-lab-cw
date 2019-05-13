#include "procedural_generation.h"

#include <vector>
#include <set>
#include <unordered_set>
#include <stack>
#include <random>
#include <iostream>

#include "../Utility.h"

namespace game::procgen
{
	struct Cell
	{
		bool solid = true;
	};

	class Grid
	{
	private:
		std::vector<Cell> grid_;
		size_t size_;

	public:
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
		std::pair<int, int> index_to_coords(size_t i) const
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

		//Carves a perfect maze to span the grid
		void generate_maze()
		{
			/* Run depth-first generation, but with pure block connections
			instead	of walls. Cells with both even x and y are always wall,
			and not	considered by the visiting algorithm, but are knocked
			down as connections. */

			std::default_random_engine rng;
			rng.seed(std::random_device()());

			using Coords = std::pair<int, int>;

			//Current path stack for backtracking procedure
			std::stack<Coords> path;

			//Cells not yet visited
			std::set<Coords> remaining;
			for (int x = 1; x < size_; x += 2)
				for (int y = 1; y < size_; y += 2)
					remaining.emplace(x, y);

			//Randomly select starting point
			std::uniform_int_distribution<> dist(0, size_ / 2 - 1);
			int x = dist(rng) * 2 + 1;
			int y = dist(rng) * 2 + 1;

			//Mark as visited
			grid_[coords_to_index(x, y)].solid = false;
			remaining.erase({ x, y });
			path.emplace(x, y);

			while (!path.empty())
			{
				//Get all unvisited neighbours
				std::vector<Coords> unvisited_neighbours;
				for (int i = -2; i <= 2; i += 4)
					if (x+i > 0 && utility::contains(remaining, { x+i, y }))
					{
						unvisited_neighbours.emplace_back(x+i, y);
					}
				for (int i = -2; i <= 2; i += 4)
					if (y+i > 0 && utility::contains(remaining, { x, y+i }))
					{
						unvisited_neighbours.emplace_back(x, y+i);
					}

				//If the current cell has unvisited neighbours
				if (!unvisited_neighbours.empty())
				{
					//Randomly pick a neighbour
					std::uniform_int_distribution<size_t> dist(0, unvisited_neighbours.size() - 1);
					size_t ind = dist(rng);
					Coords neighbour = unvisited_neighbours[ind];

					//Knock down the connecting wall
					grid_[coords_to_index(
						(neighbour.first + x) / 2, (neighbour.second + y) / 2
					)].solid = false;

					//Mark as visited
					x = neighbour.first;
					y = neighbour.second;
					grid_[coords_to_index(x, y)].solid = false;
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
		}

		//Increase the sparsity by retracting n dead ends
		void sparsify(int n)
		{
			if (n < 1) return;

			//Returns true if the given cell is a dead end
			auto is_dead_end = [this](size_t i)
			{
				auto [x, y] = index_to_coords(i);
				int num_solid = 0;
				if (x + 1 < size_ && grid_[coords_to_index(x + 1, y)].solid) num_solid++;
				if (x - 1 >= 0 && grid_[coords_to_index(x - 1, y)].solid) num_solid++;
				if (y + 1 < size_ && grid_[coords_to_index(x, y + 1)].solid) num_solid++;
				if (y - 1 >= 0 && grid_[coords_to_index(x, y - 1)].solid) num_solid++;

				return !grid_[i].solid && num_solid == 3;
			};

			//Set of dead ends
			std::unordered_set<size_t> dead_ends;

			//Find all dead ends
			for (size_t i = 0; i < grid_.size(); i++)
				if (is_dead_end(i))
					dead_ends.emplace(i);

			std::default_random_engine rng;
			rng.seed(std::random_device()());

			//Erase n dead ends
			for (int i = 0; i < n; i++)
			{
				//Select random dead end
				std::uniform_int_distribution<> dist(0, dead_ends.size() - 1);
				auto it = dead_ends.begin();
				std::advance(it, dist(rng));

				//Make solid
				auto [x, y] = index_to_coords(*it);
				grid_[*it].solid = true;
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
	};

	void generate_maze()
	{
		//Parameters
		const size_t GRID_SIZE = 21;
		const int SPARSIFICATION = 120;

		//Prepare fully-solid grid
		Grid g(GRID_SIZE);

		//Depth-first perfect maze generation
		g.generate_maze();

		//Retract dead ends to increase sparsity
		g.sparsify(SPARSIFICATION);

		//Debug -- print result
		g.print();
	}
}
