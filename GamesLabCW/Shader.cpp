#include "Shader.h"

#include <string>
#include <fstream>
#include <assert.h>

/*
	Create shader objects, currently hardcoded
*/
Shader::Shader()
{
	shaderID = glCreateProgram();
	assert(shaderID != 0 && "Error creating shader program");

	// Read shaders from file
	std::string vs = ReadFile(VS_FILE);
	AddShader(shaderID, vs.c_str(), GL_VERTEX_SHADER);
	std::string fs = ReadFile(FS_FILE);
	AddShader(shaderID, fs.c_str(), GL_FRAGMENT_SHADER);

	// Setup and validate program
	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
	}

	glUseProgram(shaderID);
}

Shader::~Shader()
{
}

/*
	Add specified shader to program
*/
void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);
	assert(ShaderObj != 0 && "Error creating shader type");

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

/*
	Read in a shader file
*/
std::string Shader::ReadFile(std::string path) {
	std::string content;

	std::ifstream fileStream(path, std::ios::in);
	assert(fileStream.is_open() && "Unable to open file for reading");

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();

	assert(content != "" && "Error reading shader program");
	return content;
}