#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Utility.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::SetScale(float ScaleX, float ScaleY, float ScaleZ)
{
	scale.x = ScaleX;
	scale.y = ScaleY;
	scale.z = ScaleZ;
}

void Camera::SetWorldPos(float x, float y, float z)
{
	worldPos.x = x;
	worldPos.y = y;
	worldPos.z = z;
}

void Camera::SetRotate(float RotateX, float RotateY, float RotateZ)
{
	rotate.x = RotateX;
	rotate.y = RotateY;
	rotate.z = RotateZ;
}

const glm::mat4* Camera::GetTrans()
{
	glm::mat4 transformation = glm::translate(worldPos) * glm::rotate(rotate) * glm::scale(scale);

	return &m_transformation;
}