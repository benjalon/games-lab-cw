#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp> 
#include "glm/gtx/transform.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <GL/glew.h>

#include "MeshPart.h"

/**
*	Load mesh from file
**/
Mesh::Mesh(const char *filename)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filename, NULL);
	assert(scene && importer.GetErrorString());

	for (int i = 0; i < scene->mNumMeshes; ++i) {
		meshEntries.push_back(new MeshPart(scene->mMeshes[i]));
	}
}

/**
*	Clear all loaded parts
**/
Mesh::~Mesh(void)
{
	for (int i = 0; i < meshEntries.size(); ++i) {
		delete meshEntries.at(i);
	}
	meshEntries.clear();
}

/**
*	Renders all loaded parts
**/
void Mesh::Render(GLuint shaderID, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
	// Calculate matrices
	modelMatrix = 
		glm::translate(position) *
		glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(rotation.y, glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::rotate(rotation.z, glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::scale(scale);
	mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVPMatrix"), 1, GL_TRUE, glm::value_ptr(mvpMatrix));

	// Render
	for (int i = 0; i < meshEntries.size(); ++i) {
		meshEntries.at(i)->Render();
	}
}