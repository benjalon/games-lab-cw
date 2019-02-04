#include "Model.h"

#include "Utility.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx\transform.hpp"

Model::Model()
{
	SetPosition(glm::vec3(0, 0, 0));
	SetRotation(glm::vec3(0, 0, 0));
	SetScale(glm::vec3(1, 1, 1));
}

Model::~Model()
{
}

void Model::Render(GLuint modelMatrixLocation)
{
	SetScale(scale * 1.001f);

	modelMatrix = glm::translate(position) *
		glm::rotate(Utility::ToRadians(rotation.x), glm::vec3(1, 0, 0)) *
		glm::rotate(Utility::ToRadians(rotation.z), glm::vec3(0, 0, 1)) *
		glm::rotate(Utility::ToRadians(rotation.y), glm::vec3(0, 1, 0)) *
		glm::scale(scale);

	glUniformMatrix4fv(modelMatrixLocation, 1, GL_TRUE, &modelMatrix[0][0]);
}