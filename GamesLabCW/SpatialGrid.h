/**
 * SpatialGrid.h
 * Defines the SpatialGrid class template, representing
 * a regular grid of positions in 3D space.
 */

#pragma once

#include <map>
#include <unordered_set>

#include "Vector3.h"

namespace game
{
	template <typename T>
	class SpatialGrid
	{
	public:
		using value_type = typename T;
		using index_type = std::tuple<int, int, int>;
		using iterator = typename std::unordered_set<value_type>::iterator;

	private:
		std::map<index_type, std::unordered_set<value_type>> grid_;
		Vector3 cell_size_ = { 40,40,40};

	public:
		//Inserts an element as per the given position vector
		void insert(Vector3 v, value_type elt)
		{
			index_type i = index(v);
			grid_[i].emplace(elt);
		}

		//Updates an element as per the given position vector
		index_type update(Vector3 v, value_type elt, index_type previous)
		{
			//New index
			index_type i = index(v);

			//If new index is different to previous, remove previous
			if (i != previous)
				grid_[previous].erase(elt);

			//Add at new index
			grid_[i].emplace(elt);

			return i;
		}

		//Gets begin/end iterators of elements corresponding to the given position vector 
		std::pair<iterator, iterator> get_cell(Vector3 v)
		{
			auto &cell = grid_[index(v)];
			return { cell.begin(), cell.end() };
		}

		//Gets begin/end iterators of all neighbouring elements to the given position
		std::pair<iterator, iterator> get_cells_near(Vector3 v)
		{
			//Union of all neighbouring cells' elements
			static std::unordered_set<value_type> cells;
			cells.clear();

			//Iteratively construct union over all immediate neighbours
			for (int i = -1; i < 2; i++)
				for (int j = -1; j < 2; j++)
					for (int k = -1; k < 2; k++)
					{
						auto [x, y, z] = index(v);
						index_type index = { x + i, y + j, z + k };
						auto &cell = grid_[index];
						cells.insert(cell.begin(), cell.end());
					}

			return { cells.begin(), cells.end() };
		}

		//Removes all elements from this grid
		void clear()
		{
			for (auto &v : grid_)
				v.second.clear();
		}

		//Gets the 3-tuple grid index of the given position vector
		index_type index(Vector3 v) const
		{
			int x = static_cast<int>(v.x / cell_size_.x);
			int y = static_cast<int>(v.y / cell_size_.y);
			int z = static_cast<int>(v.z / cell_size_.z);

			if (v.x < 0.0) x--;
			if (v.y < 0.0) y--;
			if (v.z < 0.0) z--;

			return { x,y,z };
		}
	};
}
