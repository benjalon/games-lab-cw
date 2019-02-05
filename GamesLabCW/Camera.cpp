#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx\transform.hpp"
#include "glm\gtc\matrix_transform.hpp"

Camera::Camera(float screenWidth, float screenHeight)
{
	aspectRatio = screenWidth / screenHeight;

	SetPosition(glm::vec3(0, 1, -1));
	SetTarget(glm::vec3(0, 0, 0)); // Look at origin
	SetUp(glm::vec3(0, 1, 0)); // Up is up
}

Camera::~Camera()
{
}

void Camera::Render()
{
	viewMatrix = glm::lookAt(position, target, up);

	projectionMatrix = glm::perspective(glm::radians(FOV), aspectRatio, MIN_RANGE, MAX_RANGE);
}