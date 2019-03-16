/**
 * Renderer.cpp
 * Implements the functions for loading and rendering models.
 */

#define GLM_ENABLE_EXPERIMENTAL

#include "Renderer.h"

#include <map>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "VBO.h"
#include "Texture.h"

//Quick conversion to radians
#define R(x) glm::radians((float)x)

namespace game::renderer
{
	//Returns the (potentially cached) shader using the given paramaters
	GLuint get_shader(bool textured, bool normal_mapped, size_t n_ambient, size_t n_directional, size_t n_point,
		std::string vertex_shader, std::string fragment_shader)
	{
		using Args = std::tuple<bool, bool, size_t, size_t, size_t, std::string, std::string>;

		//Cache of parametrised shaders
		static std::map<Args, Shader> shaders;

		Args args = std::make_tuple(textured, normal_mapped, n_ambient, n_directional, n_point,
			vertex_shader, fragment_shader);

		//If the requested shader already exists, return it
		auto it = shaders.find(args);
		if (it != shaders.end())
			return it->second.handle();

		//Else, compile and return the new shader
		else
		{
			//Shader parameters to prepend to source
			std::string v, f;
			auto define = [](std::string &s, std::string def) { s.append("#define " + def + "\n"); };

			if (textured) define(f, "TEXTURED");
			if (normal_mapped) define(f, "NORMAL_MAPPED");
			define(f, "N_AMBIENT " + std::to_string(n_ambient));
			define(f, "N_DIRECTIONAL " + std::to_string(n_directional));
			define(f, "N_POINT " + std::to_string(n_point));

			//Create new shader
			auto &s = shaders[args];
			s.load("",
				vertex_shader.empty() ? "shaders/Passthrough.vert" : vertex_shader.c_str(),
				fragment_shader.empty() ? "shaders/ParametrisedFragment.frag" : fragment_shader.c_str(),
				v, f);
			return s.handle();
		}
	}

	//Global vertex array object
	GLuint vao;

	//Global vertex buffer object
	VBO vbo;

	//Bone buffer object
	GLuint bonebo;

	//Global textures collection
	std::vector<Texture> textures;
	std::vector<Texture> normalMaps;

	//Represents data of a mesh
	struct Mesh
	{
		std::vector<GLuint> mesh_starts;
		std::vector<GLuint> mesh_sizes;
		std::vector<GLuint> materials;
		size_t num_materials;
		bool textured;
		bool normal_mapped;
		bool hasBones;
	};

	//Dictionary of meshes
	std::unordered_map<std::string, Mesh> meshes;

	struct BoneAnimation
	{
		const aiScene *scene;
		GLuint shader;
	};
	std::unordered_map<std::string, BoneAnimation> animations;

	std::unordered_map<std::string, BoneAnimation> GetAnimations() { return animations; }

	struct VertexBoneData
	{
		unsigned int IDs[4]; //!< An array of 4 bone Ids that influence a single vertex.
		float Weights[4]; //!< An array of the weight influence per bone. 

		VertexBoneData()
		{
			// 0's out the arrays. 
			Reset();
		}

		void Reset()
		{
			memset(IDs, 0, 4 * sizeof(IDs[0]));
			memset(Weights, 0, 4 * sizeof(Weights[0]));
		}

		void AddBoneData(unsigned int BoneID, float Weight)
		{
			for (unsigned int i = 0; i < 4; i++) {

				// Check to see if there are any empty weight values. 
				if (Weights[i] == 0.0) {
					// Add ID of bone. 
					IDs[i] = BoneID;

					// Set the vertex weight influence for this bone ID. 
					Weights[i] = Weight;
					return;
				}

			}
			// should never get here - more bones than we have space for
			assert(0);
		}
	};

	// Stores bone information
	struct BoneInfo
	{
		glm::mat4 FinalTransformation; // Final transformation to apply to vertices 
		glm::mat4 BoneOffset; // Initial offset from local to bone space. 

		BoneInfo()
		{
			/*BoneOffset = glm::mat4();
			FinalTransformation = glm::mat4();*/
		}
	};

