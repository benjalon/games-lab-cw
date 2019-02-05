#include <glm/glm.hpp>

#include <GL/glew.h>

class Model
{
private:
	glm::mat4 modelMatrix;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	GLuint modelMatrixLocation;
	GLuint vbo;
	GLuint ibo;
public:
	Model(GLuint shaderProgram);
	~Model();

	void SetPosition(glm::vec3 position) { this->position = position; }
	void SetRotation(glm::vec3 rotation) { this->rotation = rotation; }
	void SetScale(glm::vec3 scale) { this->scale = scale; }

	void Model::CreateVertexBuffer();
	void Model::CreateIndexBuffer();

	void Render();
};

