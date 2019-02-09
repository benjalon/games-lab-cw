#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Render(float windowWidth, float windowHeight)
{
	// Calculate matrices
	projectionMatrix = glm::perspective(FOV, windowWidth / windowHeight, MIN_CLIP, MAX_CLIP);
	viewMatrix = glm::lookAt(position, target, up);
}