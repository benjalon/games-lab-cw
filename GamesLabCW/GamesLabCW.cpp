#include <stdio.h>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>

#include "GamesLabCW.h"
#include "Utility.h"
#include "Camera.h"
#include "Model.h"

const char* vsFileName = "shaders/shader.vs";
const char* fsFileName = "shaders/shader.fs";

Camera* camera;

GLuint shaderProgram;
Model* model;

int main(int argc, char** argv) {
	// Setup and create a cross-platform GLUT window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Games Lab CW");

	InitializeGlutCallbacks();

	// Initialize GLEW
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	// Setup rendering
	CompileShaders();

	camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT);
	model = new Model(shaderProgram);

	// Render 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glutMainLoop();

	delete model;
	delete camera;

	return 0;
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB); // Pass buffer clear function
	glutIdleFunc(RenderSceneCB);
}

/*
Clear back buffer, render onto it and then swap it in
*/
static void RenderSceneCB() {
	glClear(GL_COLOR_BUFFER_BIT);

	static float scale = 0.0f;
	scale += 0.001f;

	model->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	model->Rotate(0.0f, scale, 0.0f);

	camera->Render();
	model->Render(camera);

	glutSwapBuffers();
}

static void AddShader(GLuint shaderProgram, const char* shaderText, GLenum shaderType) {
	GLuint shaderObj = glCreateShader(shaderType);
	if (shaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", shaderType);
		exit(0);
	}

	const GLchar* p[1];
	p[0] = shaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(shaderText);
	glShaderSource(shaderObj, 1, p, Lengths);
	glCompileShader(shaderObj);
	GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(shaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", shaderType, InfoLog);
		exit(1);
	}

	glAttachShader(shaderProgram, shaderObj);
}

static void CompileShaders()
{
	shaderProgram = glCreateProgram();
	if (shaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	std::string vs = Utility::ReadFile(vsFileName);
	if (vs == "") {
		fprintf(stderr, "Error loading shader program\n");
		exit(1);
	}
	AddShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	std::string fs = Utility::ReadFile(fsFileName);
	if (fs == "") {
		fprintf(stderr, "Error loading shader program\n");
		exit(1);
	}
	AddShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", errorLog);
		exit(1);
	}

	glValidateProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, sizeof(errorLog), NULL, errorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", errorLog);
		exit(1);
	}

	glUseProgram(shaderProgram);
}