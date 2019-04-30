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

		if (scene->HasAnimations())
		{
			globalInverseTransform = MatAssimpToGLM(scene->mRootNode->mTransformation);
			glm::inverse(globalInverseTransform);
		}

		loadMeshes(scene);
		loadMaterials(scene, modelPath);
		setupBuffers();
	}

	void Model::loadMeshes(const aiScene *scene)
	{
		size_t currentVertices = 0;
		size_t currentIndices = 0;

		// Prepare bones storage if needed
		if (scene->HasAnimations()) {
			size_t totalVertices = 0;
			for (size_t i = 0; i < scene->mNumMeshes; i++) {
				totalVertices += scene->mMeshes[i]->mNumVertices;
			}
			bones.resize(totalVertices);
		}

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			// Store information for this mesh
			materialIDs.push_back(mesh->mMaterialIndex);
			baseVertices.push_back(currentVertices);
			baseIndices.push_back(currentIndices);
			indexCounts.push_back(mesh->mNumFaces * 3);

			// Populate vertex buffer object
			for (size_t j = 0; j < mesh->mNumVertices; j++) {
				// Get all per-vertex information needed by the shaders
				aiVector3D pos = (mesh->HasPositions()) ? mesh->mVertices[j] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D uv = (mesh->GetNumUVChannels() > 0) ? mesh->mTextureCoords[0][j] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D normal = (mesh->HasNormals()) ? mesh->mNormals[j] : aiVector3D(1.0f, 1.0f, 1.0f);
				aiVector3D tangent = (mesh->HasTangentsAndBitangents()) ? mesh->mTangents[j] : aiVector3D(1.0f, 1.0f, 1.0f);

				// Package the information and add to the buffer object
				VertexData vertexData;
				vertexData.pos = glm::vec3(pos.x, pos.y, pos.z);
				vertexData.uv = glm::vec2(uv.x, uv.y);
				vertexData.normal = glm::vec3(normal.x, normal.y, normal.z);
				vertexData.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
				vertices.push_back(vertexData);
			}
			currentVertices += mesh->mNumVertices;

			// Load bones if this mesh part has them
			if (mesh->HasBones()) {
				for (unsigned int j = 0; j < mesh->mNumBones; j++) 
				{
					unsigned int boneID = 0;

					std::string BoneName(mesh->mBones[j]->mName.data);

					if (boneMapper.find(BoneName) == boneMapper.end()) 
					{
						boneID = boneCount;
						boneCount++;

						BoneInfo boneInfo;
						boneInfos.push_back(boneInfo);
					}
					else {
						boneID = boneMapper[BoneName];
					}

					boneMapper[BoneName] = boneID;
					boneInfos[boneID].boneOffset = MatAssimpToGLM(mesh->mBones[j]->mOffsetMatrix);

					for (unsigned int k = 0; k < mesh->mBones[j]->mNumWeights; k++) 
					{
						unsigned int VertexID = baseVertices[i] + mesh->mBones[j]->mWeights[k].mVertexId;
						float Weight = mesh->mBones[j]->mWeights[k].mWeight;
						bones[VertexID].AddBoneData(boneID, Weight);
					}
				}
			}

			// Populate element buffer object (indices)
			for (size_t j = 0; j < mesh->mNumFaces; j++) {
				const aiFace &face = mesh->mFaces[j];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}
			currentIndices += mesh->mNumFaces * 3;
		}
	}

	void Model::loadMaterials(const aiScene *scene, std::string modelPath)
	{
		std::vector<GLuint> materialMapper(scene->mNumMaterials);

		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial *material = scene->mMaterials[i];

			int materialIndex = 0;
			aiString materialPath;

			int foundIndex = -1;
			if (material->GetTexture(aiTextureType_DIFFUSE, materialIndex, &materialPath) == AI_SUCCESS)
			{
				isTextured = true;
				createTexture(i, stripPath(modelPath) + materialPath.data, diffuseMaps, materialMapper);
			}
			else if (material->GetTexture(aiTextureType_HEIGHT, materialIndex, &materialPath) == AI_SUCCESS) // Note that assimp treats the way .obj stores normalmaps as heightmaps
			{
				isNormalMapped = true;
				createTexture(i, stripPath(modelPath) + materialPath.data, normalMaps, materialMapper);
			}
		}

		// Since materials are shared between all the mesh parts in the model, we have to remap the IDs
		for (size_t i = 0; i < materialIDs.size(); i++)
		{
			materialIDs[i] = materialMapper[materialIDs[i]];
		}
	}

	void Model::createTexture(int materialIndex, std::string path, std::vector<Texture> &textures, std::vector<GLuint> &materialMapper)
	{
		int foundIndex = -1;
		for (size_t i = 0; i < textures.size(); i++)
		{
			if (path == textures[i].path)
			{
				foundIndex = i;
				break;
			}
		}

		// Since materials are shared between all the mesh parts in the model, the mapper allows us to index the correct ones later
		if (foundIndex != -1)
		{
			materialMapper[materialIndex] = foundIndex;
		}
		else
		{
			Texture texture(path, true);
			materialMapper[materialIndex] = textures.size();
			textures.push_back(texture);
		}
	}

	void Model::setupBuffers()
	{
		// Prepare the buffer objects
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		vbo = VBO();
		vbo.create();

		ebo = VBO(GL_ELEMENT_ARRAY_BUFFER);
		ebo.create();

		vbo.add_data(&vertices[0], sizeof(VertexData) * vertices.size());
		ebo.add_data(&indices[0], sizeof(unsigned int) * indices.size());

		// Vertex-related
		vbo.bind();
		vbo.upload(GL_STATIC_DRAW);

		//Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, pos));
		//Texture coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (GLvoid*)offsetof(VertexData, uv));
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

	void Model::Render(GLuint shaderProgram)
	{
		// Drawing stuff
		glBindVertexArray(vao);

		//Draw the model
		for (size_t i = 0; i < baseVertices.size(); i++)
		{
			int offset = 0;

			//Bind texture if the model has them
			if (isTextured)
			{
				Texture &diffuseMap = diffuseMaps[materialIDs[i]];
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, diffuseMap.handle);
				glUniform1i(glGetUniformLocation(shaderProgram, "texSampler"), 0);

				offset++;
			}

			// Bind model normal maps
			if (isNormalMapped)
			{
				Texture &normalMap = normalMaps[materialIDs[i]];
				glActiveTexture(GL_TEXTURE0 + offset);
				glBindTexture(GL_TEXTURE_2D, normalMap.handle);
				glUniform1i(glGetUniformLocation(shaderProgram, "normalSampler"), offset);

				offset++;
			}

			glDrawElementsBaseVertex(GL_TRIANGLES, indexCounts[i], GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * baseIndices[i]), baseVertices[i]);
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