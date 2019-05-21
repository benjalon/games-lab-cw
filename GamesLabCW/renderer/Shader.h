#pragma once

#include <glad/glad.h>
#include <string>

namespace game
{
	const std::string GLSL_VERSION_DIRECTIVE = "#version 330 core\n";

	class Shader
	{
	private:
		Shader(const Shader&);

		std::string m_name;
		GLuint m_vertexShader;       //identifier for the vertex shader
		GLuint m_fragmentShader;     //identifier for the fragment shader
		GLuint m_programObject;      //identifier for the program- this is used when rendering.
		GLuint loadShader(const char* filename, const GLenum type, std::string prepend = "") const;
		std::string shaderInfoLog(const GLuint shader) const;
		std::string programInfoLog(const GLuint program) const;

	public:
		Shader(void);
		~Shader(void);

		//returns what we need for rendering
		GLuint handle(void) const { return m_programObject; }

		//loads the shader program from two text files
		bool load(const std::string name, const char* vertexFilename, const char* fragmentFilename,
			std::string vertexPrepend = "", std::string fragmentPrepend = "");
	};
}