	glm::mat4 Mat3AssimpToGLM(aiMatrix3x3 mat)
	{
		glm::mat4 m;
		m[0][0] = mat.a1; m[0][1] = mat.a2; m[0][2] = mat.a3; m[0][3] = 0.0f;
		m[1][0] = mat.b1; m[1][1] = mat.b2; m[1][2] = mat.b3; m[1][3] = 0.0f;
		m[2][0] = mat.c1; m[2][1] = mat.c2; m[2][2] = mat.c3; m[2][3] = 0.0f;
		m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
		return m;
	}

	glm::mat4 Mat4AssimpToGLM(aiMatrix4x4 mat)
	{
		glm::mat4 m;
		m[0][0] = mat.a1; m[0][1] = mat.a2; m[0][2] = mat.a3; m[0][3] = mat.a4;
		m[1][0] = mat.b1; m[1][1] = mat.b2; m[1][2] = mat.b3; m[1][3] = mat.b4;
		m[2][0] = mat.c1; m[2][1] = mat.c2; m[2][2] = mat.c3; m[2][3] = mat.c4;
		m[3][0] = mat.d1; m[3][1] = mat.d2; m[3][2] = mat.d3; m[3][3] = mat.d4;
		return m;
	}

	unsigned int m_NumBones = 0; //!< Total number of bones in the model. 
	std::map<std::string, unsigned int> m_BoneMapping; //!< Map of bone names to ids
	std::vector<BoneInfo> m_BoneInfo; //!< Array containing bone information such as offset matrix and final transformation. 
	glm::mat4 GlobalTransformation; //!< Root node transformation. 
	glm::mat4 m_GlobalInverseTransform;
	std::vector<VertexBoneData> bones;

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

	void load(std::string file)
	{
		//Ensure model hasn't already been loaded
		if (meshes.find(file) != meshes.end()) return;

		//Ensure VBO has been generated
		if (!vbo.created())
			vbo.create();

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
		if (!scene) return;

		m_GlobalInverseTransform = Mat4AssimpToGLM(scene->mRootNode->mTransformation);
		m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);

		//Create new model
		Mesh &m = meshes.emplace(file, Mesh()).first->second;

