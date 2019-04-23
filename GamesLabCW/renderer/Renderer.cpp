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
#include "Model.h"

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

	
	//Global textures collection
	std::vector<Texture> textures;
	std::vector<Texture> normalMaps;
	std::map<std::string, Texture> external_textures;

	//Dictionary of meshes
	std::unordered_map<std::string, Mesh> meshes;

	struct BoneAnimation
	{
		const aiScene *scene;
		GLuint shader;
	};
	std::unordered_map<std::string, BoneAnimation> animations;

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
			BoneOffset = glm::mat4();
			FinalTransformation = glm::mat4();
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
	
	void load_external_texture(std::string path, std::string model_path, TextureType type)
	{
		auto texture = Texture(path);
		texture.type = type;
		external_textures.emplace(model_path, texture); // Need to map this texture with a model, since it was loaded externally
	}

	void load_external_cubemap(std::string paths[6], std::string model_path, TextureType type, bool skybox)
	{
		auto cubemap = Texture(paths, skybox);
		cubemap.type = type;
		cubemap.isSkybox = skybox;
		external_textures.emplace(model_path, cubemap); // Need to map this texture with a model, since it was loaded externally
	}

	void load_model(std::string file) {
		Model model = Model();
		meshes.emplace(file, model.load_model(file)).first->second;
	}

	void finalise() {
		// TODO DELETE THIS
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

	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms, std::string file)
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

			assert(i < 100);
		}


		//TODO:
		/*for (unsigned int i = 0; i < vertices.size(); ++i)
		{
			auto currentBone = bones[i];

			glm::mat4 boneTransform = Transforms[currentBone.IDs[0]] * currentBone.Weights[0];
			boneTransform += Transforms[currentBone.IDs[1]] * currentBone.Weights[1];
			boneTransform += Transforms[currentBone.IDs[2]] * currentBone.Weights[2];
			boneTransform += Transforms[currentBone.IDs[3]] * currentBone.Weights[3];
			boneTransform = glm::transpose(boneTransform);

			glm::vec4 inVertex = glm::vec4(vertices[i].position, 1);
			vertices[i].position = glm::vec3(boneTransform * inVertex);
		}

		renderer::UpdateVertices(vertices);*/
	}

	void init()
	{
		//Configure OpenGL
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		//glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glEnable(GL_DEPTH_TEST);
	}

	void render_model(CameraComponent camera, ModelComponent &model, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients, size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points)
	{

		//Get the model, aborting if not found
		auto it = meshes.find(model.model_file);
		if (it == meshes.end()) return;
		const Mesh &mesh = it->second;

		//Bind correct VAO
		glBindVertexArray(mesh.vao);

		glm::mat4 v;

		auto tx_it = external_textures.find(model.model_file);
		if (tx_it != external_textures.end() && tx_it->second.isSkybox)
		{
			// Skyboxes must be rendered behind everything else so disregard camera transform 
			// and change depth setting
		    glDepthFunc(GL_LEQUAL);
			v = glm::mat3(view_matrix(camera));
		}
		else
		{
			// Regular render settings
			glDepthFunc(GL_LESS);
			v = view_matrix(camera);
		}
		
		//Determine and use appropriate shader
		GLuint shader = get_shader(mesh.textured, mesh.normal_mapped,n_ambient, n_directional, n_point, model.vertex_shader, model.fragment_shader);
		glUseProgram(shader);

		if (mesh.hasBones)
		{
			model.hasBones = true;
			auto animationIt = animations.find(model.model_file);
			if (animationIt == animations.end()) return;
			BoneAnimation &animation = animationIt->second;
			animation.shader = shader;

			// TODO: Delete this
		}

		//Calculate MVP matrices
		glm::mat4 p = proj_matrix(camera);

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

		if (tx_it != external_textures.end())
		{
			const Texture &texture = tx_it->second;

			glActiveTexture(GL_TEXTURE0);
			switch (texture.type)
			{
			case TextureType::DIFFUSE:
				glBindTexture(GL_TEXTURE_2D, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);
				break;
			case TextureType::NORMAL:
				glBindTexture(GL_TEXTURE_2D, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "normalSampler"), 0);
				break;
			case TextureType::SPECULAR:
				// Not yet implemented
				break;
			case TextureType::CUBE:
				glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);
				glUniform1i(glGetUniformLocation(shader, "cubeSampler"), 0);
				break;

			default:
				break;
			}
			//Provide cubemap component (reflections, skyboxes etc.)
		}

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

		//Draw the triangles
		glBindVertexArray(mesh.vao);

		//Draw the model
		for (size_t i = 0; i < mesh.vertex_count.size(); i++)
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

			glDrawElementsBaseVertex(GL_TRIANGLES,
				mesh.index_count[i],
				GL_UNSIGNED_INT,
				(void*)(sizeof(unsigned int) * mesh.base_index[i]),
				mesh.base_vertex[i]);

			//glDrawArrays(GL_TRIANGLES, mesh.base_vertex[i], mesh.vertex_count[i]);
		}

		glBindVertexArray(0);
	}
}
