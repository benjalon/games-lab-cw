#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/mesh.h>

class MeshPart;

class Mesh
{
private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::mat4 modelMatrix;
	glm::mat4 mvpMatrix;

	std::vector<MeshPart*> meshEntries;

public:
	Mesh(const char *filename);
	~Mesh();

	void Render(GLuint shaderID, glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
};

