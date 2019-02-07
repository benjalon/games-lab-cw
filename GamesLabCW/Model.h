#include <glm/glm.hpp>

#include <GL/glew.h>

class Camera;

class Model
{
private:
	glm::mat4 modelMatrix;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	GLuint mvpMatrixLocation;
	GLuint vbo;
	GLuint ibo;
public:
	Model(GLuint shaderProgram);
	~Model();

	glm::vec3 GetPosition() { return position; }
	glm::vec3 GetRotation() { return rotation; }
	glm::vec3 GetScale() { return scale; }

	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetRotation(glm::vec3 rotation) { this->rotation = rotation; }
	void SetScale(glm::vec3 scale) { this->scale = scale; }
	void Move(int x, int y, int z) { this->position.x += x; this->position.y += y; this->position.z += z; }
	void Rotate(int x, int y, int z) { this->rotation.x += x; this->rotation.y += y; this->rotation.z += z; }
	void Scale(int x, int y, int z) { this->scale.x += x; this->scale.y += y; this->scale.z += z; }

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	void Render(Camera* camera);
};

