#include "Model.h"

namespace game
{
	Model::Model(std::string modelPath)
	{
		// Have Assimp load and read the model file
		static Assimp::Importer imp;
		const aiScene *scene = imp.ReadFile(modelPath,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_LimitBoneWeights
		);

		// Abort if unsuccessful
		std::cout << (scene ? "Loaded model " : "Could not load model ") << modelPath << std::endl;
		if (!scene) return;

		// Prepare the buffer objects
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		vbo = VBO();
		vbo.create();

		ebo = VBO(GL_ELEMENT_ARRAY_BUFFER);
		ebo.create();

		/*m_GlobalInverseTransform = Mat4AssimpToGLM(scene->mRootNode->mTransformation);
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);*/

		loadMeshes(scene);
		loadMaterials(scene, modelPath);

		// Vertex-related
		vbo.bind();
		vbo.upload(GL_STATIC_DRAW);

		//Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
		//Texture coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
		//Normal vectors
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, normal));
		//Tangent vectors
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, tangent));

		// Index-related (to ensure correct draw order)
		ebo.bind();
		ebo.upload(GL_STATIC_DRAW);
	}

	void Model::loadMeshes(const aiScene *scene)
	{
		size_t currentVertices = 0;
		size_t currentIndices = 0;

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			// Store information for this mesh
			materials.push_back(mesh->mMaterialIndex);
			baseVertex.push_back(currentVertices);
			baseIndex.push_back(currentIndices);
			indexCount.push_back(mesh->mNumFaces * 3);

			// Populate vertex buffer object
			for (size_t k = 0; k < mesh->mNumVertices; k++) {
				// Get all per-vertex information needed by the shaders
				aiVector3D pos = (mesh->HasPositions()) ? mesh->mVertices[k] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D uv = (mesh->GetNumUVChannels() > 0) ? mesh->mTextureCoords[0][k] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D normal = (mesh->HasNormals()) ? mesh->mNormals[k] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D tangent = (mesh->HasTangentsAndBitangents()) ? mesh->mTangents[k] : aiVector3D(1.0f, 1.0f, 1.0f);

				// Package the information and add to the buffer object
				VertexData vertexData;
				vertexData.pos = glm::vec3(pos.x, pos.y, pos.z);
				vertexData.uv = glm::vec2(uv.x, uv.y);
				vertexData.normal = glm::vec3(normal.x, normal.y, normal.z);
				vertexData.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
				vbo.add_data(&vertexData, sizeof(VertexData));
			}

			// Populate element buffer object (indices)
			for (size_t k = 0; k < mesh->mNumFaces; k++) {
				const aiFace& Face = mesh->mFaces[k];
				assert(Face.mNumIndices == 3);
				ebo.add_data(&Face.mIndices[0], sizeof(unsigned int));
				ebo.add_data(&Face.mIndices[1], sizeof(unsigned int));
				ebo.add_data(&Face.mIndices[2], sizeof(unsigned int));
			}

			//if (mesh->HasBones())
			//{
			//	m.hasBones = true;

			//	BoneAnimation &b = animations.emplace(file, BoneAnimation()).first->second;

			//	// Copy all the data from the scene 
			//	b.scene = scene;
			//	// End of horrible copy code

			//	bones.resize(totalVertices);

			//	// Loop through all bones in the Assimp mesh.
			//	for (unsigned int i = 0; i < mesh->mNumBones; i++) {

			//		unsigned int BoneIndex = 0;

			//		// Obtain the bone name.
			//		std::string BoneName(mesh->mBones[i]->mName.data);

			//		// If bone isn't already in the map. 
			//		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {

			//			// Set the bone ID to be the current total number of bones. 
			//			BoneIndex = m_NumBones;

			//			// Increment total bones. 
			//			m_NumBones++;

			//			// Push new bone info into bones vector. 
			//			BoneInfo bi;
			//			m_BoneInfo.push_back(bi);
			//		}
			//		else {
			//			// Bone ID is already in map. 
			//			BoneIndex = m_BoneMapping[BoneName];
			//		}

			//		m_BoneMapping[BoneName] = BoneIndex;

			//		// Obtains the offset matrix which transforms the bone from mesh space into bone space. 
			//		m_BoneInfo[BoneIndex].BoneOffset = Mat4AssimpToGLM(mesh->mBones[i]->mOffsetMatrix);


			//		// Iterate over all the affected vertices by this bone i.e weights. 
			//		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {

			//			// Obtain an index to the affected vertex within the array of vertices.
			//			unsigned int VertexID = prevTotal + mesh->mBones[i]->mWeights[j].mVertexId;
			//			// The value of how much this bone influences the vertex. 
			//			float Weight = mesh->mBones[i]->mWeights[j].mWeight;

			//			// Insert bone data for particular vertex ID. A maximum of 4 bones can influence the same vertex. 
			//			bones[VertexID].AddBoneData(BoneIndex, Weight);
			//		}
			//	}
			//}

			currentVertices += mesh->mNumVertices;
			currentIndices += mesh->mNumFaces * 3;
		}
	}

	void Model::loadMaterials(const aiScene *scene, std::string modelPath)
	{
		materialRemap.resize(scene->mNumMaterials);

		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial *material = scene->mMaterials[i];

			int materialIndex = 0;
			aiString materialPath;

			int foundIndex = -1;
			if (material->GetTexture(aiTextureType_DIFFUSE, materialIndex, &materialPath) == AI_SUCCESS)
			{
				isTextured = true;
				createTexture(i, stripPath(modelPath) + materialPath.data, diffuseMaps);
			}
			else if (material->GetTexture(aiTextureType_HEIGHT, materialIndex, &materialPath) == AI_SUCCESS) // Note that assimp treats the way .obj stores normalmaps as heightmaps
			{
				isNormalMapped = true;
				createTexture(i, stripPath(modelPath) + materialPath.data, normalMaps);
			}
		}

		for (size_t i = 0; i < materials.size(); i++)
		{
			materials[i] = (GLuint)materialRemap[materials[i]];
		}
	}

	void Model::createTexture(int materialIndex, std::string path, std::vector<Texture> &textures)
	{
		int foundIndex = -1;
		for (size_t i = 0; i < textures.size(); i++)
		{
			Texture texture = textures[i].path;
			if (path == textures[i].path)
			{
				foundIndex = i;
				break;
			}
		}

		if (foundIndex != -1)
		{
			materialRemap[materialIndex] = foundIndex;
		}
		else
		{
			Texture t(path, true);
			materialRemap[materialIndex] = textures.size();
			textures.push_back(t);
		}
	}

	void Model::Render(GLuint shaderProgram)
	{
		// Drawing stuff
		glBindVertexArray(vao);

		//Draw the model
		for (size_t i = 0; i < baseVertex.size(); i++)
		{
			int offset = 0;

			//Bind texture if the model has them
			if (isTextured)
			{
				Texture &t = diffuseMaps[materials[i]];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, t.handle);
				glUniform1i(glGetUniformLocation(shaderProgram, "texSampler"), 0);

				offset++;
			}

			// Bind model normal maps
			if (isNormalMapped)
			{
				Texture &n = normalMaps[materials[i]];

				glActiveTexture(GL_TEXTURE0 + offset);
				glBindTexture(GL_TEXTURE_2D, n.handle);
				glUniform1i(glGetUniformLocation(shaderProgram, "normalSampler"), offset);

				offset++;
			}

			glDrawElementsBaseVertex(GL_TRIANGLES, indexCount[i], GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndex[i]), baseVertex[i]);
		}

		glBindVertexArray(0);
	}

	std::string Model::stripPath(std::string path)
	{
		std::string dir = "";
		for (int i = (int)path.size() - 1; i >= 0; i--)
		{
			if (path[i] == '\\' || path[i] == '/')
			{
				dir = path.substr(0, i + 1);
				break;
			}
		}
		return dir;
	}
}