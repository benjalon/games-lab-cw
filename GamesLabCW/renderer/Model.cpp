#include "Model.h"

#include <iostream>

namespace game
{
	Model::Model(std::string modelPath)
	{
		// Have Assimp load and read the model file
		static Assimp::Importer imp;
		scene = imp.ReadFile(modelPath,
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_GenSmoothNormals |
			aiProcess_LimitBoneWeights
		);

		// Abort if unsuccessful
		std::cout << (scene ? "Loaded model " : "Could not load model ") << modelPath << std::endl;
		if (!scene) return;

		if (scene->HasAnimations())
		{
			globalInverseTransform = scene->mRootNode->mTransformation;
			globalInverseTransform.Inverse();
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
					boneInfos[boneID].boneOffset = mesh->mBones[j]->mOffsetMatrix;

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
		vertices_backup = vertices;
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

	void Model::Animate(double time)
	{
		vertices = vertices_backup;

		Matrix4f identity;
		identity.InitIdentity();

		float ticksPerSecond = scene->mAnimations[0]->mTicksPerSecond;
		float tickTime = time * ticksPerSecond;
		float animationTime = fmod(tickTime, scene->mAnimations[0]->mDuration);

		readNodeHierarchy(21, scene->mRootNode, identity);

		std::vector<Matrix4f> transforms(boneCount);

		// Populates transforms vector with new bone transformation matrices. 
		for (unsigned int i = 0; i < boneCount; i++) {
			transforms[i] = boneInfos[i].finalTransformation;
		}

		for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			auto currentBone = bones[i];

			glm::mat4 boneTransform = Matrix4fToGLM(transforms[currentBone.ids[0]]) * currentBone.weights[0];
			boneTransform += Matrix4fToGLM(transforms[currentBone.ids[1]]) * currentBone.weights[1];
			boneTransform += Matrix4fToGLM(transforms[currentBone.ids[2]]) * currentBone.weights[2];
			boneTransform += Matrix4fToGLM(transforms[currentBone.ids[3]]) * currentBone.weights[3];
			boneTransform = glm::transpose(boneTransform);

			if (boneTransform[0][0] != 0 && boneTransform[0][1] != 0 && boneTransform[0][2] != 0)
			{
				int ff = 0;
			}

			vertices[i].pos = glm::vec3(boneTransform * glm::vec4(vertices[i].pos, 1));
		}

		updateVertices();
	}

	void Model::readNodeHierarchy(float animationTime, const aiNode* node, const Matrix4f& ParentTransform)
	{
		Matrix4f identity;
		identity.InitIdentity();

		std::string nodeName(node->mName.data);

		const aiAnimation* pAnimation = scene->mAnimations[0];

		Matrix4f nodeTransform(node->mTransformation);

		const aiNodeAnim* nodeAnimation = NULL;

		for (unsigned i = 0; i < pAnimation->mNumChannels; i++) {
			const aiNodeAnim* nodeAnimationID = pAnimation->mChannels[i];

			if (std::string(nodeAnimationID->mNodeName.data) == nodeName) {
				nodeAnimation = nodeAnimationID;
			}
		}

		if (nodeAnimation) {

			//// Interpolate scaling and generate scaling transformation matrix
			//aiVector3D Scaling;
			//CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			//Matrix4f ScalingM;
			//ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion rot;
			CalcInterpolatedRotation(rot, animationTime, nodeAnimation);
			Matrix4f RotationM = Matrix4f(rot.GetMatrix());

			// Interpolate translation and generate translation transformation matrix
			aiVector3D tl;
			CalcInterpolatedTranslation(tl, animationTime, nodeAnimation);
			Matrix4f TranslationM;
			TranslationM.InitTranslationTransform(tl.x, tl.y, tl.z);

			// Combine the above transformations
			nodeTransform = TranslationM * RotationM;/* *ScalingM;*/
		}

		Matrix4f GlobalTransformation = ParentTransform * nodeTransform;

		// Apply the final transformation to the indexed bone in the array. 
		if (boneMapper.find(nodeName) != boneMapper.end()) {
			unsigned int BoneIndex = boneMapper[nodeName];
			boneInfos[BoneIndex].finalTransformation = globalInverseTransform * GlobalTransformation *
				boneInfos[BoneIndex].boneOffset;

			std::cout << boneInfos[BoneIndex].finalTransformation.m[0][0] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[0][1] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[0][2] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[0][3] << "\n";

			std::cout << boneInfos[BoneIndex].finalTransformation.m[1][0] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[1][1] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[1][2] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[1][3] << "\n";

			std::cout << boneInfos[BoneIndex].finalTransformation.m[2][0] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[2][1] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[2][2] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[2][3] << "\n";

			std::cout << boneInfos[BoneIndex].finalTransformation.m[3][0] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[3][1] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[3][2] << ", "
				<< boneInfos[BoneIndex].finalTransformation.m[3][3] << "\n";
		}

		// Do the same for all the node's children. 
		for (unsigned i = 0; i < node->mNumChildren; i++) {
			readNodeHierarchy(animationTime, node->mChildren[i], GlobalTransformation);
		}
	}

	void Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}
		// Obtain the current rotation keyframe. 
		unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);

		// Calculate the next rotation keyframe and check bounds. 
		unsigned int NextRotationIndex = (RotationIndex + 1);
		assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);

		// Calculate delta time, i.e time between the two keyframes.
		float DeltaTime = pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime;

		// Calculate the elapsed time within the delta time.  
		float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
		//assert(Factor >= 0.0f && Factor <= 1.0f);

		// Obtain the quaternions values for the current and next keyframe. 
		const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
		const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;

		// Interpolate between them using the Factor. 
		aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);

		// Normalise and set the reference. 
		Out = Out.Normalize();
	}

	void Model::CalcInterpolatedTranslation(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumPositionKeys == 1) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}


		unsigned int PositionIndex = FindTranslation(AnimationTime, pNodeAnim);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		float DeltaTime = pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime;
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		//assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;

		aiVector3D Delta = End - Start;
		Out = Start + Factor * Delta;
	}

	unsigned int Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// Check if there are rotation keyframes. 
		assert(pNodeAnim->mNumRotationKeys > 0);

		// Find the rotation key just before the current animation time and return the index. 
		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}
		assert(0);

		return 0;
	}

	unsigned int Model::FindTranslation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumPositionKeys > 0);

		// Find the translation key just before the current animation time and return the index. 
		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}
		assert(0);

		return 0;
	}

	/*glm::mat4 Model::InitTranslationTransform(float x, float y, float z)
	{
		glm::mat4 m = glm::mat4(1);
		m[0][3] = x;
		m[1][3] = y;
		m[2][3] = z;
		return m;
	}
*/
	void Model::updateVertices()
	{
		for (int i = 0; i < vertices.size(); i++)
		{
			std::cout << i << ": " << vertices[i].pos.x << ", " << vertices[i].pos.y << ", " << vertices[i].pos.z << "\n";
		}

		vbo.bind();
		vbo.add_data(&vertices[0], sizeof(VertexData) * vertices.size());
		vbo.upload(GL_STATIC_DRAW);
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

	glm::mat4 Model::Matrix4fToGLM(Matrix4f mat) {
		glm::mat4 mat2 = glm::mat4();
		mat2[0][0] = mat.m[0][0]; mat2[0][1] = mat.m[0][1]; mat2[0][2] = mat.m[0][2]; mat2[0][3] = mat.m[0][3];
		mat2[1][0] = mat.m[1][0]; mat2[1][1] = mat.m[1][1]; mat2[1][2] = mat.m[1][2]; mat2[1][3] = mat.m[1][3];
		mat2[2][0] = mat.m[2][0]; mat2[2][1] = mat.m[2][1]; mat2[2][2] = mat.m[2][2]; mat2[2][3] = mat.m[2][3];
		mat2[3][0] = mat.m[3][0]; mat2[3][1] = mat.m[3][1]; mat2[3][2] = mat.m[3][2]; mat2[3][3] = mat.m[3][3];

		return mat2;
	}
}