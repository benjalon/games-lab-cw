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
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "BO.h"
#include "Mesh.h"

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
	BO vbo;
	BO ibo(GL_ELEMENT_ARRAY_BUFFER);

	//Global textures collection
	std::vector<Texture> textures;
	std::vector<Texture> normalMaps;
	std::map<std::string, Texture> external_textures;

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

	struct Model
	{
		std::vector<Mesh> meshes;
		bool textured;
		bool normal_mapped;
	};
	//Dictionary of models
	std::unordered_map<std::string, Model> models;

	void load_model(std::string file)
	{
		//Ensure model hasn't already been loaded
		if (models.find(file) != models.end()) return;

		//Ensure vertex buffer has been generated
		if (!vbo.created())
			vbo.create();
		//Ensure index buffer has been generated
		if (!ibo.created())
			ibo.create();

		//Load model from file
		Assimp::Importer imp;
		const aiScene *scene = imp.ReadFile(file,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
		);

		//Abort if unsuccessful
		std::cout << (scene ? "Loaded model " : "Could not load model ") << file << std::endl;
		if (!scene) return;

		//Load all model data from aiScene
		const size_t vertexTotalSize = sizeof(aiVector3D) * 3 + sizeof(aiVector2D);
		size_t totalVertices = 0;

		Model &model = models.emplace(file, Model()).first->second;

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* meshData = scene->mMeshes[i];
			Mesh loadedMesh = Mesh(vao, vbo, ibo, meshData, scene->mMaterials[meshData->mMaterialIndex]);
			model.meshes.push_back(loadedMesh);
		}

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

	void init()
	{
		//Configure OpenGL
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
	}

	void render_model(CameraComponent camera, ModelComponent m, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients, size_t n_directional, DirectionalLightComponent *directionals,
		size_t n_point, PointLightComponent *points)
	{
		//Get the model, aborting if not found
		auto it = models.find(m.model_file);
		if (it == models.end()) return;
		const Model &model = it->second;

		glm::mat4 vm;

		//auto tx_it = external_textures.find(m.model_file);
		//if (tx_it != external_textures.end() && tx_it->second.isSkybox)
		//{
		//	// Skyboxes must be rendered behind everything else so disregard camera transform 
		//	// and change depth setting
		//    glDepthFunc(GL_LEQUAL);
		//	vm = glm::mat3(view_matrix(camera));
		//}
		//else
		//{
		//	// Regular render settings
		//	glDepthFunc(GL_LESS);
			vm = view_matrix(camera);
		//}
		//
		//Determine and use appropriate shader
		GLuint shader = get_shader(model.textured, model.normal_mapped, n_ambient, n_directional, n_point,
			m.vertex_shader, m.fragment_shader);
		glUseProgram(shader);

		//Calculate MVP matrices
		glm::mat4 pm = proj_matrix(camera);

		glm::mat4 mm = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		//Provide MVP matrices
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "projectionMatrix"),
			1, GL_FALSE, glm::value_ptr(pm)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "viewMatrix"),
			1, GL_FALSE, glm::value_ptr(vm)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "modelMatrix"),
			1, GL_FALSE, glm::value_ptr(mm)
		);

		//if (tx_it != external_textures.end())
		//{
		//	const Texture &texture = tx_it->second;

		//	glActiveTexture(GL_TEXTURE0);
		//	switch (texture.type)
		//	{
		//	case TextureType::DIFFUSE:
		//		glBindTexture(GL_TEXTURE_2D, texture.handle);
		//		glUniform1i(glGetUniformLocation(shader, "texSampler"), 0);
		//		break;
		//	case TextureType::NORMAL:
		//		glBindTexture(GL_TEXTURE_2D, texture.handle);
		//		glUniform1i(glGetUniformLocation(shader, "normalSampler"), 0);
		//		break;
		//	case TextureType::SPECULAR:
		//		// Not yet implemented
		//		break;
		//	case TextureType::CUBE:
		//		glBindTexture(GL_TEXTURE_CUBE_MAP, texture.handle);
		//		glUniform1i(glGetUniformLocation(shader, "cubeSampler"), 0);
		//		break;

		//	default:
		//		break;
		//	}
		//	//Provide cubemap component (reflections, skyboxes etc.)
		//}

		////Provide flat colour component
		//glUniform4f(
		//	glGetUniformLocation(shader, "flatColour"),
		//	(GLfloat)c.colour.x, (GLfloat)c.colour.y, (GLfloat)c.colour.z,
		//	(GLfloat)c.alpha
		//);

		////Provide shininess value (used to determine how much specular highlighting the model will have)
		//glUniform1f(
		//	glGetUniformLocation(shader, "shininess"),
		//	(GLfloat)m.shininess
		//);

		//// Provide camera position for eye calculations
		//glUniform3f(
		//	glGetUniformLocation(shader, "cameraPosition"),
		//	(GLfloat)camera.position.x, (GLfloat)camera.position.y, (GLfloat)camera.position.z
		//);

		////Provide ambient lights information
		//for (size_t i = 0; i < n_ambient; i++)
		//{
		//	std::string j = std::to_string(i);
		//	glUniform3f(glGetUniformLocation(shader,
		//		("ambientLights[" + j + "].colour").c_str()),
		//		(GLfloat)ambients[i].colour.x,
		//		(GLfloat)ambients[i].colour.y,
		//		(GLfloat)ambients[i].colour.z);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("ambientLights[" + j + "].intensity").c_str()),
		//		(GLfloat)ambients[i].intensity);
		//}

		////Provide directional lights information
		//for (size_t i = 0; i < n_directional; i++)
		//{
		//	std::string j = std::to_string(i);
		//	glUniform3f(glGetUniformLocation(shader,
		//		("directionalLights[" + j + "].colour").c_str()),
		//		(GLfloat)directionals[i].colour.x,
		//		(GLfloat)directionals[i].colour.y,
		//		(GLfloat)directionals[i].colour.z);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("directionalLights[" + j + "].intensity").c_str()),
		//		(GLfloat)directionals[i].intensity);
		//	glUniform3f(glGetUniformLocation(shader,
		//		("directionalLights[" + j + "].direction").c_str()),
		//		(GLfloat)directionals[i].direction.x,
		//		(GLfloat)directionals[i].direction.y,
		//		(GLfloat)directionals[i].direction.z);
		//}

		////Provide point lights information
		//for (size_t i = 0; i < n_point; i++)
		//{
		//	std::string j = std::to_string(i);
		//	glUniform3f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].colour").c_str()),
		//		(GLfloat)points[i].colour.x,
		//		(GLfloat)points[i].colour.y,
		//		(GLfloat)points[i].colour.z);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].intensity").c_str()),
		//		(GLfloat)points[i].intensity);
		//	glUniform3f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].position").c_str()),
		//		(GLfloat)points[i].position.x,
		//		(GLfloat)points[i].position.y,
		//		(GLfloat)points[i].position.z);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].constant").c_str()),
		//		(GLfloat)points[i].constant);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].linear").c_str()),
		//		(GLfloat)points[i].linear);
		//	glUniform1f(glGetUniformLocation(shader,
		//		("pointLights[" + j + "].exponent").c_str()),
		//		(GLfloat)points[i].exponent);
		//}

		// Bind the indices from the array object
		glBindVertexArray(vao);

		ibo.bind(); // This should be unnecessary but for whatever reason the state isn't being kept

		// Draw each mesh part of this current model
		for (size_t i = 0; i < model.meshes.size(); i++)
		{
			glDrawElements(GL_TRIANGLES, model.meshes[i].indices.size(), GL_UNSIGNED_INT, 0);
		}

		// Unbind
		glBindVertexArray(0);
	}
}
