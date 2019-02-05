#include <glm/glm.hpp>

#include <GL/glew.h>

class Camera
{
private:
	glm::mat4 viewMatrix;
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	glm::mat4 projectionMatrix;

	float aspectRatio;

	const float FOV = 45.0f;
	const float MIN_RANGE = 0.1f;
	const float MAX_RANGE = 100.0f;

public:
	Camera(float screenWidth, float screenHeight);
	~Camera();

	glm::mat4 GetViewMatrix() { return viewMatrix; }
	glm::mat4 GetProjectionMatrix() { return projectionMatrix; }

	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetTarget(glm::vec3 target) { this->target = target; }
	void SetUp(glm::vec3 up) { this->up = up; }
	void Move(int x, int y, int z) { this->position.x += x; this->position.y += y; this->position.z += z; }

	void Render();
};