		//Load all model data from aiScene
		const size_t vertexTotalSize = sizeof(aiVector3D) * 3 + sizeof(aiVector2D);
		size_t totalVertices = 0;
		size_t currentVertices = 0;
		size_t prevTotal;

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[i];
			totalVertices += mesh->mNumVertices;
		}

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh *mesh = scene->mMeshes[i];
			size_t meshFaces = mesh->mNumFaces;
			m.materials.push_back(mesh->mMaterialIndex);
			size_t size0 = vbo.size();
			m.mesh_starts.push_back((GLuint)size0 / vertexTotalSize);

			for (size_t j = 0; j < meshFaces; j++)
			{
				const aiFace &face = mesh->mFaces[j];
				for (int k = 0; k < 3; k++)
				{
					aiVector3D pos = (mesh->HasPositions()) ?
						mesh->mVertices[face.mIndices[k]] :
						aiVector3D(1.0f, 1.0f, 1.0f);
					//WARNING - SHOULD THIS BE aiVector2D ???
					aiVector3D uv = (mesh->GetNumUVChannels() > 0) ?
						mesh->mTextureCoords[0][face.mIndices[k]] :
						aiVector3D(1.0f, 1.0f, 1.0f);
					aiVector3D normal = (mesh->HasNormals()) ?
						mesh->mNormals[face.mIndices[k]] :
						aiVector3D(1.0f, 1.0f, 1.0f);
					aiVector3D tangent = (mesh->HasTangentsAndBitangents()) ?
						mesh->mTangents[face.mIndices[k]] :
						aiVector3D(1.0f, 1.0f, 1.0f);

					vbo.add_data(&pos, sizeof(aiVector3D));
					vbo.add_data(&uv, sizeof(aiVector2D));
					vbo.add_data(&normal, sizeof(aiVector3D));
					vbo.add_data(&tangent, sizeof(aiVector3D));
				}
			}

			prevTotal = currentVertices;
			currentVertices += mesh->mNumVertices;
			m.mesh_sizes.push_back((GLuint)(vbo.size() - size0) / vertexTotalSize);

			if (mesh->HasBones())
			{
				m.hasBones = true;

				BoneAnimation &b = animations.emplace(file, BoneAnimation()).first->second;

				// Copy all the data from the scene 
				b.scene = scene;
				// End of horrible copy code

				bones.resize(totalVertices);

				// Loop through all bones in the Assimp mesh.
				for (unsigned int i = 0; i < mesh->mNumBones; i++) {

					unsigned int BoneIndex = 0;

					// Obtain the bone name.
					std::string BoneName(mesh->mBones[i]->mName.data);

					// If bone isn't already in the map. 
					if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {

						// Set the bone ID to be the current total number of bones. 
						BoneIndex = m_NumBones;

						// Increment total bones. 
						m_NumBones++;

						// Push new bone info into bones vector. 
						BoneInfo bi;
						m_BoneInfo.push_back(bi);
					}
					else {
						// Bone ID is already in map. 
						BoneIndex = m_BoneMapping[BoneName];
					}

					m_BoneMapping[BoneName] = BoneIndex;

					// Obtains the offset matrix which transforms the bone from mesh space into bone space. 
					m_BoneInfo[BoneIndex].BoneOffset = Mat4AssimpToGLM(mesh->mBones[i]->mOffsetMatrix);


					// Iterate over all the affected vertices by this bone i.e weights. 
					for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {

						// Obtain an index to the affected vertex within the array of vertices.
						unsigned int VertexID = prevTotal + mesh->mBones[i]->mWeights[j].mVertexId;
						// The value of how much this bone influences the vertex. 
						float Weight = mesh->mBones[i]->mWeights[j].mWeight;

						// Insert bone data for particular vertex ID. A maximum of 4 bones can influence the same vertex. 
						bones[VertexID].AddBoneData(BoneIndex, Weight);
					}
				}
			}

		}

		m.num_materials = scene->mNumMaterials;
		std::vector<size_t> materialRemap(m.num_materials);

		for (size_t i = 0; i < m.num_materials; i++)
		{
			const aiMaterial *material = scene->mMaterials[i];
			int texIndex = 0;
			aiString path;

			if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &path) == AI_SUCCESS)
			{
				m.textured = true;
				std::string fullPath = strip_last_path(file) + path.data;

				int texFound = -1;
				for (int j = 0; j < (int)textures.size(); j++)
					if (fullPath == textures[j].path)
					{
						texFound = j;
						break;
					}
				if (texFound != -1)
					materialRemap[i] = texFound;
				else
				{
					Texture t(fullPath, true);
					materialRemap[i] = textures.size();
					textures.push_back(t);
				}
			}
		}


		for (size_t i = 0; i < m.num_materials; i++)
		{
			const aiMaterial *material = scene->mMaterials[i];
			int texIndex = 0;
			aiString path;

			if (material->GetTexture(aiTextureType_HEIGHT, texIndex, &path) == AI_SUCCESS) // Note assimp treats the way .obj stores normalmaps as heightmaps
			{
				m.normal_mapped = true;
				std::string fullPath = strip_last_path(file) + path.data;

				int normalFound = -1;
				for (int j = 0; j < (int)normalMaps.size(); j++)
					if (fullPath == normalMaps[j].path)
					{
						normalFound = j;
						break;
					}
				if (normalFound != -1)
					materialRemap[i] = normalFound;
				else
				{
					Texture t(fullPath, true);
					materialRemap[i] = normalMaps.size();
					normalMaps.push_back(t);
				}
			}
		}

		for (int i = 0; i < (int)m.mesh_sizes.size(); i++)
		{
			int o = m.materials[i];
			m.materials[i] = (GLuint)materialRemap[o];
		}
	}

	void finalise()
	{
		//Generate and bind VAO
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		//Bind and upload VBO
		vbo.bind();
		vbo.upload(GL_STATIC_DRAW);

		//Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)0);
		//Texture coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)sizeof(aiVector3D));
		//Normal vectors
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)(sizeof(aiVector3D) + sizeof(aiVector2D)));
		//Tangent vectors
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)(2 * sizeof(aiVector3D) + sizeof(aiVector2D)));


		// This is probably wrong
		//Generate and bind VAO
		glGenVertexArrays(1, &bonebo);
		glBindVertexArray(bonebo);

		//Bones
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(0, 4, GL_INT, GL_FALSE, sizeof(VertexBoneData), (void*)0);
		//Bone weights
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)16);

		bones.clear();
	}

	//Calculates the projection matrix for a camera
	glm::mat4 proj_matrix(CameraComponent camera)
	{
		return glm::infinitePerspective(
			R(camera.fov),
			(float)ASPECT_RATIO_VAL,
			0.1f
		);
	}

	//Calculates the view matrix for a camera
	glm::mat4 view_matrix(CameraComponent camera)
	{
		//Calculate direction of view from angles
		glm::vec3 dir = camera.orientation.direction_hv();

		//Get a vector perpendicularly upwards
		glm::vec3 right = camera.orientation.direction_hv_right();
		glm::vec3 up = glm::cross(right, dir);

		return glm::lookAt(
			glm::vec3(camera.position),
			glm::vec3(camera.position) + dir,
			up
		);
	}

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

	unsigned int FindTranslation(float AnimationTime, const aiNodeAnim* pNodeAnim)
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

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

	void CalcInterpolatedTranslation(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
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

	glm::mat4 InitTranslationTransform(float x, float y, float z)
	{
		glm::mat4 m = glm::mat4(1);
		m[0][3] = x;
		m[1][3] = y;
		m[2][3] = z;
		return m;
	}

	void ReadNodeHierarchy(float AnimationTime, const aiScene* scene, const aiNode* pNode, const glm::mat4& ParentTransform)
	{
		glm::mat4 IdentityTest = glm::mat4(1);

		// Obtain the name of the current node 
		std::string NodeName(pNode->mName.data);

		// Use the first animation 
		const aiAnimation* pAnimation = scene->mAnimations[0];

		// Obtain transformation relative to node's parent. 
		glm::mat4 NodeTransformation = Mat4AssimpToGLM(pNode->mTransformation);

		const aiNodeAnim* pNodeAnim = NULL;

		// Find the animation channel of the current node. 
		for (unsigned i = 0; i < pAnimation->mNumChannels; i++) {
			const aiNodeAnim* pNodeAnimIndex = pAnimation->mChannels[i];

			// If there is a match for a channel with the current node's name, then we've found the animation channel. 
			if (std::string(pNodeAnimIndex->mNodeName.data) == NodeName) {
				pNodeAnim = pNodeAnimIndex;
			}
		}

		if (pNodeAnim) {

			//// Interpolate scaling and generate scaling transformation matrix
			//aiVector3D Scaling;
			//CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			//Matrix4f ScalingM;
			//ScalingM.InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);

			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion RotationQ;
			CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
			glm::mat4 RotationM = Mat3AssimpToGLM(RotationQ.GetMatrix());

			// Interpolate translation and generate translation transformation matrix
			aiVector3D Translation;
			CalcInterpolatedTranslation(Translation, AnimationTime, pNodeAnim);


			glm::mat4 TranslationM = InitTranslationTransform(Translation.x, Translation.y, Translation.z);

			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM;/* *ScalingM;*/
		}

		glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

		// Apply the final transformation to the indexed bone in the array. 
		if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
			unsigned int BoneIndex = m_BoneMapping[NodeName];
			m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation *
				m_BoneInfo[BoneIndex].BoneOffset;
		}

		// Do the same for all the node's children. 
		for (unsigned i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHierarchy(AnimationTime, scene, pNode->mChildren[i], GlobalTransformation);
		}
	}

	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms, GLuint& shader, std::string file)
	{
		glm::mat4 Identity = glm::mat4(1);

		auto it = renderer::animations.find(file);
		if (it == renderer::animations.end()) return;
		const renderer::BoneAnimation &animation = it->second;

		float TicksPerSecond = animation.scene->mAnimations[0]->mTicksPerSecond;
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, animation.scene->mAnimations[0]->mDuration);

		ReadNodeHierarchy(AnimationTime, animation.scene, animation.scene->mRootNode, Identity);

		Transforms.resize(m_NumBones);

		// Populates transforms vector with new bone transformation matrices. 
		for (unsigned int i = 0; i < m_NumBones; i++) {
			Transforms[i] = m_BoneInfo[i].FinalTransformation;
		}

		shader = animation.shader;
	}

	void init()
	{
		//Configure OpenGL
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
	}

	void render_model(CameraComponent camera, ModelComponent model, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients, size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points)
	{
		//Bind correct VAO
		glBindVertexArray(vao);

		//Get the model, aborting if not found
		auto it = meshes.find(model.model_file);
		if (it == meshes.end()) return;
		const Mesh &mesh = it->second;

		model.hasBones = mesh.hasBones;

		//Determine and use appropriate shader
		GLuint shader = get_shader(mesh.textured, mesh.normal_mapped, n_ambient, n_directional, n_point,
			model.vertex_shader, model.fragment_shader);
		glUseProgram(shader);

		// if boned
		auto animationIt = animations.find(model.model_file);
		if (animationIt == animations.end()) return;
		BoneAnimation &animation = animationIt->second;
		animation.shader = shader;

		//Calculate MVP matrices
		glm::mat4 p = proj_matrix(camera);
		glm::mat4 v = view_matrix(camera);

		glm::mat4 m = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		//Provide MVP matrices
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "projectionMatrix"),
			1, GL_FALSE, glm::value_ptr(p)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "viewMatrix"),
			1, GL_FALSE, glm::value_ptr(v)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "modelMatrix"),
			1, GL_FALSE, glm::value_ptr(m)
		);

		//Provide flat colour component
		glUniform4f(
			glGetUniformLocation(shader, "flatColour"),
			(GLfloat)c.colour.x, (GLfloat)c.colour.y, (GLfloat)c.colour.z,
			(GLfloat)c.alpha
		);

		//Provide shininess value (used to determine how much specular highlighting the model will have)
		glUniform1f(
			glGetUniformLocation(shader, "shininess"),
			(GLfloat)model.shininess
		);

		// Provide camera position for eye calculations
		glUniform3f(
			glGetUniformLocation(shader, "cameraPosition"),
			(GLfloat)camera.position.x, (GLfloat)camera.position.y, (GLfloat)camera.position.z
		);

		//Provide ambient lights information
		for (size_t i = 0; i < n_ambient; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("ambientLights[" + j + "].colour").c_str()),
				(GLfloat)ambients[i].colour.x,
				(GLfloat)ambients[i].colour.y,
				(GLfloat)ambients[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("ambientLights[" + j + "].intensity").c_str()),
				(GLfloat)ambients[i].intensity);
		}

		//Provide directional lights information
		for (size_t i = 0; i < n_directional; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].colour").c_str()),
				(GLfloat)directionals[i].colour.x,
				(GLfloat)directionals[i].colour.y,
				(GLfloat)directionals[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].intensity").c_str()),
				(GLfloat)directionals[i].intensity);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].direction").c_str()),
				(GLfloat)directionals[i].direction.x,
				(GLfloat)directionals[i].direction.y,
				(GLfloat)directionals[i].direction.z);
		}

		//Provide point lights information
		for (size_t i = 0; i < n_point; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("pointLights[" + j + "].colour").c_str()),
				(GLfloat)points[i].colour.x,
				(GLfloat)points[i].colour.y,
				(GLfloat)points[i].colour.z);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].intensity").c_str()),
				(GLfloat)points[i].intensity);
			glUniform3f(glGetUniformLocation(shader,
				("pointLights[" + j + "].position").c_str()),
				(GLfloat)points[i].position.x,
				(GLfloat)points[i].position.y,
				(GLfloat)points[i].position.z);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].constant").c_str()),
				(GLfloat)points[i].constant);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].linear").c_str()),
				(GLfloat)points[i].linear);
			glUniform1f(glGetUniformLocation(shader,
				("pointLights[" + j + "].exponent").c_str()),
				(GLfloat)points[i].exponent);
		}

		//Draw the model
		for (size_t i = 0; i < mesh.mesh_sizes.size(); i++)
		{
			int offset = 0;

			//Bind texture if the model has them
			if (mesh.textured)
			{
				Texture &t = textures[mesh.materials[i]];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, t.handle);
				glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);

				offset++;
			}

			// Bind model normal maps
			if (mesh.normal_mapped)
			{
				Texture &n = normalMaps[mesh.materials[i]];

				glActiveTexture(GL_TEXTURE0 + offset);
				glBindTexture(GL_TEXTURE_2D, n.handle);
				glUniform1i(glGetUniformLocation(shader, "normalSampler"), offset);

				offset++;
			}

			//Draw the triangles
			glDrawArrays(GL_TRIANGLES, mesh.mesh_starts[i], mesh.mesh_sizes[i]);
		}
	}
}
