#pragma once

#include "../Components.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

struct Mesh
{
	GLuint vao;
	std::vector<GLuint> mesh_starts;
	std::vector<GLuint> mesh_sizes;
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
	GLuint vbo;
	std::vector<VertexData> vbo_data;

public:
	Model();
	~Model();

	Mesh load_model(std::string file);
};

