#include "procedural_generation.h"

#include <vector>
#include <set>
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
			for (int x = 0; x < size_; x++)
			{
				for (int y = 0; y < size_; y++)
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
					if (y + i > 0 && utility::contains(remaining, { x, y+i }))
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
					path.emplace( x, y );
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
	};

	void generate_maze()
	{
		//Prepare fully-solid grid
		const size_t GRID_SIZE = 31;
		Grid g(GRID_SIZE);

		//Depth-first perfect maze generation
		g.generate_maze();

		g.print();
	}
}
