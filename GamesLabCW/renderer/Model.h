#pragma once

#include "../Components.h"

#include <map>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Texture.h"
#include "VBO.h"
#include "../Math3D.h"

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

	struct VertexBoneData
	{
		unsigned int ids[4];
		float weights[4];

		VertexBoneData()
		{
			Reset();
		}

		void Reset()
		{
			memset(ids, 0, 4 * sizeof(ids[0]));
			memset(weights, 0, 4 * sizeof(weights[0]));
		}

		void AddBoneData(unsigned int BoneID, float Weight)
		{
			for (unsigned int i = 0; i < 4; i++) {
				if (weights[i] == 0.0) {
					ids[i] = BoneID;
					weights[i] = Weight;
					return;
				}

			}
			assert(0);
		}
	};

	struct BoneInfo
	{
		Matrix4f finalTransformation; // Final transformation to apply to vertices 
		Matrix4f boneOffset; // Initial offset from local to bone space.

		BoneInfo()
		{
			boneOffset.SetZero();
			finalTransformation.SetZero();
		}
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

		// Bone related
		const aiScene* scene;
		std::vector<VertexBoneData> bones;
		std::vector<BoneInfo> boneInfos;
		std::map<std::string, unsigned int> boneMapper;
		unsigned int boneCount = 0;
		Matrix4f globalTransform;
		Matrix4f globalInverseTransform;

		// Texture loading
		std::vector<GLuint> materialIDs; // Diffuse, normal etc. maps are all recorded in the same group of materials and have to be indexed
		std::vector<Texture> diffuseMaps; // AKA textures
		std::vector<Texture> normalMaps;

		// Switches
		bool isTextured = false;
		bool isNormalMapped = false;

		void Model::loadMeshes(const aiScene *scene);
		void Model::loadMaterials(const aiScene *scene, std::string filePath);
		void Model::createTexture(int materialIndex, std::string path, std::vector<Texture> &textures, std::vector<GLuint> &materialMapper);
		void Model::setupBuffers();
		void Model::readNodeHierarchy(float animationTime, const aiNode* node, const Matrix4f& ParentTransform);
		void Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		void Model::CalcInterpolatedTranslation(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
		unsigned int Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		unsigned int Model::FindTranslation(float AnimationTime, const aiNodeAnim* pNodeAnim);
		//Matrix4f Model::InitTranslationTransform(float x, float y, float z);
		void Model::updateVertices(); // Only necessary for CPU bone animation

		/*glm::mat4 MatAssimpToGLM(aiMatrix3x3 mat) 
		{ 
			glm::mat4 m;
			m[0][0] = mat.a1; m[0][1] = mat.a2; m[0][2] = mat.a3; m[0][3] = 0.0f;
			m[1][0] = mat.b1; m[1][1] = mat.b2; m[1][2] = mat.b3; m[1][3] = 0.0f;
			m[2][0] = mat.c1; m[2][1] = mat.c2; m[2][2] = mat.c3; m[2][3] = 0.0f;
			m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
			return m;
		}

		glm::mat4 MatAssimpToGLM(aiMatrix4x4 mat)
		{
			glm::mat4 m;
			m[0][0] = mat.a1; m[0][1] = mat.a2; m[0][2] = mat.a3; m[0][3] = mat.a4;
			m[1][0] = mat.b1; m[1][1] = mat.b2; m[1][2] = mat.b3; m[1][3] = mat.b4;
			m[2][0] = mat.c1; m[2][1] = mat.c2; m[2][2] = mat.c3; m[2][3] = mat.c4;
			m[3][0] = mat.d1; m[3][1] = mat.d2; m[3][2] = mat.d3; m[3][3] = mat.d4;
			return m;
		}*/

		std::string Model::stripPath(std::string path);
		glm::mat4 Model::Matrix4fToGLM(Matrix4f mat);
	public:
		Model::Model(std::string path);

		void Model::Render(GLuint shaderProgram);
		void Model::Animate(double time);

		const bool Model::IsTextured() { return isTextured; }
		const bool Model::IsNormalMapped() { return isNormalMapped; }
		const bool Model::IsAnimated() { return bones.size() > 0; }
	};
}

