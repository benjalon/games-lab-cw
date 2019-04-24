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
		Stores per-vertex information on the model in a convenient object for easy passing to buffers.
	*/
	struct VertexData
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
	};

	/*
		Each Model represents one Assimp scene, which can contain one or more Assimp mesh objects. Model unravels
		all of this to keep a 1:1 relationship with the files which are loaded in.
	*/
	class Model
	{
	private:
		// Buffer objects
		GLuint vao;
		VBO vbo; // Vertex buffer: stores vertex related stuff like positions and normals
		VBO ebo; // Indices: tracks the draw order so that vertices aren't reused

		// Buffer data: all of the vertices (including positions, uv, normal data etc.) and indices in the model
		std::vector<VertexData> vertices;
		std::vector<unsigned int> indices;

		// Drawing related stuff: since one model can have multiple meshes, we need to track where to start drawing from and for how long
		std::vector<GLuint> baseVertices;
		std::vector<GLuint> baseIndices;
		std::vector<GLuint> indexCounts;

		// Texture loading
		std::vector<GLuint> materialIDs; // Diffuse, normal etc. maps are all recorded in the same group of materials and have to be indexed
		std::vector<Texture> diffuseMaps; // AKA textures
		std::vector<Texture> normalMaps;

		// Switches
		bool isTextured = false;
		bool isNormalMapped = false;
		bool isAnimated = false;

		void Model::loadMeshes(const aiScene *scene);
		void Model::loadMaterials(const aiScene *scene, std::string filePath);
		void Model::createTexture(int materialIndex, std::string path, std::vector<Texture> &textures, std::vector<GLuint> &materialMapper);
		void Model::setupBuffers();

		std::string stripPath(std::string path);
	public:
		Model::Model(std::string path);

		void Model::Render(GLuint shaderProgram);

		const bool Model::IsTextured() { return isTextured; }
		const bool Model::IsNormalMapped() { return isNormalMapped; }
		const bool Model::IsAnimated() { return isAnimated; }
	};
}