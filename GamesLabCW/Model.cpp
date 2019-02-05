#include "Model.h"

#include "Utility.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx\transform.hpp"

Model::Model(GLuint shaderProgram)
{
	SetPosition(glm::vec3(0, 0, 0));
	SetRotation(glm::vec3(0, 0, 0));
	SetScale(glm::vec3(1, 1, 1));

	modelMatrixLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
	assert(modelMatrixLocation != 0xFFFFFFFF);
}

Model::~Model()
{
}

void Model::CreateVertexBuffer()
{
	glm::vec3 vertices[4];
	vertices[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
	vertices[1] = glm::vec3(0.0f, -1.0f, 1.0f);
	vertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);
	vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void Model::CreateIndexBuffer()
{
	unsigned int indices[] = { 0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2 };

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Model::Render()
{
	SetScale(scale * 1.001f);

	modelMatrix = glm::translate(position) *
		glm::rotate(Utility::ToRadians(rotation.x), glm::vec3(1, 0, 0)) *
		glm::rotate(Utility::ToRadians(rotation.z), glm::vec3(0, 0, 1)) *
		glm::rotate(Utility::ToRadians(rotation.y), glm::vec3(0, 1, 0)) *
		glm::scale(scale);

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_TRUE, &modelMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
}