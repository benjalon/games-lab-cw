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

		//Builds the maze into the given scene, returning suggested player position
		Vector3 build_scene(Scene &scene)
		{
			//Utility to place key
			auto place_key = [&](Vector3 position)
			{
				ModelComponent m_key; m_key.model_file = "models/Key/Key_B_02.obj";
				KeyComponent k_key; k_key.destination = { -40, 10, 10 };
				TransformComponent t_key; t_key.scale = { 0.5, 0.5, 0.5 }; t_key.rotation = { 0, 180, 180 };
				t_key.position = position;
				PointLightComponent pl_key{ {1, 180 / 255.0, 120.0 / 255.0}, 60, t_key.position };
				KinematicComponent kn_key; kn_key.angular_velocity = { 0, 90, 0 };
				scene.instantiate("Key", m_key, t_key, k_key, pl_key, kn_key);
			};

			//Utility to place torch
			auto place_torch = [&](Vector3 position)
			{
				ParticleComponent p_torch; p_torch.texture_file = "models/Particles/fire.png"; p_torch.respawn_count = 10;
				p_torch.position_variation = Vector3(100, 50, 40);
				p_torch.velocity_variation = Vector3(100, 50, 50);
				p_torch.color_variation = Vector3(100, -0.5, 100);
				p_torch.color_modifier = Vector3(1, 0.15, 0);

				TransformComponent t_torch; t_torch.scale = { 5, 5, 5 };
				t_torch.position = position;

				scene.instantiate("Model", t_torch, ModelComponent{ "models/Torch/torch.obj" });

				// Light most torches at random
				if (rand() % 10 > 3)
				{
					scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 45,
						t_torch.position + Vector3(0, 8, 0)
						});
					scene.instantiate("ParticleEffect", p_torch, TransformComponent{
						t_torch.position + Vector3(0, 8, 0)
						});
				}
			};

			//Instantiate models for each cell type
			ModelComponent m_type_1; m_type_1.model_file = "models/Procedural/type1.obj";
			ModelComponent m_type_2; m_type_2.model_file = "models/Procedural/type2.obj";
			ModelComponent m_type_3; m_type_3.model_file = "models/Procedural/type3.obj";
			ModelComponent m_type_4; m_type_4.model_file = "models/Procedural/type4.obj";
			ModelComponent m_type_5; m_type_5.model_file = "models/Procedural/type5.obj";

			//Parameters of model
			double model_size = 20.0;
			double scale = 1.2;

			//Effective size
			double cell_size = model_size * scale;

			//Separation between cells to prevent z-fighting
			double give = -0.001;

			//Size of individual solid planes
			double plane_size = cell_size / 2 + 2;

			//Has the key been instantiated yet?
			bool key = false;
			Coords key_pos;

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
						case 0:
							//Room piece
							scene.instantiate("Model", m_type_5, t);
							/*scene.instantiate("Model", m_minotaur, c_minotaur, t_minotaur, h_minotaur, d_minotaur, s_minotaur, CollisionComponent{ 6 });*/

							break;

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
								t.position.y += 0.1 * scale; //Account for slight gap
								scene.instantiate("Model", m_type_1, t);
							}
							else if (north && south)
							{
								t.position.y += 0.1 * scale; //Account for slight gap
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

								//Instantiate torch
								place_torch({ x * cell_size, -11.5, y * cell_size });
							}
							break;

						case 3:
							//Dead end piece
							if (!north) t.rotation = { 0, 90.0, 0 };
							else if (!west) t.rotation = { 0, 180.0, 0 };
							else if (!south) t.rotation = { 0, 270.0, 0 };

							scene.instantiate("Model", m_type_4, t);

							//Instantiate key
							if (!key)
							{
								place_key({ x * cell_size, 0.5, y * cell_size });
								key_pos = { x, y };
								key = true;
							}

							break;
						}

						//Place solid collision planes
						if (west) scene.instantiate("SolidPlane", SolidPlaneComponent{ { 1, 0, 0 }, t.position + Vector3(-cell_size / 2, 0, 0), plane_size });
						if (east) scene.instantiate("SolidPlane", SolidPlaneComponent{ { -1, 0, 0 }, t.position + Vector3(cell_size / 2, 0, 0), plane_size });
						if (north) scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 0, 1 }, t.position + Vector3(0, 0, -cell_size / 2), plane_size });
						if (south) scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 0, -1 }, t.position + Vector3(0, 0, cell_size / 2), plane_size });
					}

			//If there were no dead ends, place key in random cell
			if (!key)
			{
				std::uniform_int_distribution<size_t> index_dist(0, size_ - 1);
				size_t i = index_dist(rng());
				while (!grid_[i].solid)
					i = index_dist(rng());

				auto [x, y] = index_to_coords(i);
				place_key({ x * cell_size, 0.5, y * cell_size });
				key_pos = { x, y };
				key = true;
			}

			//Solid floor
			scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 1, 0 }, { 0, -7, 0 } });

			//Calculate suggested player position as cell furthest from key (Manhattan distance)
			Coords player_pos;
			double longest_dist = -std::numeric_limits<double>::infinity();
			for (size_t i = 0; i < grid_.size(); i++)
				if (!grid_[i].solid)
				{
					auto [x, y] = index_to_coords(i);
					double dist = abs(x - key_pos.first) + abs(y - key_pos.second);
					if (dist > longest_dist)
						player_pos = { x, y };
				}

			Vector3 playerPos = { player_pos.first * cell_size, 6, player_pos.second * cell_size };

			auto player = scene.instantiate("FirstPersonController", TransformComponent{ playerPos , { 180,0,0 } }, CollisionComponent{ 6 }, KinematicComponent{ true });
			auto camera = scene.instantiate("Camera", CameraComponent{ player });

			// Minotaur setup
			ModelComponent m_minotaur; m_minotaur.model_file = "models/Minotaur/Minotaur@Idle.fbx";
			ColourComponent c_minotaur; c_minotaur.colour = { 0, 0, 255 };
			DetectionComponent d_minotaur; d_minotaur.c.radius = 30; d_minotaur.camera = camera;
			TransformComponent t_minotaur; t_minotaur.scale = { 0.10, 0.1, 0.1 }; t_minotaur.position = { 0, 6, -15 }; /*t_minotaur.rotation = { 90, 180, 0 };*/
			HitboxComponent h_minotaur; h_minotaur.c.radius = 2.5;
			StatsComponent s_minotaur; s_minotaur.health = 3, s_minotaur.mana = 0;

			return playerPos;
		}
	};

	Vector3 generate_maze(Scene &scene, size_t grid_size, int sparsification, int number_rooms, int min_room_size, int max_room_size)
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
		return g.build_scene(scene);
	}

	void load_hub(Scene &scene, int keys_collected)
	{
		//Player/camera
		StatsComponent s_player; s_player.health = 3; s_player.mana = 0;
		auto player = scene.instantiate("FirstPersonController", TransformComponent{ {0,6,5} , { 180,0,0 } }, CollisionComponent{ 6 }, KinematicComponent{ true }, s_player);
		scene.camera = scene.instantiate("Camera", CameraComponent{ player });

		//Room stuff
		ModelComponent m_water; m_water.model_file = "models/Water/water.obj"; m_water.vertex_shader = "shaders/Water.vert"; m_water.fragment_shader = "shaders/Water.frag";
		scene.instantiate("Model", m_water);

		ModelComponent m_room; m_room.model_file = "models/Room/room.obj";
		TransformComponent t_room; t_room.position.y = 10; t_room.scale = { 0.5, 0.5, 0.5 };
		scene.instantiate("Model", m_room, t_room);

		//Solid planes
		scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 1, 0 }, { 0, 0, 0 } }); //floor
		scene.instantiate("SolidPlane", SolidPlaneComponent{ { -1, 0, 0 }, { 30, 0, 0 } }); //right wall
		scene.instantiate("SolidPlane", SolidPlaneComponent{ { 1, 0, 0 }, { -35, 0, 0 } }); //left wall
		scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 0, -1 }, { 0, 0, 30 } }); //back wall
		scene.instantiate("SolidPlane", SolidPlaneComponent{ { 0, 0, 1 }, { 0, 0, -30 } }); //front wall

		// Minotaur test model
		//Entity camera;
		////ModelComponent, ColourComponent, TransformComponent, HitboxComponent, KinematicComponent, AIComponent, CameraComponent, ProjectileComponent, DetectionComponent,StatsComponent, CollisionComponent
		//ModelComponent m_minotaur; m_minotaur.model_file = "models/Minotaur/Minotaur@Idle.fbx";
		//ColourComponent c_minotaur; c_minotaur.colour = { 0, 0, 255 };
		//DetectionComponent d_minotaur; d_minotaur.c.radius = 30; d_minotaur.camera = camera;
		//TransformComponent t_minotaur; t_minotaur.scale = { 0.10, 0.1, 0.1 }; t_minotaur.position = { 0, 6, -15 };
		//HitboxComponent h_minotaur; h_minotaur.c.radius = 2.5;
		//StatsComponent s_minotaur; s_minotaur.health = 3, s_minotaur.mana = 1;
		//scene.instantiate("AIModel", m_minotaur, c_minotaur, t_minotaur, h_minotaur, d_minotaur, s_minotaur, CollisionComponent{ 6 });

		//Torches
		ModelComponent m_torch; m_torch.model_file = "models/Torch/torch.obj";

		ParticleComponent p_torch; p_torch.texture_file = "models/Particles/fire.png"; p_torch.respawn_count = 1;
		p_torch.position_variation = Vector3(100, 50, 40);
		p_torch.velocity_variation = Vector3(100, 50, 50);
		p_torch.color_variation = Vector3(100, -0.5, 100);
		p_torch.color_modifier = Vector3(1, 0.15, 0);

		TransformComponent t_torch1; t_torch1.position = { 26, 0, -23 }; t_torch1.scale = { 5, 5, 5 };
		scene.instantiate("Model", m_torch, t_torch1);
		scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 40, {26, 7, -23} });
		scene.instantiate("ParticleEffect", p_torch, TransformComponent{ { 26, 8, -23 } });

		TransformComponent t_torch2; t_torch2.position = { 26, 0, 23 }; t_torch2.scale = { 5, 5, 5 };
		scene.instantiate("Model", m_torch, t_torch2);
		scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 40, {26, 7, 23} });
		scene.instantiate("ParticleEffect", p_torch, TransformComponent{ { 26, 8, 23 } });

		TransformComponent t_torch3; t_torch3.position = { -26, 0, -10 }; t_torch3.scale = { 5, 5, 5 };
		scene.instantiate("Model", m_torch, t_torch3);
		scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 40, { -26, 7, -10 } });
		scene.instantiate("ParticleEffect", p_torch, TransformComponent{ { -26, 8, -10 } });

		TransformComponent t_torch4; t_torch4.position = { -26, 0, 10 }; t_torch4.scale = { 5, 5, 5 };
		scene.instantiate("Model", m_torch, t_torch4);
		scene.instantiate("PointLight", PointLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 40, { -26, 7, 10 } });
		scene.instantiate("ParticleEffect", p_torch, TransformComponent{ { -26, 8, 10 } });

		//Portal
		ParticleComponent p_portal; p_portal.texture_file = "models/Particles/star.png"; p_portal.respawn_count = 1;
		p_portal.position_variation = Vector3(100, 50, 10);
		p_portal.velocity_variation = Vector3(100, 50, 40);
		p_portal.color_variation = Vector3(100, -0.5, 100);
		p_portal.color_modifier = Vector3(0.8, 0.2, 1);
		scene.instantiate("ParticleEffect", p_portal, TransformComponent{ Vector3(3, 5, 22) });
		scene.instantiate("PointLight", PointLightComponent{ {1, 105.0 / 255.0, 180.0 / 255.0}, 40, { 3, 3, 22} });
		scene.instantiate("Portal", TransformComponent{ { 3, 3, 22 } });

		//UI
		OverlayComponent i_hp; i_hp.texture_file = "models/UI/hearts-2.png";
		scene.instantiate("Overlay", i_hp);
		OverlayComponent i_mp; i_mp.texture_file = "models/UI/mana-3.png";
		scene.instantiate("Overlay", i_mp);
		OverlayComponent i_ch; i_ch.texture_file = "models/UI/crosshair.png";
		scene.instantiate("Overlay", i_ch);

		//Skybox
		TransformComponent t_skybox; t_skybox.scale = { 20, 20, 20 };
		ModelComponent m_skybox; m_skybox.model_file = "models/Skybox/skybox.obj"; m_skybox.vertex_shader = "shaders/Skybox.vert"; m_skybox.fragment_shader = "shaders/Skybox.frag";
		scene.instantiate("Model", m_skybox, t_skybox);

		//Generic scene lighting
		scene.instantiate("AmbientLight", AmbientLightComponent{ {1, 147.0 / 255.0, 41.0 / 255.0}, 0.01 });
		scene.instantiate("DirectionalLight", DirectionalLightComponent{ {0, 0, 0}, 0, {0,0,0} });

		//Add keys to door, depending on number of keys collected
		ModelComponent m_key; m_key.model_file = "models/Key/Key_B_02.obj";

		for (int i = 0; i < keys_collected; i++)
		{
			TransformComponent t; t.scale = { 0.5, 0.5, 0.5 }; t.rotation = { 90, 180, 180 };

			if (i < 3)
				t.position = { -40, 10 + i * 5.0, 10 };
			else
				t.position = { -40, 10 + (i - 3) * 5.0, -10 };

			scene.instantiate("Model", t, ModelComponent{ "models/Key/Key_B_02.obj" });
		}
	}
}
