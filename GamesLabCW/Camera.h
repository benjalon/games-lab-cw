#include <glm/glm.hpp>

class Camera
{
private:
	const float FOV = glm::radians(45.0f);
	const float MIN_CLIP = 0.1f;
	const float MAX_CLIP = 100.0f;

	glm::vec3 position = glm::vec3(5.0f, 5.0f, 5.0f);
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

public:
	Camera();
	~Camera();

	glm::mat4 GetViewMatrix() { return viewMatrix; }
	glm::mat4 GetProjectionMatrix() { return projectionMatrix; }

	void Render(float windowWidth, float windowHeight);
};

