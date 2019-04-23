#pragma once

#include "../Components.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "VBO.h"

namespace game
{
	struct Mesh
	{
		GLuint vao;
		std::vector<GLuint> base_vertex;
		std::vector<GLuint> base_index;
		std::vector<GLuint> index_count;
		std::vector<GLuint> vertex_count;
		std::vector<GLuint> materials;
		size_t num_materials;

		bool textured;
		bool normal_mapped;
		bool hasBones;
	};

	struct VertexData
	{
		aiVector3D pos;
		aiVector3D uv;
		aiVector3D normal;
		aiVector3D tangent;
	};

	class Model
	{
	private:
		VBO vbo;
		VBO ebo;
	public:
		Mesh load_model(std::string file);
	};
}