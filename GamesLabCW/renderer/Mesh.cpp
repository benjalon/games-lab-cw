#include "Mesh.h"

namespace game {

	Mesh::Mesh(GLuint vao, BO vbo, BO ibo, aiMesh* mesh, aiMaterial* material)
	{
		LoadVertices(mesh);
		LoadIndices(mesh);
		//LoadMaterials(material);

		UploadBuffers(vao, vbo, ibo);
	}

	void Mesh::LoadVertices(aiMesh* mesh)
	{
		vertices.reserve(mesh->mNumVertices);

		for (size_t i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex v;
			v.pos = (mesh->HasPositions()) ? mesh->mVertices[i] : aiVector3D(1.0f, 1.0f, 1.0f);
			v.uv = (mesh->GetNumUVChannels() > 0) ? aiVector2D(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : aiVector2D(1.0f, 1.0f);
			v.normal = (mesh->HasNormals()) ? mesh->mNormals[i] : aiVector3D(1.0f, 1.0f, 1.0f);
			v.tangent = (mesh->HasTangentsAndBitangents()) ? mesh->mTangents[i] : aiVector3D(1.0f, 1.0f, 1.0f);
			vertices.push_back(v);
		}
	}

	void Mesh::LoadIndices(aiMesh* mesh)
	{
		indices.reserve(mesh->mNumVertices);

		for (size_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}

	void Mesh::UploadBuffers(GLuint vao, BO vbo, BO ibo)
	{
		// --- Add data to buffers ---


		// Vertex data
		vbo.bind();
		vbo.add_data(&vertices[0], vertices.size() * sizeof(vertices[0]));
		vbo.upload(GL_STATIC_DRAW);

		// Index data
		ibo.bind();
		ibo.add_data(&indices[0], indices.size() * sizeof(GLuint));
		ibo.upload(GL_STATIC_DRAW);


		// --- Create VAO and store the buffer states / memory alignments ---


		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		// Vertex related
		vbo.bind();
		// Position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, pos));
		// UV
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, uv));
		// Normal
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		// Tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

		// Index related
		ibo.bind();

		// Unbind
		glBindVertexArray(0);
	}

	//void Mesh::LoadMaterials(aiMaterial* material)
	//{
	//	if (!material)
	//	{
	//		return; // No internally stored textures
	//	}

	//	std::vector<Texture> diffuse_maps = LoadTextureFromFile(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
	//	bool exist = false;
	//	for (int i = 0; (i < textures.size()) && (diffuse_maps.size() != 0); i++)
	//	{
	//		if (textures[i].path == diffuse_maps[0].path)
	//		{
	//			exist = true;
	//		}
	//	}
	//	if (!exist && diffuse_maps.size() != 0) textures.push_back(diffuse_maps[0]);

	//	/*std::vector<Texture> specular_maps = LoadMaterialTexture(material, aiTextureType_SPECULAR, "texture_specular");
	//	exist = false;
	//	for (int i = 0; (i < textures.size()) && (specular_maps.size() != 0); i++)
	//	{
	//		if (textures[i].path == specular_maps[0].path)
	//		{
	//			exist = true;
	//		}
	//	}
	//	if (!exist  && specular_maps.size() != 0) textures.push_back(specular_maps[0]);*/
	//}

	//std::vector<Texture> Mesh::LoadTextureFromFile(aiMaterial* material, aiTextureType aitype, TextureType type)
	//{
	//	std::vector<Texture> textures;

	//	//aiString path;

	//	//for (size_t i = 0; i < material->GetTextureCount(aitype); i++)
	//	//{
	//	//	if (material->GetTexture(aitype, i, &path) == AI_SUCCESS)
	//	//	{
	//	//		std::string fullPath = strip_last_path(file) + path.data;

	//	//		int texFound = -1;
	//	//		for (int j = 0; j < (int)textures.size(); j++)
	//	//			if (fullPath == textures[j].path)
	//	//			{
	//	//				texFound = j;
	//	//				break;
	//	//			}
	//	//		if (texFound != -1)
	//	//			materialRemap[i] = texFound;
	//	//		else
	//	//		{
	//	//			materialRemap[i] = textures.size();
	//	//			textures.push_back(t);
	//	//		}
	//	//	}
	//	//}

	//	//switch (aitype)
	//	//{
	//	//	case aiTextureType_DIFFUSE:
	//	//		textured = true;
	//	//		break;
	//	//	case aiTextureType_HEIGHT:
	//	//	case aiTextureType_NORMALS:
	//	//		normal_mapped = true;
	//	//		break;
	//	//	default:
	//	//		break;
	//	//}

	//	//for (size_t i = 0; i < material->GetTextureCount(aitype); i++)
	//	//{
	//	//	Texture t(fullPath, true);
	//	//	aiString ai_str;
	//	//	material->GetTexture(aitype, i, &ai_str);

	//	//	std::string filename = std::string(ai_str.C_Str());
	//	//	filename = directory + '/' + filename;

	//	//	Texture texture;
	//	//	texture.id = Triangle::loadImageToTexture(filename.c_str()); // return prepaired openGL texture
	//	//	texture.type = type_name;
	//	//	texture.path = ai_str;
	//	//	textures.push_back(texture);
	//	//}
	//	return textures;
	//}


	////Utility function to remove the final path node from the given file path
	//std::string strip_last_path(std::string path)
	//{
	//	std::string dir = "";
	//	for (int i = (int)path.size() - 1; i >= 0; i--)
	//		if (path[i] == '\\' || path[i] == '/')
	//		{
	//			dir = path.substr(0, i + 1);
	//			break;
	//		}
	//	return dir;
	//}
}