#include "Model.h"

#include "Utility.h"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx\transform.hpp"

Model::Model(GLuint shaderProgram)
{
	SetPosition(glm::vec3(0, 0, 0));
	SetRotation(glm::vec3(0, 0, 0));
	SetScale(glm::vec3(1, 1, 1));

	mvpMatrixLocation = glGetUniformLocation(shaderProgram, "mvpMatrix");
	assert(mvpMatrixLocation != 0xFFFFFFFF);
}

Model::~Model()
{
}

void Model::CreateVertexBuffer()
{
	glm::vec3 vertices[4];
	vertices[0] = glm::vec3(-1.0f, -1.0f, 0.5773f);
	vertices[1] = glm::vec3(0.0f, -1.0f, -1.15475f);
	vertices[2] = glm::vec3(1.0f, -1.0f, 0.5773f);
	vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void Model::CreateIndexBuffer()
{
	unsigned int indices[] = 
	{ 
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2 
	};

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void Model::Render(Camera* camera)
{
	//SetScale(scale * 1.001f);

	modelMatrix = glm::translate(position) *
		glm::rotate(glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::radians(rotation.z), glm::vec3(0, 0, 1)) *
		glm::rotate(glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
		glm::scale(scale);

	glm::mat4 mvpMatrix = modelMatrix * camera->GetViewMatrix() * camera->GetProjectionMatrix();
	glUniformMatrix4fv(mvpMatrixLocation, 1, GL_TRUE, &mvpMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
}