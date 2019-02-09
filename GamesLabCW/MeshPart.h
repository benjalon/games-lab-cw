#include <GL/glew.h>
#include <assimp/mesh.h>

class MeshPart
{
private:
	enum BUFFERS { VERTEX_BUFFER, TEXCOORD_BUFFER, NORMAL_BUFFER, INDEX_BUFFER };

	GLuint vao;
	GLuint vbo[4];

	unsigned int elementCount;

public:
	MeshPart(aiMesh *mesh);
	~MeshPart();

	void Render();
};

