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
#include "VBO.h"
#include "Texture.h"

//Quick conversion to radians
#define R(x) glm::radians((float)x)

namespace game::renderer
{
	//Returns the (potentially cached) shader using the given paramaters
	GLuint get_shader(bool textured, size_t n_ambient, size_t n_directional)
	{
		//Cache of parametrised shaders
		static std::map<std::tuple<bool, size_t>, Shader> shaders;

		auto args = std::make_tuple(textured, n_ambient);

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
			define(f, "N_AMBIENT " + std::to_string(n_ambient));
			define(f, "N_DIRECTIONAL " + std::to_string(n_directional));

			//Create new shader
			auto &s = shaders[args];
			s.load("", "shaders/Passthrough.vert", "shaders/ParametrisedFragment.frag", v, f);
			//s.load("", "shaders/BlinnPhong.vert", "shaders/BlinnPhong.frag", v, f);
			return s.handle();
		}
	}

	//Global vertex array object
	GLuint vao;

	//Global vertex buffer object
	VBO vbo;

	//Global textures collection
	std::vector<Texture> textures;

	//Represents data of a mesh
	struct Mesh
	{
		std::vector<GLuint> mesh_starts;
		std::vector<GLuint> mesh_sizes;
		std::vector<GLuint> materials;
		size_t num_materials;
		bool textured;
	};

	//Dictionary of meshes
	std::unordered_map<std::string, Mesh> meshes;

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

		//Create new model
		Mesh &m = meshes.emplace(file, Mesh()).first->second;

		//Load all model data from aiScene
		const size_t vertexTotalSize = sizeof(aiVector3D) * 2 + sizeof(aiVector2D);
		size_t totalVertices = 0;

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

					vbo.add_data(&pos, sizeof(aiVector3D));
					vbo.add_data(&uv, sizeof(aiVector2D));
					vbo.add_data(&normal, sizeof(aiVector3D));
				}
			}

			totalVertices += mesh->mNumVertices;
			m.mesh_sizes.push_back((GLuint)(vbo.size() - size0) / vertexTotalSize);
		}

		m.num_materials = scene->mNumMaterials;
		std::vector<size_t> materialRemap(m.num_materials);

		for (size_t i = 0; i < m.num_materials; i++)
		{
			const aiMaterial *material = scene->mMaterials[i];
			int a = 5;
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)0);
		//Texture coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)sizeof(aiVector3D));
		//Normal vectors
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(aiVector3D) + sizeof(aiVector2D), (void*)(sizeof(aiVector3D) + sizeof(aiVector2D)));
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
		glDepthFunc(GL_LESS);
	}

	void render_model(CameraComponent camera, ModelComponent model, ColourComponent c, TransformComponent t,
		size_t n_ambient, AmbientLightComponent *ambients, size_t n_directional, DirectionalLightComponent *directionals)
	{
		//Bind correct VAO
		glBindVertexArray(vao);

		//Get the model, aborting if not found
		auto it = meshes.find(model.model_file);
		if (it == meshes.end()) return;
		const Mesh &mesh = it->second;

		//Determine and use appropriate shader
		GLuint shader = get_shader(mesh.textured, n_ambient, n_directional);
		glUseProgram(shader);

		//Calculate MVP matrices
		glm::mat4 p = proj_matrix(camera);
		glm::mat4 v = view_matrix(camera);

		glm::mat4 m = glm::translate(glm::vec3(t.position)) *
			glm::rotate(R(t.rotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(R(t.rotation.z), glm::vec3(0, 0, 1)) *
			glm::rotate(R(t.rotation.y), glm::vec3(0, 1, 0)) *
			glm::scale(glm::vec3(t.scale));

		glm::mat4 modelview = v * m;

		//Provide MVP matrices
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "projectionMatrix"),
			1, GL_FALSE, glm::value_ptr(p)
		);
		glUniformMatrix4fv(
			glGetUniformLocation(shader, "modelviewMatrix"),
			1, GL_FALSE, glm::value_ptr(modelview)
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

		//Provide ambient lights information
		for (size_t i = 0; i < n_directional; i++)
		{
			std::string j = std::to_string(i);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].colour").c_str()),
				(GLfloat)directionals[i].colour.x,
				(GLfloat)directionals[i].colour.y,
				(GLfloat)directionals[i].colour.z);
			glUniform3f(glGetUniformLocation(shader,
				("directionalLights[" + j + "].position").c_str()),
				(GLfloat)directionals[i].position.x,
				(GLfloat)directionals[i].position.y,
				(GLfloat)directionals[i].position.z);
		}

		//Draw the model
		for (size_t i = 0; i < mesh.mesh_sizes.size(); i++)
		{
			//Bind texture if the model has them
			if (mesh.textured)
			{
				Texture &t = textures[mesh.materials[i]];
				glBindTexture(GL_TEXTURE_2D, t.handle);
				glBindSampler(0, t.sampler);
			}
			//Draw the triangles
			glDrawArrays(GL_TRIANGLES, mesh.mesh_starts[i], mesh.mesh_sizes[i]);
		}
	}
}
