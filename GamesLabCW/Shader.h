#include <GL/glew.h>
#include <string>

class Shader
{
private:
	GLuint shaderID;
	GLuint mvpID;

	const std::string VS_FILE = "Shaders/shader.vs";
	const std::string FS_FILE = "Shaders/shader.fs";
public:
	Shader();
	~Shader();

	GLuint GetShaderID() { return shaderID; }

	void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	std::string Shader::ReadFile(std::string path);
};

