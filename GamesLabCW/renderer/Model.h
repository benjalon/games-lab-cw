#pragma once

#include "../Components.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Texture.h"
#include "VBO.h"

namespace game
{
	/*
		Stores per-vertex information on the model in a convenient object to easily pass to the buffers.
	*/
	struct VertexData
	{
		aiVector3D pos;
		aiVector3D uv;
		aiVector3D normal;
		aiVector3D tangent;
	};

	/*
		Each Model represents one Assimp scene, which can contain one or more mesh objects. Model unravels
		all of this to keep a 1:1 relationship with the files which are loaded in.
	*/
	class Model
	{
	private:
		// Buffer objects
		GLuint vao;
		VBO vbo; // Vertices
		VBO ebo; // Indices

		// Mesh-related
		std::vector<GLuint> base_vertex;
		std::vector<GLuint> base_index;
		std::vector<GLuint> index_count;
		std::vector<GLuint> vertex_count;

		// Textures and maps
		std::vector<GLuint> materials;
		size_t num_materials;

		std::vector<Texture> textures;
		std::vector<Texture> normalMaps;

		// Switches
		bool isTextured = false;
		bool isNormalMapped = false;
		bool isAnimated = false;
	public:
		Model::Model(std::string file);

		void Model::Render(GLuint shader);

		const bool Model::IsTextured() { return isTextured; }
		const bool Model::IsNormalMapped() { return isNormalMapped; }
		const bool Model::IsAnimated() { return isAnimated; }
	};
}