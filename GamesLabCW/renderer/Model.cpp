#include "Model.h"



Model::Model()
{
}


Model::~Model()
{
}


//Utility function to remove the final path node from the given file path
std::string strip_last_path(std::string path)
{
	std::string dir = "";
	for (int i = (int)path.size() - 1; i >= 0; i--)
		if (path[i] == '\\' || path[i] == '/')
		{
			dir = path.substr(0, i + 1);
			break;
		}
	return dir;
}

Mesh Model::load_model(std::string file)
{
	//Load model from file
	static Assimp::Importer imp;
	const aiScene *scene = imp.ReadFile(file,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_LimitBoneWeights
	);

	//Abort if unsuccessful
	std::cout << (scene ? "Loaded model " : "Could not load model ") << file << std::endl;
	if (!scene) return Mesh();

	Mesh m = Mesh();

	glGenVertexArrays(1, &m.vao);
	glBindVertexArray(m.vao);

	//Ensure VBO has been generated
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	//Load all model data from aiScene
	size_t totalVertices = 0;
	size_t currentVertices = 0;
	size_t currentIndices = 0;
	size_t prevTotal;

	/*m_GlobalInverseTransform = Mat4AssimpToGLM(scene->mRootNode->mTransformation);
	m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);*/

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[i];
		totalVertices += mesh->mNumVertices;
	}

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[i];
		m.materials.push_back(mesh->mMaterialIndex);
		m.base_vertex.push_back(currentVertices);
		m.base_index.push_back(currentIndices);
		m.index_count.push_back(mesh->mNumFaces * 3);

		for (size_t k = 0; k < mesh->mNumVertices; k++) {
			aiVector3D pos = (mesh->HasPositions()) ?
				mesh->mVertices[k] :
				aiVector3D(1.0f, 1.0f, 1.0f);
			//WARNING - SHOULD THIS BE aiVector2D ???
			aiVector3D uv = (mesh->GetNumUVChannels() > 0) ?
				mesh->mTextureCoords[0][k] :
				aiVector3D(1.0f, 1.0f, 1.0f);
			aiVector3D normal = (mesh->HasNormals()) ?
				mesh->mNormals[k] :
				aiVector3D(1.0f, 1.0f, 1.0f);
			aiVector3D tangent = (mesh->HasTangentsAndBitangents()) ?
				mesh->mTangents[k] :
				aiVector3D(1.0f, 1.0f, 1.0f);

			VertexData vertexData;
			vertexData.pos = pos;
			vertexData.uv = uv;
			vertexData.normal = normal;
			vertexData.tangent = tangent;
			vbo_data.push_back(vertexData);
		}

		prevTotal = currentVertices;
		currentVertices += mesh->mNumVertices;
		currentIndices += mesh->mNumFaces * 3;
		m.vertex_count.push_back(currentVertices);

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


		// Populate the indices array 
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& Face = mesh->mFaces[i];
			assert(Face.mNumIndices == 3);
			ebo_data.push_back(Face.mIndices[0]);
			ebo_data.push_back(Face.mIndices[1]);
			ebo_data.push_back(Face.mIndices[2]);
		}

	}

	//m.num_materials = scene->mNumMaterials;
	//std::vector<size_t> materialRemap(m.num_materials);

	//for (size_t i = 0; i < m.num_materials; i++)
	//{
	//	const aiMaterial *material = scene->mMaterials[i];
	//	int texIndex = 0;
	//	aiString path;

	//	if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &path) == AI_SUCCESS)
	//	{
	//		m.textured = true;
	//		std::string fullPath = strip_last_path(file) + path.data;

	//		int texFound = -1;
	//		for (int j = 0; j < (int)textures.size(); j++)
	//			if (fullPath == textures[j].path)
	//			{
	//				texFound = j;
	//				break;
	//			}
	//		if (texFound != -1)
	//			materialRemap[i] = texFound;
	//		else
	//		{
	//			Texture t(fullPath, true);
	//			materialRemap[i] = textures.size();
	//			textures.push_back(t);
	//		}
	//	}
	//}


	//for (size_t i = 0; i < m.num_materials; i++)
	//{
	//	const aiMaterial *material = scene->mMaterials[i];
	//	int texIndex = 0;
	//	aiString path;

	//	if (material->GetTexture(aiTextureType_HEIGHT, texIndex, &path) == AI_SUCCESS) // Note assimp treats the way .obj stores normalmaps as heightmaps
	//	{
	//		m.normal_mapped = true;
	//		std::string fullPath = strip_last_path(file) + path.data;

	//		int normalFound = -1;
	//		for (int j = 0; j < (int)normalMaps.size(); j++)
	//			if (fullPath == normalMaps[j].path)
	//			{
	//				normalFound = j;
	//				break;
	//			}
	//		if (normalFound != -1)
	//			materialRemap[i] = normalFound;
	//		else
	//		{
	//			Texture t(fullPath, true);
	//			materialRemap[i] = normalMaps.size();
	//			normalMaps.push_back(t);
	//		}
	//	}
	//}

	//for (int i = 0; i < (int)m.mesh_sizes.size(); i++)
	//{
	//	int o = m.materials[i];
	//	m.materials[i] = (GLuint)materialRemap[o];
	//}

	// Vertex-related

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(VertexData), &vbo_data[0], GL_STATIC_DRAW);

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

	// TODO: CLEAR VBO DATA

	// Index-related (to ensure correct draw order)
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ebo_data[0]) * ebo_data.size(), &ebo_data[0], GL_STATIC_DRAW);

	ebo_data.clear();

	return m;
}
