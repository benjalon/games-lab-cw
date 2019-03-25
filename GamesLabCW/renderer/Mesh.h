#pragma once

#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Components.h"

#include "Texture.h"
#include "BO.h"

namespace game
{

	// Mesh
	struct Vertex
	{
		aiVector3D pos;
		aiVector2D uv;
		aiVector3D normal;
		aiVector3D tangent;
	};

	struct Mesh 
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		//std::vector<GLuint> materials; // Materials are internally stored textures
		//std::vector<Texture> textures; // Textures are externally stored textures

		Mesh(GLuint vao, BO vbo, BO ibo, aiMesh* mesh, aiMaterial* material);
		void LoadVertices(aiMesh* mesh);
		void LoadIndices(aiMesh* mesh);

		void UploadBuffers(GLuint vao, BO vbo, BO ibo);


		/*void LoadMaterials(aiMaterial* material);

	

		std::vector<Texture> LoadTextureFromFile(aiMaterial* material, aiTextureType aitype, TextureType type);*/
	};
}
